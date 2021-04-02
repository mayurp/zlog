//
//  fmt_helpers.hpp
//  template_fun
//
//  Created by Mayur Patel on 02/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef fmt_helpers_h
#define fmt_helpers_h

#include "reflection.hpp"

#define FMT_ENFORCE_COMPILE_STRING
#include <fmt/format.h>


namespace fmt_helpers
{

// TODO make constexpr string helpers a bit nicer

template<typename T>
constexpr int formatStringSize()
{
    using R = reflection::reflect_members<T>;
    int length = 0;
    for (int i = 0; i < R::N; ++i)
    {
        const std::string_view name = R::names[i];
        length += (name.length() + 4);
        if (i < R::N - 1)
            length += 2;
    }
    length += 2;
    return length;
}

template <typename T>
constexpr auto makeFormatString()
{
    constexpr int LENGTH = formatStringSize<T>();
    FixedString<LENGTH> result = {};
    using R = reflection::reflect_members<T>;

    int dst = 0;
    result.data[dst++] = '(';

    for (int i = 0; i < R::N; ++i)
    {
        const std::string_view name = R::names[i];
        for (char c : name)
            result.data[dst++] = c;
        result.data[dst++] = ':';
        result.data[dst++] = ' ';
        result.data[dst++] = '{';
        result.data[dst++] = '}';
        if (i != R::N - 1)
        {
            result.data[dst++] = ',';
            result.data[dst++] = ' ';
        }
    }
    result.data[dst++] = ')';

    result.size = dst;
    
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

// standard containers
// TODO make work for any container
template<typename T>
struct fmt::formatter<std::vector<T>> : fmt::formatter<std::string>
{
    auto format(const std::vector<T>& t, fmt::format_context& ctx)
    {
        return formatter<std::string>::format(fmt::format(FMT_STRING("[{}]"), fmt::join(t, ", ")), ctx);
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
