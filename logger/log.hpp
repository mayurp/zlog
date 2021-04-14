//
//  log.hpp
//
//  Created by Mayur Patel on 10/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#pragma once

//#include "etw.hpp"
#include "format.hpp"
#include "type_name.hpp"

#include <array>
#include <string_view>
#include <type_traits>
#include <vector>

#define FMT_ENFORCE_COMPILE_STRING
//#include <fmt/format.h>
//#include "fmt_helpers.hpp"


#undef EXPORT
#ifdef BLIP_BASE_MODULE
#define EXPORT __declspec( dllexport )
#else
#define EXPORT __declspec( dllimport )
#endif


namespace logging
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

EXPORT std::vector<LogMetaData>& getRegistry();
EXPORT void addToRegistry(const LogMetaData& data);
EXPORT void addToVector(std::vector<std::string_view>& vec, const std::string_view& str);

template <typename Task, typename MacroData, typename... Args>
struct MetaDataNode
{
    MetaDataNode() : id(log_id++)
    {
        LogMetaData data;
        data.logId = id;
        data.taskId = task_id<Task>;
        data.taskName = type_name<Task>();
        data.macroData = macroData;
        //constexpr auto parsedFields = ParseString<macroData.format>{}();
        //static_assert(parsedFields.argNames.size() == sizeof...(Args), "number of args must match format string");

        //for (const auto& v : parsedFields.argNames)
        //    addToVector(data.fieldNames, v);

        (addToVector(data.fieldTypes, type_name<Args>()), ...);
        
        for (const auto& k : Task::keywords)
            addToVector(data.keywords, k);

        addToRegistry(data);
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
    static constexpr std::string_view format = MacroData{}().format;
    static constexpr auto parsedFields = ParseString<format>{}();
    static constexpr std::string_view cleanFormat = parsedFields.formatStr.view();
    //logEtw(std::forward<Args>(args)...);
    //console << fmt::format((cleanFormat), std::forward<Args>(args)...);
    console << cleanFormat;// fmt::format(FMT_STRING(cleanFormat.view()), std::forward<Args>(args)...) << std::endl;
}

struct DefaultTask
{
    static constexpr std::array keywords = {"he", "there"};
};

#define LOGTASK(Task, format, ...)                              \
do                                                              \
{                                                               \
    using namespace logging;                                    \
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

#define LOG(format, ...)  LOGTASK(logging::DefaultTask, format, __VA_ARGS__)

} // namespace logging
