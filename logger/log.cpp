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
        assert(!events.empty());
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
                ss << "        ";
                ss << "          -{Name: \"" << metaData.fieldNames[i] << ", Type: *";
                const reflection::Type& type = metaData.fieldTypes[i];
                std::visit([&](auto&& t)
                {
                    using T = std::decay_t<decltype(t)>;
                    if constexpr (std::is_same_v<T, reflection::Clazz> || std::is_same_v<T, reflection::Primitive>)
                        ss << t.name;
                    else
                        ss << "---not supported yet----\n";
                }, type);
                ss << "}\n";
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

}
