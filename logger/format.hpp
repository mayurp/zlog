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
#include "ctstring.hpp"


// Compile time parsing of log format string to:
//    1. Extract named arguments 
//    2. Strip out arg names so make it compatible with fmt
//       example: "Some message: {argName}"  -> "Some message: {}"
//
// TODO: check arg names are valid variable names (e.g no spaces)
template <std::string_view const& Format>
struct ParseFormatString
{
private:
    // Just count unescaped open brackets
    // parse function does error handling
    static constexpr size_t countArgs(const std::string_view& str)
    {
        size_t count = 0;
        auto it = str.begin();
        while (it != str.end())
        {
            const char ch = *it++;
            if (ch == '{')
            {
                if (it != str.end() && *it == '{')
                {
                    ++it;
                    continue;
                }
                ++count;
            }
        }
        return count;
    }

    // TODO validate argNmme?
    static constexpr std::string_view getArgName(const std::string_view& argStr)
    {
        const size_t colon_pos = argStr.find(':');
        return argStr.substr(0, colon_pos);
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
                if (it != format.end() && *it == '{')
                {
                    strippedFormat.data[dst++] = ch;
                    strippedFormat.data[dst++] = *it;
                    ++it;
                    continue;
                }

                const size_t pos = it - format.begin();
                const size_t end_brace_pos = format.find('}', pos);
                if (end_brace_pos == std::string_view::npos)
                    throw std::runtime_error("unmatched { in format string");
                const size_t argLen = end_brace_pos - pos;
                if (argLen < 1)
                    throw std::runtime_error("must specify an arg name");
                const std::string_view argStr(&(*it), argLen);
                const std::string_view argName = getArgName(argStr);
                const std::string_view argFormatting = argStr.substr(argName.size());
                argNames[argIndex++] = argName;
                strippedFormat.data[dst++] = '{';
                for (char c : argFormatting)
                    strippedFormat.data[dst++] = c;
                strippedFormat.data[dst++] = '}';
                it += (argLen + 1);
                continue;
            }
            if (ch == '}')
            {
                if (it != format.end() && *it == '}')
                {
                    strippedFormat.data[dst++] = ch;
                    strippedFormat.data[dst++] = *it;
                    ++it;
                    continue;
                }

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
