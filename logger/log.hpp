//
//  log.hpp
//
//  Created by Mayur Patel on 10/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef log_hpp
#define log_hpp

//#include "etw.hpp"
#include "format.hpp"
#include "type_name.hpp"
#include <array>
#include <string_view>
#include <type_traits>
#include <vector>

#define FMT_ENFORCE_COMPILE_STRING
#include <fmt/format.h>
#include "fmt_helpers.hpp"

#ifdef BUILDING_IN_D3
#undef EXPORT
#ifdef BLIP_BASE_MODULE
#define EXPORT __declspec( dllexport )
#else
#define EXPORT __declspec( dllimport )
#endif
#else
#include <iostream>
#define EXPORT
#define console std::cout
#endif

// defined per dll
extern int ETWStartEventId;

enum class LogLevel
{
    All = 0,
    Critical = 1,
    Error = 2,
    Warning = 3,
    Informational = 4,
    Verbose = 5
};

namespace logging
{

struct LogMacroData
{
    std::string_view format;
    std::string_view file;
    int32_t line;
};

struct LogMetaData
{
    int eventId;
    int taskId;
    std::string_view taskName;
    LogLevel level;
    LogMacroData macroData;
    // TODO use std::span so no runtime allocation is needed?
    std::vector<std::string_view> keywords;
    std::vector<std::string_view> fieldNames;
    std::vector<std::string_view> fieldTypes;
};

inline int eventIdSeq = ETWStartEventId;
inline int taskIdSeq = ETWStartEventId; // TODO: check if this is ok. doesn't need to be the same, but saves defining 2 globals per dll
template< typename T > inline const int task_id = taskIdSeq++;

EXPORT std::vector<LogMetaData>& getRegistry();
EXPORT void addToRegistry(const LogMetaData& data);
EXPORT void addToVector(std::vector<std::string_view>& vec, const std::string_view& str);
EXPORT std::string generateEventsYaml();

template <LogLevel level, typename Task, typename MacroData, typename... Args>
struct MetaDataNode
{
    MetaDataNode() : eventId(eventIdSeq++)
    {
        static constexpr LogMacroData macroData = MacroData{}();
        LogMetaData data;
        data.eventId = eventId;
        data.taskId = task_id<Task>;
        data.taskName = type_name<Task>();
        data.level = level;
        data.macroData = macroData;
        
        static constexpr std::string_view format = macroData.format;
        static constexpr auto argNames = ParseFormatString<format>::argNames;
        static_assert(argNames.size() == sizeof...(Args), "number of args must match format string");

        for (const auto& v : argNames)
            addToVector(data.fieldNames, v);

        (addToVector(data.fieldTypes, type_name<Args>()), ...);
        
        for (const auto& k : Task::keywords)
            addToVector(data.keywords, k);

        addToRegistry(data);
    }

    int32_t eventId;
};

template<LogLevel level, typename Task, typename MacroData, typename... Args>
inline MetaDataNode<level, Task, MacroData, Args...> meta_data_node{};

template <LogLevel level, typename Task, typename MacroData, typename... Args>
void logFunc(Args&&... args)
{
    static const auto metaData = meta_data_node<level, Task, MacroData, Args...>;
    static const int eventId = metaData.eventId;
    static constexpr std::string_view format = MacroData{}().format;
    static constexpr std::string_view parsedFormat = ParseFormatString<format>::format;
    //logEtw(std::forward<Args>(args)...);
    if constexpr (level <= LogLevel::Informational)
        console << fmt::format(FMT_STRING(parsedFormat), std::forward<Args>(args)...) << "\n";
}

// TODO remove after testing
struct DefaultTask
{
    static constexpr std::array keywords = {"hi", "there"};
};

// Macros
#define LOGTASK(level, Task, format, ...)                       \
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
    logFunc<level, Task, decltype(anonymous_meta_data)>(__VA_ARGS__);  \
} while(false)                                                  \

#define LOG(level, format, ...)  LOGTASK(level, logging::DefaultTask, format, __VA_ARGS__)

#define LOGA(...) LOG(LogLevel::All,  __VA_ARGS__)
#define LOGC(...) LOG(LogLevel::Critical,  __VA_ARGS__)
#define LOGE(...) LOG(LogLevel::Error,  __VA_ARGS__)
#define LOGW(...) LOG(LogLevel::Warning,  __VA_ARGS__)
#define LOGI(...) LOG(LogLevel::Informational,  __VA_ARGS__)
#define LOGD(...) LOG(LogLevel::Verbose,  __VA_ARGS__)

// TODO add all of these
#define LOGTASKI(...) LOGTASK(LogLevel::Informational,  __VA_ARGS__)

} // namespace logging

#endif // log_hpp
