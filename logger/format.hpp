//
//  format.hpp
//  template_fun
//
//  Created by Mayur Patel on 12/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef format_h
#define format_h


#include <string_view>


// statically allocated string - not null terminated
template <size_t N>
struct FixedString
{
    constexpr std::string_view view() const { return { data, size }; }
    char data[N];
    size_t size = N; // default to entire buffer
};

//template <typename STRING>//typename Char>
constexpr size_t countArgs(const std::string_view& str)
{
    size_t count = 0;
    for (char c : str)
        if (c == '{') ++count;
    return count;
}

template<size_t SSIZE, size_t NVARS>
struct ParseResult
{
    FixedString<SSIZE> formatStr;
    std::array<std::string_view, NVARS> argNames;
};

template <std::string_view const& formatStr>
struct ParseString
{
    static constexpr auto impl()
    {
        using Char = char;
        FixedString<formatStr.size()> result = {};
        constexpr size_t nArgs = countArgs(formatStr);
        static_assert(nArgs <= 10, "Maximum of 10 args supported");
        std::array<std::string_view, nArgs> argNames;

        size_t argIndex = 0;
        auto it = formatStr.begin();
        int dst = 0;
        while (it != formatStr.end())
        {
            const Char ch = *it++;
            if (ch != '{' && ch != '}')
            {
                result.data[dst++] = ch;
                continue;
            }
            if (ch == '{')
            {
                // TODO handle escapeed brances "{{"

                if (it == formatStr.end() || *it == '}')
                    throw std::runtime_error("must specify arg name");

                const auto start = it;
                for (;it != formatStr.end() && *it != '}'; ++it)
                {}
                if (it == formatStr.end() || *it != '}')
                    throw std::runtime_error("unmatched '{' in format string");
                const auto end = it;

                result.data[dst++] = '{';
                result.data[dst++] = '0' + Char(argIndex); // TODO support more than 10 args
                argNames[argIndex] = std::string_view(&(*start), end - start);  // wrong??
                argIndex++;

                result.data[dst++] = '}';
                ++it;
                continue;
            }
            if (ch == '}')
            {
                throw std::runtime_error("unmatched '}' in format string");
            }
        }
        result.size = dst;

        // check if argnames are unique
        for (int i = 0; i < argNames.size(); ++i)
        {
            for (int j = i+1; j < argNames.size(); ++j)
            {
                if (argNames[i] == argNames[j])
                    throw std::runtime_error("arg names repeated in format string");
            }
        }

        return ParseResult<formatStr.size(), nArgs>{result, argNames};
    }

    constexpr auto operator()()
    {
        return impl();
    }
};

#endif /* format_h */
