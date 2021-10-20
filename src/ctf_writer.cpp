//
//  logger.cpp
//  logger
//
//  Created by Mayur Patel on 09/09/2021.
//

#include "ctf_writer.hpp"
#include "barectf-platform-linux-fs.h"
#include "log.hpp"

#include <exception>
#include <fstream>
#include <map>
#include <sstream>
#include <thread>


namespace barectf
{

std::atomic_int thread_counter = 0;
thread_local int thread_id = thread_counter++;
thread_local std::unique_ptr<Context> default_thread_context;

std::string generate_ctf_metadata();

Session::Session(const std::filesystem::path& _trace_dir)
    : trace_dir(_trace_dir)
{
    const std::string metadata = barectf::generate_ctf_metadata();
    std::filesystem::create_directories(trace_dir);
    std::ofstream fs(trace_dir / "metadata");
    if (fs.is_open())
        fs << metadata;
    else
        throw std::runtime_error("Unable to create metadata file");
}

// Don't call this in hot path due to locking needed around local static
/*static*/ Session& Session::get_default_session()
{
    static Session session("trace");
    return session;
}

bool has_default_context()
{
    return default_thread_context != nullptr;
}

uint32_t get_threadid()
{
    return thread_id;
}

Context::Context(const Session& session)
{
    if (default_thread_context)
        throw std::runtime_error("barectf::Context already exists for this thread");
    const std::filesystem::path filename = session.trace_dir / ("thread_" + std::to_string(thread_id));
    platform_ctx = barectf_platform_linux_fs_init(512, filename.string().c_str(), 0, 0, 0);
    ctx = barectf_platform_linux_fs_get_barectf_ctx(platform_ctx);
}

barectf_default_ctx* Context::get_default_context()
{
    if (!default_thread_context)
        default_thread_context = std::make_unique<Context>(Session::get_default_session());
    return default_thread_context->ctx;
}

Context::~Context()
{
    if (platform_ctx)
        barectf_platform_linux_fs_fini(platform_ctx);
    platform_ctx = nullptr;
    ctx = nullptr;
}

static const char* ctf_basic_config()
{
    return R"(/* CTF 1.8 */
trace {
    major = 1;
    minor = 8;
    byte_order = le;
    packet.header := struct {
        integer {
            signed = false;
            size = 32;
            align = 8;
            byte_order = native;
            base = 10;
        } magic;
        integer {
            signed = false;
            size = 64;
            align = 8;
            byte_order = native;
            base = 10;
        } stream_id;
    } align(8);
};

env {
    domain = "bare";
    tracer_name = "barectf";
    tracer_major = 3;
    tracer_minor = 0;
    tracer_patch = 1;
    tracer_pre = "";
    barectf_gen_date = "2021-08-28T20:56:22.074322";
};

clock {
    name = default;
    freq = 1000000000;
    precision = 0;
    offset_s = 0;
    offset = 0;
    absolute = false;
};

/* Data stream type `default` */
stream {
    id = 0;
    packet.context := struct {
        integer {
            signed = false;
            size = 64;
            align = 8;
            byte_order = native;
            base = 10;
        } packet_size;
        integer {
            signed = false;
            size = 64;
            align = 8;
            byte_order = native;
            base = 10;
        } content_size;
        integer {
            signed = false;
            size = 64;
            align = 8;
            byte_order = native;
            base = 10;
            map = clock.default.value;
        } timestamp_begin;
        integer {
            signed = false;
            size = 64;
            align = 8;
            byte_order = native;
            base = 10;
            map = clock.default.value;
        } timestamp_end;
        integer {
            signed = false;
            size = 64;
            align = 8;
            byte_order = native;
            base = 10;
        } events_discarded;
    } align(8);
    event.header := struct {
        integer {
            signed = false;
            size = 64;
            align = 8;
            byte_order = native;
            base = 10;
        } id;
        integer {
            signed = false;
            size = 64;
            align = 8;
            byte_order = native;
            base = 10;
            map = clock.default.value;
        } timestamp;
    } align(8);
    event.context := struct {
        integer { size = 32; align = 8; signed = 1; encoding = none; base = 10; } _vtid;
    } align(8);
};

event {
    name = "lttng_ust_cyg_profile:func_entry";
    id = 1;
    stream_id = 0;
    loglevel = 12;
    fields := struct {
        integer { size = 64; align = 8; signed = 0; encoding = none; base = 16; } _addr;
        integer { size = 64; align = 8; signed = 0; encoding = none; base = 16; } _call_site;
    };
};

event {
    name = "lttng_ust_cyg_profile:func_exit";
    id = 2;
    stream_id = 0;
    loglevel = 12;
    fields := struct {
        integer { size = 64; align = 8; signed = 0; encoding = none; base = 16; } _addr;
        integer { size = 64; align = 8; signed = 0; encoding = none; base = 16; } _call_site;
    };
};
    )";
}

