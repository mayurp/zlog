//
//  log.hpp
//
//  Created by Mayur Patel on 10/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef log_hpp
#define log_hpp

//#include "etw.hpp"
#include "ctf/ctf_logger.hpp"
#include "format.hpp"
#include "type_name.hpp"
#include "type_traits.hpp"
#include <array>
#include <string_view>
#include <type_traits>
#include <vector>


#define FMT_ENFORCE_COMPILE_STRING
#include <fmt/format.h>
#include "fmt_helpers.hpp"

#include "magic_enum.hpp"


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
    Critical,
    Error,
    Warning,
    Informational,
    Debug,
};

namespace logging
{

struct LogMacroData
{
    std::string_view format;
    std::string_view function;
    std::string_view file;
    int32_t line;
};

struct LogMetaData
{
    int eventId;
    std::string_view eventName;
    int taskId;
    std::string_view taskName;
    LogLevel level;
    LogMacroData macroData;
    // TODO use std::span so no runtime allocation is needed?
    std::vector<std::string_view> keywords;
    std::vector<std::string_view> fieldNames;
    std::vector<reflection::Type> fieldTypes;
};

inline int eventIdSeq = ETWStartEventId;
inline int taskIdSeq = ETWStartEventId; // TODO: check if this is ok. doesn't need to be the same, but saves defining 2 globals per dll
template< typename T > inline const int task_id = taskIdSeq++;

EXPORT std::vector<LogMetaData>& getRegistry();
EXPORT void addToRegistry(const LogMetaData& data);
EXPORT void addToVector(std::vector<std::string_view>& vec, const std::string_view& str);
EXPORT std::string generateEventsYaml();

// transform type to one suitable for serialisation
// e.g int -> int32_t
//     SomeType& -> SomeType
template<typename T>
struct serialised_type
{
private:
    using U = typename std::decay_t<T>;
public:
    using type = typename std::conditional_t<
        std::is_integral_v<U>,
        explicit_int_type_t<U>, U>;
};

template<typename T>
using serialised_type_t = typename serialised_type<T>::type;


// TODO: move to reflection?
template<typename T>
reflection::Type makeReflectionType()
{
    using U = serialised_type_t<T>;
    static constexpr std::string_view name = type_name_v<U>;
    if constexpr (std::is_enum_v<U>)
    {
        reflection::Enum e;
        e.name = name;
        e.integerType = type_name_v<typename magic_enum::underlying_type<T>::type>;
        for (const auto value : magic_enum::enum_values<T>())
        {
            e.fields.push_back({ magic_enum::enum_name(value), magic_enum::enum_integer(value) });
        }
        // Only enums will get auto registered when they are used my LOG macros
        // Classes need to be explicitly registered
        reflection::getTypeRegistry().push_back(e);
        return e;
    }
    else if constexpr (std::is_arithmetic_v<U> || is_string_v<U>)
    {
        reflection::Primitive prim{name};
        return prim;
    }
    else if constexpr (is_std_array_v<U>)
    {
        reflection::Array array;
        array.isDynamic = false;
        using VT = serialised_type_t<typename T::value_type>;
        array.valueType = type_name_v<VT>;
        array.size == T::size;
        return array;
    }
    else
    {
        // Fields not populated here since we only need the typename
        // TODO: make this nicer
        reflection::Clazz clazz{name};
        return clazz;
    }
}

template <LogLevel level, typename Task, typename MacroData, typename... Args>
struct MetaDataNode
{
    MetaDataNode() : eventId(eventIdSeq++)
    {
        static constexpr LogMacroData macroData = MacroData{}();
        LogMetaData data;
        data.eventId = eventId;
        data.eventName = funcName(macroData.function);
        static constexpr std::string_view taskName = className(macroData.function);
        data.taskId = -1;// task_id<Task>; // TODO: fix. Doesn't make sense since this we're ignore Task type
        data.taskName = taskName;//type_name<Task>();
        data.level = level;
        data.macroData = macroData;
        
        static constexpr std::string_view format = macroData.format;
        static constexpr auto argNames = ParseFormatString<format>::argNames;
        static_assert(argNames.size() == sizeof...(Args), "number of args must match format string");

        // Add arg names and types for event definition
        (data.fieldTypes.push_back(makeReflectionType<Args>()), ...);
        for (const auto& argName : argNames)
            addToVector(data.fieldNames, argName);

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
    barectf::log_event(eventId, std::forward<Args>(args)...);
    if constexpr (level <= LogLevel::Informational)
        console << fmt::format(FMT_STRING(parsedFormat), std::forward<Args>(args)...) << "\n";
}

// TODO remove after testing
struct DefaultTask
{
    static constexpr std::array keywords = {"hi", "there"};
};

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

// Macros
#define LOGTASK(level, Task, format, ...)                       \
do                                                              \
{                                                               \
    using namespace logging;                                    \
    static constexpr std::string_view function = __PRETTY_FUNCTION__ ;\
    struct MetaData                                             \
    {                                                           \
        constexpr LogMacroData operator()() const noexcept      \
        {                                                       \
            return LogMacroData{format, function, __FILE__, __LINE__};    \
        }                                                       \
    };                                                          \
                                                                \
    logFunc<level, Task, MetaData>(__VA_ARGS__);                \
} while(false)                                                  \

#define LOG(level, format, ...)  LOGTASK(level, logging::DefaultTask, format, __VA_ARGS__)
#define LOGC(format, ...) LOG(LogLevel::Critical,  format, __VA_ARGS__)
#define LOGE(format, ...) LOG(LogLevel::Error,  format, __VA_ARGS__)
#define LOGW(format, ...) LOG(LogLevel::Warning,  format, __VA_ARGS__)
#define LOGI(format, ...) LOG(LogLevel::Informational,  format, __VA_ARGS__)
#define LOGD(format, ...) LOG(LogLevel::Debug,  format, __VA_ARGS__)

#define LOGTASKC(Task, format, ...) LOGTASK(LogLevel::Critical, Task,  format, __VA_ARGS__)
#define LOGTASKE(Task, format, ...) LOGTASK(LogLevel::Error, Task, format, __VA_ARGS__)
#define LOGTASKW(Task, format, ...) LOGTASK(LogLevel::Warning, Task, format, __VA_ARGS__)
#define LOGTASKI(Task, format, ...) LOGTASK(LogLevel::Informational, Task, format, __VA_ARGS__)
#define LOGTASKD(Task, format, ...) LOGTASK(LogLevel::Debug, Task, format, __VA_ARGS__)

} // namespace logging

#endif // log_hpp
