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

#include <filesystem>
#include <string>
#include <type_traits>

#include <iostream>


struct barectf_platform_linux_fs_ctx;
struct barectf_default_ctx;

#ifndef NO_INSTRUMENT
    #define NO_INSTRUMENT __attribute__((no_instrument_function))
#endif

namespace barectf
{

// session for entire application
struct Session
{
    NO_INSTRUMENT
    Session(const std::filesystem::path& _trace_dir);

    NO_INSTRUMENT
    static Session& get_default_session();
    
    std::filesystem::path trace_dir;
};

// RAII wrapper for barectf
struct Context
{
    NO_INSTRUMENT
    Context(const Session& session);
    
    NO_INSTRUMENT
    ~Context();
  
    // Get default barectf context of current thread
    NO_INSTRUMENT
    static barectf_default_ctx* get_default_context();

private:
    barectf_platform_linux_fs_ctx* platform_ctx = nullptr;
    barectf_default_ctx* ctx = nullptr;
};

// Get sequential thread id used in logs
NO_INSTRUMENT
uint32_t get_threadid();

// Helpers to for binary serialisation
NO_INSTRUMENT
inline size_t arg_size(const std::string& str)
{
    return str.length() + 1;
}

NO_INSTRUMENT
inline size_t arg_size(const std::string_view& str)
{
    return str.length() + 1;
}

NO_INSTRUMENT
inline size_t arg_size(const char* str)
{
    return strlen(str) + 1;
}

template<typename T,
    std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>> ||
                     std::is_enum_v<std::remove_reference_t<T>>, bool> = true
>
NO_INSTRUMENT
constexpr size_t arg_size(T&& arg)
{
    return sizeof(arg);
}

template<typename T,
    std::enable_if_t<reflection::is_reflected_v<std::decay_t<T>>, bool> = true
>
NO_INSTRUMENT
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
NO_INSTRUMENT
inline constexpr size_t payload_size(Args&&... args)
{
    if constexpr (sizeof...(Args) == 0)
        return 0;
    else
        return (arg_size(std::forward<Args>(args)) + ...);
}

NO_INSTRUMENT
inline void serialize_arg(uint8_t*& buf, const char* arg)
{
    const size_t size = strlen(arg) + 1;
    memcpy(buf, arg, size);
    buf += size;
}

NO_INSTRUMENT
inline void serialize_arg(uint8_t*& buf, const std::string& arg)
{
    memcpy(buf, arg.data(), arg.length());
    buf += arg.length();
    *buf = '\0';
    buf += 1;
}

NO_INSTRUMENT
inline void serialize_arg(uint8_t*& buf, const std::string_view& arg)
{
    memcpy(buf, arg.data(), arg.length());
    buf += arg.length();
    *buf = '\0';
    buf += 1;
}

template<typename T,
    std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>> ||
                     std::is_enum_v<std::remove_reference_t<T>>, bool> = true
>
NO_INSTRUMENT
inline void serialize_arg(uint8_t*& buf, T&& arg)
{
    memcpy(buf, &arg, sizeof(arg));
    buf += sizeof(arg);
}

template<typename T,
    std::enable_if_t<reflection::is_reflected_v<std::decay_t<T>>, bool> = true
>
NO_INSTRUMENT
inline constexpr void serialize_arg(uint8_t*& buf, T&& arg)
{
    reflection::for_each(arg, [&](std::string_view member, const auto& value)
    {
        serialize_arg(buf, value);
    });
}

template<typename Arg1, typename... Args>
NO_INSTRUMENT
inline void serialize_args(uint8_t*& buf, Arg1&& arg1, Args&&... args)
{
    serialize_arg(buf, std::forward<Arg1>(arg1));
    if constexpr (sizeof...(args) > 0)
        serialize_args(buf, std::forward<Args>(args)...);
}

// Write barectf packet
template <typename... Args>
NO_INSTRUMENT
void log_event(uint32_t event_id, Args&&... args)
{
    // Get context for current thread
    barectf_default_ctx* sctx = Context::get_default_context();
    if (!sctx)
        throw std::logic_error("barectf not initialised on this thread");

    barectf_ctx* const ctx = &sctx->parent;
     
    if (ctx->in_tracing_section)
    {
        // prevent recursion when using cyg_profile hooks
        return;
    }
    
    /* Save timestamp */
    sctx->cur_last_event_ts = ctx->cbs.default_clock_get_value(ctx->data);

    // TODO: check if this will be needed once log enabling is controlled from higher up in log.hpp
    if (!ctx->is_tracing_enabled )
    {
        return;
    }

    /* We can alter the packet */
    ctx->in_tracing_section = 1;
    
    const uint32_t er_header_size = barectf_size_default_header(ctx);
    const uint32_t er_payload_size = uint32_t(_BYTES_TO_BITS(payload_size(std::forward<Args>(args)...)));
    const uint32_t er_event_ctx_size = barectf_size_stream_event_context(ctx);
    const uint32_t packet_size = er_header_size + er_payload_size + er_event_ctx_size;
    
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

    /* Serialize event context */
    barectf_serialize_er_stream_event_context(ctx, get_threadid());
    
    /* Write payload structure */
    if constexpr (sizeof...(Args) > 0)
    {
        uint8_t* const start = ctx->buf + _BITS_TO_BYTES(ctx->at);
        uint8_t* curr = start;
        
        // Sanity check
        //const uint32_t before = ctx->at;
        
        serialize_args(curr, std::forward<Args>(args)...);
        const size_t bytes_written = curr - start;
           
        // Snaity check
        //if (before != ctx->at)
        //    throw std::logic_error("ctx->at should not have changed");

        ctx->at += _BYTES_TO_BITS(bytes_written);
    }
    
    // Sanity check
    /*
    uint32_t diff = ctx->at - begin;
    if (diff != packet_size)
        throw std::logic_error("incorrect number of btyes written for packet");
    */

    /* Commit event record */
    barectf_commit_er(ctx);

    /* Not tracing anymore */
    ctx->in_tracing_section = 0;
}

}

#endif /* logger_hpp */
