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

template<size_t Size, size_t NArgs>
struct ParseResult
{
    FixedString<Size> format;
    std::array<std::string_view, NArgs> argNames;
};

template <std::string_view const& Format>
struct ParseFormatString
{
private:
    static constexpr auto impl()
    {
        using Char = char;
        constexpr std::string_view format = Format; // workaround for MSVC bug
        FixedString<format.size()> strippedFormat = {};
        constexpr size_t nArgs = countArgs(format);
        static_assert(nArgs <= 10, "Maximum of 10 args supported");
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
                // TODO handle escapeed brances "{{"

                if (it == format.end() || *it == '}')
                    throw std::runtime_error("must specify arg name");

                const auto start = it;
                for (;it != format.end() && *it != '}'; ++it)
                {}
                if (it == format.end() || *it != '}')
                    throw std::runtime_error("unmatched '{' in format string");
                const auto end = it;

                strippedFormat.data[dst++] = '{';
                strippedFormat.data[dst++] = '0' + Char(argIndex); // TODO support more than 10 args
                argNames[argIndex++] = std::string_view(&(*start), end - start);  // wrong??

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

        // check if argnames are unique
        for (int i = 0; i < argNames.size(); ++i)
        {
            for (int j = i+1; j < argNames.size(); ++j)
            {
                if (argNames[i] == argNames[j])
                    throw std::runtime_error("arg names repeated in format string");
            }
        }

        return ParseResult<format.size(), nArgs>{strippedFormat, argNames};
    }

    static constexpr auto parseResult = impl();

public:
    static constexpr std::string_view format = parseResult.format.view();
    static constexpr auto argNames = parseResult.argNames;
};

#endif /* format_h */
