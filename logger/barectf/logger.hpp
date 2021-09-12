//
//  logger.hpp
//  logger
//
//  Created by Mayur Patel on 28/08/2021.
//

#ifndef logger_hpp
#define logger_hpp


#include "barectf.h"
#include "../reflection.hpp"

#include <string>
#include <type_traits>



struct barectf_platform_linux_fs_ctx;
struct barectf_default_ctx;

namespace barectf
{

struct ScopedContext
{
    ScopedContext();
    ~ScopedContext();

    barectf_platform_linux_fs_ctx* platform_ctx;
};

barectf_default_ctx* get_context();


inline size_t arg_size(const std::string& str)
{
    return str.length() + 1;
}

inline size_t arg_size(const std::string_view& str)
{
    return str.length() + 1;
}

inline size_t arg_size(const char* str)
{
    return strlen(str) + 1;
}

template<typename T,
    std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, bool> = true
>
constexpr size_t arg_size(T&& arg)
{
    return sizeof(arg);
}

template<typename T,
    std::enable_if_t<reflection::is_reflected_v<std::decay_t<T>>, bool> = true
>
constexpr size_t arg_size(T&& arg)
{
    size_t size = 0;
    reflection::for_each(arg, [&](std::string_view member, const auto& value)
    {
        size += arg_size(value);
    });
    return size;
}

template<typename... Args>
constexpr size_t payload_size(Args&&... args)
{
    return (arg_size(std::forward<Args>(args)) + ...);
}

inline void serialize_arg(uint8_t*& buf, const char* arg)
{
    const size_t size = strlen(arg) + 1;
    memcpy(buf, arg, size);
    buf += size;
}

inline void serialize_arg(uint8_t*& buf, const std::string& arg)
{
    memcpy(buf, arg.data(), arg.length());
    buf += arg.length();
    *buf = '\0';
    buf += 1;
}

inline void serialize_arg(uint8_t*& buf, const std::string_view& arg)
{
    memcpy(buf, arg.data(), arg.length());
    buf += arg.length();
    *buf = '\0';
    buf += 1;
}

template<typename T,
    std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, bool> = true
>
void serialize_arg(uint8_t*& buf, T&& arg)
{
    memcpy(buf, &arg, sizeof(arg));
    buf += sizeof(arg);
}

template<typename T,
    std::enable_if_t<reflection::is_reflected_v<std::decay_t<T>>, bool> = true
>
constexpr void serialize_arg(uint8_t*& buf, T&& arg)
{
    reflection::for_each(arg, [&](std::string_view member, const auto& value)
    {
        serialize_arg(buf, value);
    });
}

template<typename Arg1, typename... Args>
void serialize_args(uint8_t*& buf, Arg1&& arg1, Args&&... args)
{
    serialize_arg(buf, std::forward<Arg1>(arg1));
    if constexpr (sizeof...(args) > 0)
        serialize_args(buf, std::forward<Args>(args)...);
}

// templated helpers
template <typename... Args>
void logEvent(uint32_t event_id, Args&&... args)
{
    // Get context for current thread
    barectf_default_ctx* sctx = get_context();
    if (!sctx)
        throw std::logic_error("barectf not initialised on this thread");

    barectf_ctx* const ctx = &sctx->parent;
    
    /* Save timestamp */
    sctx->cur_last_event_ts = ctx->cbs.default_clock_get_value(ctx->data);

    // TODO: check if this will be needed once log enabling is controlled from higher up in log.hpp
    if (!ctx->is_tracing_enabled)
    {
        return;
    }

    /* We can alter the packet */
    ctx->in_tracing_section = 1;
    
    const uint32_t er_header_size = barectf_size_default_header(ctx);
    const uint32_t er_payload_size = uint32_t(_BYTES_TO_BITS(payload_size(std::forward<Args>(args)...)));
    const uint32_t packet_size = er_header_size + er_payload_size;
    
    /* Is there enough space to serialize? */
    if (!barectf_reserve_er_space(ctx, packet_size))
    {
        /* don't forget this */
        ctx->in_tracing_section = 0;
        return;
    }

    /* Serialize event record */

    /* Serialize header */
    barectf_serialize_er_header_default(ctx, event_id);

    /* Write payload structure */
    {
        uint8_t* const start = ctx->buf + _BITS_TO_BYTES(ctx->at);
        uint8_t* curr = start;
        serialize_args(curr, std::forward<Args>(args)...);
        const size_t bytes_written = curr - start;
        ctx->at += _BYTES_TO_BITS(bytes_written);
    }
        
    /* Commit event record */
    barectf_commit_er(ctx);

    /* Not tracing anymore */
    ctx->in_tracing_section = 0;
}

}

#endif /* logger_hpp */