static std::string ctf_basic_types()
{
    std::ostringstream ss;
    for (int size : {8, 16, 32, 64} )
    {
        for (bool signd : { true, false })
        {
            ss << "typealias integer {\n";
            ss << "    signed = " << (signd ? "true" : "false") << ";\n";
            ss << "    size = " << size << ";\n";
            ss << "    byte_order = native;\n";
            ss << "    base = 10;\n";
            ss << "    align = 1;\n";
            ss << "} := " << (signd ? "" : "u") << "int" << size << "_t;\n\n";
        }
    }

    ss << R"(
typealias floating_point {
    exp_dig = 8;
    mant_dig = 24;
    byte_order = native;
    align = 1;
} := float;

typealias floating_point {
    exp_dig = 11;
    mant_dig = 53;
    byte_order = native;
    align = 1;
} := double;

typealias integer {
    size = 8;
    signed = true;
    byte_order = native;
    base = 10;
    align = 1;
} := bool;
    
    )";
    
    return ss.str();
}

// Visit reflection::Type variant and output TDSL
// Final ";\n" is omiitted to support recursion
struct Visitor
{
    Visitor(std::ostream& ss_, int indentCount_, const std::string_view& fieldName_)
        : ss(ss_), indentCount(indentCount_), fieldName(fieldName_)
    {}
    
    Visitor indented_visitor(const std::string_view& fieldName_ = "") const
    {
        return Visitor(ss, indentCount + 4, fieldName_);
    }

    void operator()(const reflection::Primitive& t) const
    {
        indent();
        ss << t.name << " " << fieldName;
    }

    void operator()(const reflection::Clazz& t) const
    {
        indent();
        ss << t.name << " " << fieldName;
    }

    void operator()(const reflection::Array& t) const
    {
        if (t.isDynamic)
        {
            indent();
            ss << "uint32_t " << fieldName << "_length;\n";
            std::visit(*this, t.valueType);
            ss << " [" << fieldName << "_length]";
        }
        else
        {
            std::visit(*this, t.valueType);
            ss << " [" << t.size << "]";
        }
    }

    void operator()(const reflection::Tuple& t) const
    {
        indent();
        ss << "struct {\n";
        size_t i = 0;;
        for (const auto& type : t.types)
        {
            std::visit(indented_visitor("v" + std::to_string(i++)), type);
            ss << ";\n";
        }
        indent();
        ss << "} align(1) " << fieldName;
    }

    void operator()(const reflection::Enum& t) const
    {
        indent();
        ss << "enum " << t.name << " " << fieldName;
    }

    void operator()(const reflection::Map& map) const
    {
        indent();
        ss << "typealias struct {\n";
        std::visit(indented_visitor("key"), map.keyType);
        ss << ";\n";
        std::visit(indented_visitor("value"), map.valueType);
        ss << ";\n";
        indent();
        ss << "} :=  " << fieldName << "_entry;\n";
        indent();
        ss << "uint32_t " << fieldName << "_length;\n";
        indent();
        ss << fieldName << "_entry " << fieldName << "[" << fieldName << "_length]";
    }
    
    void indent() const
    {
        ss << std::setw(indentCount) << "";
    }
    
    std::ostream& ss;
    int indentCount = 0;
    std::string_view fieldName;
};

