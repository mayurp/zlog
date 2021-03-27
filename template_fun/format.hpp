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


template <size_t N>
struct FixedString
{
    constexpr std::string_view view() const { return { data, size }; }
    char data[N];
    size_t size;
};

//template <typename StringHolder>
//constexpr auto parseString(StringHolder holder)

template <typename Char>
constexpr size_t countVarNames(const Char* it)
{
    size_t count = 0;
    while(*it)
    {
        char ch = *it++;
        // TODO get char type
        if (ch == '{')
            ++count;
    }
    return count;
}

template<size_t SSIZE, size_t NVARS>
struct ParseResult{
    FixedString<SSIZE> formatStr;
    std::array<std::string_view, NVARS> varNames;
};


template <typename StringHolder>
constexpr auto parseString(StringHolder holder)
{
    using Char = char;
    constexpr std::string_view formatStr = holder();
    FixedString<formatStr.length()> result = {};
    constexpr size_t nVars = countVarNames(formatStr.begin());
    std::array<std::string_view, nVars> varNames = {};
    
    int argId = 0;
    auto it = formatStr.begin();
    int dst = 0;
    while (*it)
    {
        char ch = *it++;
        if (ch != '{' && ch != '}')
        {
            result.data[dst++] = ch;
            continue;
        }
        if (ch == '{')
        {
            if (*it == '}')
                throw std::runtime_error("must specify var name");

            result.data[dst++] = '{';
            result.data[dst++] = '0' + argId++;            
            const Char* start = it;
            for (;*it && *it != '}'; ++it)
            {}
            if (*it != '}')
                throw std::runtime_error("unmatched '{' in format string");
            
            const Char* end = it-1;
            varNames[argId-1] = std::string_view(start, end - start + 1);
            
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
    for (int i = 0; i < varNames.size(); ++i)
    {
        for (int j = i+1; j < varNames.size(); ++j)
        {
            if (varNames[i] == varNames[j])
                throw std::runtime_error("arg names repeated in format string");
        }
    }
    
    return ParseResult<formatStr.length(), nVars>{result, varNames};
}

#define PARSE_STRING(text) \
parseString([](){return text;})


#define FORMAT(text) [](){return text;}


#endif /* format_h */
