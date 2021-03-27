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

#include <iostream>
#include <type_traits>
#include <string_view>
#include <vector>

#include <ctti/type_id.hpp>

namespace log
{

inline int log_id = 0;

inline int task_id_seq = 0;
template< typename T > inline const int task_id = task_id_seq++;

struct LogMacroData
{
    std::string_view format;
    std::string_view file;
    int32_t line;
};

struct LogMetaData
{
    int logId;
    int taskId;
    std::string_view taskName;
    LogMacroData macroData;
    // TODO use std::span so no runtime allocation is needed?
    std::vector<std::string_view> keywords;
    std::vector<std::string_view> fieldNames;
    std::vector<std::string_view> fieldTypes;
};

inline std::vector<LogMetaData>& getRegistry()
{
    static std::vector<LogMetaData> registry;
    return registry;
}
    
constexpr std::string_view convert(const ctti::detail::cstring& str)
{
    return std::string_view(str.begin(), str.length());
}

template <typename Task, typename MacroData, typename... Args>
struct MetaDataNode
{
    MetaDataNode() : id(log_id++)
    {
        LogMetaData data;
        data.logId = id;
        data.taskId = task_id<Task>;
        data.taskName = convert(ctti::nameof<Task>());
        data.macroData = macroData;
        constexpr auto parsedFields = parseString([](){return MacroData{}().format;});
        static_assert(parsedFields.varNames.size() == sizeof...(Args), "number of args must match format string");
        
        for (const auto& v : parsedFields.varNames)
            data.fieldNames.emplace_back(v);
        
        (data.fieldTypes.emplace_back(convert(ctti::nameof<Args>())), ...);
        
        for (const auto& k : Task::keywords)
            data.keywords.emplace_back(k);
        
        getRegistry().emplace_back(data);
    }

    int32_t id;
    LogMacroData macroData = MacroData{}();
};

template<typename Task, typename MacroData, typename... Args>
inline MetaDataNode<Task, MacroData, Args...> meta_data_node{};


template <typename Task, typename MacroData, typename... Args>
void logFunc(Args&&... args)
{
    static const auto id = meta_data_node<Task, MacroData, Args...>.id;
    constexpr auto parsedFields = parseString([](){return MacroData{}().format;});
    constexpr auto cleanFormat = parsedFields.formatStr;
    std::cout << id << " " << cleanFormat.view() << "\n";
}

struct DefaultTask
{
    static constexpr std::array keywords = {"he", "there"};
};
    
//int defaultId = task_id<DefaultTask>;
    
#define LOGTASK(Task, format, ...)                              \
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
    logFunc<Task, decltype(anonymous_meta_data)>(__VA_ARGS__);  \
} while(false)                                                  \

#define LOG(format, ...)  LOGTASK(log::DefaultTask, format, __VA_ARGS__)

}
#endif /* log_h */