static std::string ctf_custom_types()
{
    using namespace reflection;
    std::ostringstream ss;
    for (const auto& type: reflection::get_type_registry())
    {
        std::visit([&](auto&& t)
        {
            using T = std::decay_t<decltype(t)>;
            if constexpr (std::is_same_v<T, reflection::Clazz>)
            {
                ss << "typealias struct {\n";
                for (const auto& field: t.fields)
                {
                    std::visit(Visitor(ss, 4, field.name), field.type);
                    ss << ";\n";
                }
                ss << "} := " << t.name << ";\n\n";
            }
            if constexpr (std::is_same_v<T, reflection::Enum>)
            {
                ss << "enum " << t.name << " : " << t.integerType << " {\n";
                for (const auto& field: t.fields)
                {
                    ss << "    " << field.name << " = " << field.value << ",\n";
                }
                ss << "};\n\n";
            }
        }, type);
    }
    return ss.str();
}

enum class LttngLogLevel
{
    TRACE_EMERG = 0,
    TRACE_ALERT = 1,
    TRACE_CRIT = 2,
    TRACE_ERR = 3,
    TRACE_WARNING = 4,
    TRACE_NOTICE = 5,
    TRACE_INFO = 6,
    TRACE_DEBUG = 14
};

static LttngLogLevel toLttngLogLevel(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Critical: return LttngLogLevel::TRACE_CRIT;
        case LogLevel::Error: return LttngLogLevel::TRACE_ERR;
        case LogLevel::Warning: return LttngLogLevel::TRACE_WARNING;
        case LogLevel::Informational: return LttngLogLevel::TRACE_INFO;
        case LogLevel::Debug: return LttngLogLevel::TRACE_DEBUG;
    }
}

std::string generate_ctf_metadata()
{
    std::ostringstream ss;

    ss << ctf_basic_config() << "\n";
    ss << ctf_basic_types() << "\n";
    ss << ctf_custom_types() << "\n";

    for (const auto& metaData : logging::get_registry())
    {
        // event name is identifier for callsite block so need to ensure it's
        // unique by including the eventId
        // add format string here as workaround for tooling not supporting this as a separate field
        std::ostringstream ns;
        ns << "[" << metaData.eventId << "] " << metaData.macroData.format;
        const std::string eventName = ns.str();
        
        const auto& macroData = metaData.macroData;
        ss << "event {\n";
        ss << "    stream_id = 0;\n";
        ss << "    id = " << metaData.eventId << ";\n";
        ss << "    name = \"" << eventName << "\";\n";
        ss << "    loglevel = " << static_cast<int>(toLttngLogLevel(metaData.level)) << ";\n";
        // TODO: add back when user attributes are properly supported in babeltrace and TraceCompass
        // This will come with CTF 2.0 support
        //ss << "    msg = \"" << macroData.format << "\";\n";
        ss << "    fields := struct {\n";
        for (int i = 0; i < metaData.fieldNames.size(); ++i)
        {
            const reflection::Type& type = metaData.fieldTypes[i];
            const std::string_view& fieldName = metaData.fieldNames[i];
            std::visit(Visitor(ss, 8, fieldName), type);
            ss << ";\n";
        }
        ss << "    } align(1);\n";
        ss << "};\n\n";
        
        // defined separately from event for some reason
        ss << "callsite {\n";
        ss << "    name = \"" << eventName << "\";\n";
        ss << "    file = \"" << macroData.file << "\";\n";
        ss << "    line = " << macroData.line << ";\n";
        ss << "    func = \"" << macroData.function << "\";\n";
        ss << "};\n\n";
    }

    return ss.str();
}

}

extern "C"
{

// For profiling and flamegraphs
// TODO: don't hardcode event ids

NO_INSTRUMENT
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    // TODO: don't hardcode event ids
    if (barectf::has_default_context())
        barectf::log_event(1, (uint64_t) this_fn, (uint64_t) call_site);
}

NO_INSTRUMENT
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    if (barectf::has_default_context())
        barectf::log_event(2, (uint64_t) this_fn, (uint64_t) call_site);
}

}
