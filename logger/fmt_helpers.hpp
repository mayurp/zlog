//
//  fmt_helpers.hpp
//  template_fun
//
//  Created by Mayur Patel on 02/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef fmt_helpers_h
#define fmt_helpers_h


#include "format.hpp"
#include "reflection.hpp"
#include "type_traits.hpp"

#define FMT_ENFORCE_COMPILE_STRING
#include <fmt/format.h>
//#include <fmt/ranges.h>


namespace fmt_helpers
{

// append to char array if not nullpt, otherwise just increment dst
constexpr void append(char arr[], int& dst, const std::string_view& str)
{
    for (char c : str)
    {
        if (arr)
            arr[dst] = c;
        ++dst;
    }
}

template <typename T>
constexpr auto makeFormatString()
{
    auto makeString = [](char arr[])
    {
        using R = reflection::reflect_members<T>;
        int dst = 0;
        append(arr, dst, "(");
        for (int i = 0; i < R::N; ++i)
        {
            const std::string_view name = R::names[i];
            append(arr, dst, name);
            append(arr, dst, ": {}");
            if (i != R::N - 1)
                append(arr, dst, ", ");
        }
        append(arr, dst, ")");
        return dst;
    };

    // first pass to get size
    constexpr int size = makeString(nullptr);
    FixedString<size> result = {};
    
    // actually make string
    makeString(result.data);
    
    return result;
}

template <typename T>
constexpr auto makeTupleFormatString()
{
    auto makeString = [](char arr[])
    {
        constexpr int N = std::tuple_size<T>{};
        int dst = 0;
        append(arr, dst, "(");
        for (int i = 0; i < N; ++i)
        {
            append(arr, dst, "{}");
            if (i != N - 1)
                append(arr, dst, ", ");
        }
        append(arr, dst, ")");
        return dst;
    };
    
    // first pass to get size
    constexpr int size = makeString(nullptr);
    FixedString<size> result = {};

    // actually make string
    makeString(result.data);
    
    return result;
}

// base implementation using reflection to generate formatter with compile time format string
template<typename T>
struct ReflectionFormatter : fmt::formatter<std::string>
{
    template <std::size_t ...Is>
    auto makeFormatter(const T& t, fmt::format_context& ctx, std::index_sequence<Is...>)
    {
        using R = reflection::reflect_members<reflection::remove_cvref_t<T>>;
        static constexpr auto fstr = makeFormatString<T>();
        return formatter<std::string>::format(
                                fmt::format(FMT_STRING(fstr.view()), R::template get<Is>(t)...),
                                ctx);
    }
    auto format(const T& t, fmt::format_context& ctx)
    {
        using R = reflection::reflect_members<reflection::remove_cvref_t<T>>;
        return makeFormatter(t, ctx, std::make_index_sequence<R::N>{});
    }
};

}

// Use custom formatters for containers and tuples to ensure compile time strings are used
template<typename T, typename Char>
struct fmt::formatter<T, Char, typename std::enable_if_t<is_range_v<T, Char>>> : fmt::formatter<std::string>
{
    auto format(const T& t, fmt::format_context& ctx)
    {
        return formatter<std::string>::format(fmt::format(FMT_STRING("[{}]"), fmt::join(t, ", ")), ctx);
    }
};

template<typename T, typename Char>
struct fmt::formatter<T, Char, typename std::enable_if_t<is_tuple_v<T>>> : fmt::formatter<std::string>
{
    template <std::size_t ...Is>
    auto makeFormatter(const T& t, fmt::format_context& ctx, std::index_sequence<Is...>)
    {
        static constexpr auto fstr = fmt_helpers::makeTupleFormatString<T>();
        return formatter<std::string>::format(
                                fmt::format(FMT_STRING(fstr.view()), std::get<Is>(t)...),
                                ctx);
    }

    auto format(const T& t, fmt::format_context& ctx)
    {
        return makeFormatter(t, ctx, std::make_index_sequence<std::tuple_size<T>{}>{});;
    }
};

#define FORMATTER(T) \
template <> \
struct fmt::formatter<T> : fmt_helpers::ReflectionFormatter<T> \
{};\

#define REFLECT_WITH_FORMATTER(T, ...) \
    REFLECT(T, __VA_ARGS__)\
    FORMATTER(T)\

#endif /* fmt_helpers_h */
