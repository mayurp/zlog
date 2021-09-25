//
//  log.cpp
//  template_fun
//
//  Created by Mayur Patel on 12/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#include "log.hpp"

#include "magic_enum.hpp"

#include <sstream>
#include <map>
#include <regex>
#include <vector>

#define CHECK_LOGIC(X) assert(X)
#define CONTEXT()

namespace logging
{

std::vector<LogMetaData>& getRegistry()
{
    static std::vector<LogMetaData> registry;
    return registry;
}

void addToRegistry(const LogMetaData& data)
{
    getRegistry().emplace_back(data);
}

void addToVector(std::vector<std::string_view>& vec, const std::string_view& str)
{
    vec.emplace_back(str);
}

static std::string makeEtwFormatString(const std::string& fmtString)
{
    CONTEXT();

    const std::regex re(R"(\{([^}]+)\})");
    int n = 0;
    std::string output;
    const std::sregex_token_iterator begin(fmtString.begin(), fmtString.end(), re, { -1, 0 });
    for (auto it = begin; it != std::sregex_token_iterator(); ++it)
    {
        const auto m = *it;

        if (std::regex_match(m.str(), re))
            output += "%" + std::to_string(++n);
        else
            output += m.str();
    }

    return output;
}

std::string generateEventsYaml()
{
    CONTEXT();

    // TODO group events by task
    std::map<int, std::vector<logging::LogMetaData>> perTaskEvents;

    for (const auto& metaData : logging::getRegistry())
    {
        perTaskEvents[metaData.taskId].push_back(metaData);
    }

    std::ostringstream ss;
    ss << "Tasks:\n";
    for (const auto& entry : perTaskEvents)
    {
        const int taskId = entry.first;
        const auto& events = entry.second;
        CHECK_LOGIC(!events.empty());
        ss << "- Name: \"" << events[0].taskName << "\"\n";
        // TODO add task message ??
        ss << "  Mesage: " << events[0].taskName << "\n";
        ss << "  Id: " << taskId << "\n";
        ss << "  Events:\n";
        for (const auto& metaData : events)
        {
            const auto& macroData = metaData.macroData;
            // TODO add level and name to macro data
            ss << "      - Name: \"TODO\"\n";
            ss << "        Id: " << metaData.eventId << "\n";
            ss << "        Level: *" << magic_enum::enum_name(metaData.level) << "\n";
            // TODO regex replace args with ordinal values %1 , %2 etc {};
            // TODO escape specal chars?
            ss << "        Message: \"" << makeEtwFormatString(std::string(macroData.format)) << "\"\n";
            ss << "        File: \"" << macroData.file << "\"\n";
            ss << "        Line: " << macroData.line << "\n";

            if (!metaData.fieldNames.empty())
                ss << "        Parameters:\n";

            for (int i = 0; i < metaData.fieldNames.size(); ++i)
            {
                // TODO map to ETW typenames
                ss << "          -{Name: \"" << metaData.fieldNames[i] << ", Type: *" << metaData.fieldTypes[i] << "}\n";
            }

            if (!metaData.keywords.empty())
                ss << "        Keywords: [";
            for (int i = 0; i < metaData.keywords.size(); ++i)
            {
                ss << "&" << metaData.keywords[i];
                if (i != metaData.keywords.size() - 1)
                    ss << ", ";
            }
            ss << "]\n";
        }

    }
    return ss.str();
}

const char* ctfConfigDef()
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
    )";
}

const char* ctfBasicTypes()
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

std::string ctfCustomTypes()
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

std::string generateCtfMetaData()
{
    CONTEXT();

    // TODO group events by task
    std::map<int, std::vector<logging::LogMetaData>> perTaskEvents;

    for (const auto& metaData : logging::getRegistry())
    {
        perTaskEvents[metaData.taskId].push_back(metaData);
    }

    std::ostringstream ss;

    ss << ctfConfigDef() << "\n";
    ss << ctfBasicTypes() << "\n";
    ss << ctfCustomTypes() << "\n";

    for (const auto& entry : perTaskEvents)
    {
        const int taskId = entry.first;
        const auto& events = entry.second;
        CHECK_LOGIC(!events.empty());
        for (const auto& metaData : events)
        {
            if (metaData.eventId < 1007)
                continue;

            const auto& macroData = metaData.macroData;
            ss << "event {\n";
            ss << "    stream_id = 0;\n";
            ss << "    id = " << metaData.eventId << ";\n";
            ss << "    name = \"" << metaData.eventName << "_" << metaData.eventId << "\";\n";
            ss << "    loglevel = " << static_cast<int>(metaData.level) << ";\n";
            ss << "    msg = \"" << macroData.format << "\";\n";
            // TODO: add to common meta data?
            ss << "    fields := struct {\n";
            for (int i = 0; i < metaData.fieldNames.size(); ++i)
            {
                ss << "        " << metaData.fieldTypes[i] << " " << metaData.fieldNames[i] << ";\n";
            }
            ss << "    } align(1);\n";
            ss << "};\n\n";
            
            // defined separately from event for some reason
            // TODO: event name is identifier here so need to ensure it's unique somehow
            ss << "callsite {\n";
            ss << "    name = \"" << metaData.eventName << "_" << metaData.eventId << "\";\n";
            ss << "    file = \"" << macroData.file << "\";\n";
            ss << "    line = " << macroData.line << ";\n";
            ss << "    func = \"" << macroData.function << "\";\n";
            ss << "};\n\n";
        }

    }
    return ss.str();
}



}
