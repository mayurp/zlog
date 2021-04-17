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

// Compile time parsing of log format string to:
//    1. Extract named arguments 
//    2. Strip out arg names so make it compatible with fmt
//       example: "Some message: {argName}"  -> "Some message: {}"
//
// TODO: handle escaped braces
// TOOD: allow for fmt specifiers. e.g {argName:d}.  Currently the entire string inside {} is taken as the arg name 
template <std::string_view const& Format>
struct ParseFormatString
{
private:
    static constexpr size_t countArgs(const std::string_view& str)
    {
        size_t count = 0;
       for (char c : str)
            if (c == '{') ++count;
        return count;
    }

    // TODO simplify
    static constexpr auto parse()
    {
        using Char = char;
        constexpr std::string_view format = Format; // workaround for MSVC bug
        FixedString<format.size()> strippedFormat = {};
        constexpr size_t nArgs = countArgs(format);
        std::array<std::string_view, nArgs> argNames;

        size_t argIndex = 0;
        auto it = format.begin();
        int dst = 0;
        while (it != format.end())
        {
            const Char ch = *it++;
            if (ch != '{' && ch != '}')
            {
                strippedFormat.data[dst++] = ch;
                continue;
            }
            if (ch == '{')
            {
                if (it == format.end() || *it == '}')
                    throw std::runtime_error("must specify arg name");

                const auto start = it;
                for (;it != format.end() && *it != '}'; ++it)
                {}
                if (it == format.end() || *it != '}')
                    throw std::runtime_error("unmatched '{' in format string");
                const auto end = it; // 1 position after end of string

                strippedFormat.data[dst++] = '{';
                argNames[argIndex++] = std::string_view(&(*start), end - start);

                strippedFormat.data[dst++] = '}';
                ++it;
                continue;
            }
            if (ch == '}')
            {
                throw std::runtime_error("unmatched '}' in format string");
            }
        }
        strippedFormat.size = dst;

        // check argnames are unique
        for (int i = 0; i < argNames.size(); ++i)
        {
            for (int j = i+1; j < argNames.size(); ++j)
            {
                if (argNames[i] == argNames[j])
                    throw std::runtime_error("arg names repeated in format string");
            }
        }

        return std::pair{strippedFormat, argNames};
    }

    static constexpr auto parseResult = parse();
public:
    static constexpr std::string_view format = parseResult.first.view();
    static constexpr auto argNames = parseResult.second;
};

#endif /* format_h */
