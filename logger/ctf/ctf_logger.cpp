//
//  logger.cpp
//  logger
//
//  Created by Mayur Patel on 09/09/2021.
//

#include "ctf_logger.hpp"
#include "barectf-platform-linux-fs.h"
#include "../log.hpp"

#include <exception>
#include <map>
#include <sstream>
#include <fstream>
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

static const char* ctf_basic_types()
{
    return R"(
typealias integer {
    signed = false;
    size = 32;
    byte_order = native;
    base = 10;
    align = 1;
} := uint32_t;

typealias integer {
    signed = true;
    size = 32;
    byte_order = native;
    base = 10;
    align = 1;
} := int64_t;
    
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
}

static std::string ctf_custom_types()
{
    std::ostringstream ss;
    for (const auto& type: reflection::getTypeRegistry())
    {
        ss << "typealias struct {\n";
        for (const auto& field: type.fields)
        {
            ss << "    " << field.typeName << " " << field.name << ";\n";
        }
        ss << "} := " << type.name << ";\n\n";
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
    // TODO group events by task
    std::map<int, std::vector<logging::LogMetaData>> perTaskEvents;

    for (const auto& metaData : logging::getRegistry())
    {
        perTaskEvents[metaData.taskId].push_back(metaData);
    }

    std::ostringstream ss;

    ss << ctf_basic_config() << "\n";
    ss << ctf_basic_types() << "\n";
    ss << ctf_custom_types() << "\n";

    for (const auto& entry : perTaskEvents)
    {
        const int taskId = entry.first;
        const auto& events = entry.second;
        assert(!events.empty());
        for (const auto& metaData : events)
        {
            if (metaData.macroData.line < 200)
                continue;

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
                ss << "        " << metaData.fieldTypes[i] << " " << metaData.fieldNames[i] << ";\n";
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

    }
    return ss.str();
}

}

#define _GNU_SOURCE
#include <dlfcn.h>

extern "C"
{

NO_INSTRUMENT
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    // TODO: don't hardcode event id
    if (barectf::has_default_context())
        barectf::log_event(1, (uint64_t)this_fn, (uint64_t)call_site);
//
//    Dl_info info;
//    if (dladdr(this_fn, &info))
//        printf("%p %s\n",
//               this_fn,
//               //info.dli_fname ? info.dli_fname : "?",
//               info.dli_sname ? info.dli_sname : "?");
}

NO_INSTRUMENT
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    if (barectf::has_default_context())
        barectf::log_event(2, (uint64_t) this_fn, (uint64_t) call_site);
}

}
