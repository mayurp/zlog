//
//  log.hpp
//  template_fun
//
//  Created by Mayur Patel on 10/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef log_h
#define log_h

#include "format.hpp"


#include <type_traits>
#include <string_view>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#include <ctti/type_id.hpp>

namespace log
{

struct LogMacroData
{
    std::string_view format;
    std::string_view file;
    int32_t line;
};

struct LogMetaData
{
    LogMacroData macroData;
    std::vector<std::string_view> fieldNames;
    std::vector<std::string_view> fieldTypes;

};

inline std::map<int, LogMetaData>& getRegistry()
{
    static std::map<int, LogMetaData> registry;
    return registry;
}
    
constexpr std::string_view convert(const ctti::detail::cstring& str)
{
    return std::string_view(str.begin(), str.length());
}

template <typename MacroData, typename... Args>
struct MetaDataNode
{
    MetaDataNode() : id(log_id++)
    {
        LogMetaData data;
        data.macroData = macroData;
        constexpr auto parsedFields = parseString([](){return MacroData{}().format;});
        static_assert(parsedFields.varNames.size() == sizeof...(Args), "number of args must match format string");
        for (const auto& v : parsedFields.varNames)
        {
            data.fieldNames.push_back(v);
        }
        (data.fieldTypes.emplace_back(convert(ctti::nameof<Args>())), ...);
        getRegistry()[id] = data;
    }

    int32_t id;
    LogMacroData macroData = MacroData{}();
};

template<typename MacroData, typename... Args>
inline MetaDataNode<MacroData, Args...> meta_data_node{};


template <typename MacroData, typename... Args>
void logFunc(Args&&... args)
{
    static const auto id = meta_data_node<MacroData, Args...>.id;
    constexpr auto parsedFields = parseString([](){return MacroData{}().format;});
    constexpr auto cleanFormat = parsedFields.formatStr;
    std::cout << id << " " << cleanFormat.view() << "\n";
}

#define LOGID(format, ...)                                      \
do                                                              \
{                                                               \
    using namespace log;                                        \
    struct                                                      \
    {                                                           \
        constexpr LogMacroData operator()() const noexcept      \
        {                                                       \
            return LogMacroData{format, __FILE__, __LINE__};    \
        }                                                       \
    } anonymous_meta_data;                                      \
                                                                \
    logFunc<decltype(anonymous_meta_data)>(__VA_ARGS__);        \
} while(false)                                                  \

}
#endif /* log_h */
