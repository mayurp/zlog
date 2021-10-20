//
//  log.hpp
//
//  Created by Mayur Patel on 10/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef log_hpp
#define log_hpp

#include "ctf_writer.hpp"
#include "format_parser.hpp"
#include "reflection.hpp"
#include "type_name.hpp"
#include "type_traits.hpp"
#ifdef USE_FMT_LOGGING
#include "fmt_helpers.hpp"
#endif


#include "magic_enum.hpp"

#include <string_view>
#include <type_traits>
#include <vector>


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
    LogLevel level;
    LogMacroData macroData;
    // TODO use std::span so no runtime allocation is needed?
    std::vector<std::string_view> fieldNames;
    std::vector<reflection::Type> fieldTypes;
};

// TODO put this somewhere more obvious
// 1 and 2 reserved for ctf cyg_profile
inline int event_id_seq = 3;

std::vector<LogMetaData>& get_registry();

void add_to_registry(const LogMetaData& data);

// transform type to one suitable for serialisation
// e.g int -> int32_t
//     SomeType& -> SomeType
template<typename T>
struct serialised_type
{
private:
    using U = remove_cvref_t<T>;
public:
    using type = typename std::conditional_t<
        std::is_integral_v<U>,
        explicit_int_type_t<U>, U>;
};

template<typename T>
using serialised_type_t = typename serialised_type<T>::type;


template <LogLevel level, typename MacroData, typename... Args>
struct MetaDataNode
{
    MetaDataNode() : eventId(event_id_seq++)
    {
        static constexpr LogMacroData macroData = MacroData{}();
        LogMetaData data;
        data.eventId = eventId;
        data.eventName = type_helper::func_name(macroData.function);
        data.level = level;
        data.macroData = macroData;
        
        static constexpr std::string_view format = macroData.format;
        static constexpr auto argNames = ParseFormatString<format>::argNames;
        static_assert(argNames.size() == sizeof...(Args), "number of args must match format string");

        // Add arg names and types for event definition
        (data.fieldTypes.emplace_back(reflection::make_reflection_type<Args>()), ...);
        for (const auto& argName : argNames)
            data.fieldNames.emplace_back(argName);
        
        add_to_registry(data);
    }

    int32_t eventId;
};

template<LogLevel level, typename MacroData, typename... Args>
inline MetaDataNode<level, MacroData, Args...> meta_data_node{};

template <LogLevel level, typename MacroData, typename... Args>
void log_func(Args&&... args)
{
    static const auto metaData = meta_data_node<level, MacroData, Args...>;
    static const int eventId = metaData.eventId;
    static constexpr std::string_view format = MacroData{}().format;
    static constexpr std::string_view parsedFormat = ParseFormatString<format>::format;

    barectf::log_event(eventId, std::forward<Args>(args)...);

#ifdef USE_FMT_LOGGING
    // TODO: make this work again and use same timestamp as barectf
    if constexpr (level <= LogLevel::Informational)
        std::cout << fmt::format(FMT_STRING(parsedFormat), std::forward<Args>(args)...) << std::endl;
#endif
}

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

// Macros
#define LOG(level, format, ...)                                 \
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
    log_func<level, MetaData>(__VA_ARGS__);                      \
} while(false)                                                  \

#define LOGC(format, ...) LOG(LogLevel::Critical, format, __VA_ARGS__)
#define LOGE(format, ...) LOG(LogLevel::Error, format, __VA_ARGS__)
#define LOGW(format, ...) LOG(LogLevel::Warning, format, __VA_ARGS__)
#define LOGI(format, ...) LOG(LogLevel::Informational, format, __VA_ARGS__)
#define LOGD(format, ...) LOG(LogLevel::Debug, format, __VA_ARGS__)

} // namespace logging

#endif // log_hpp
