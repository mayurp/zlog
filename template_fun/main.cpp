#include <array>
#include <type_traits>
#include <string_view>
#include <string>
#include <iostream>
#include <sstream>

#include <ctti/type_id.hpp>
#include "Common.hpp"
#include "log.hpp"
#include "somewhere.hpp"


template <size_t N>
struct fixed_string
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
    fixed_string<SSIZE> formatStr;
    std::array<std::string_view, NVARS> varNames;
};


template <typename StringHolder>
constexpr auto parseString(StringHolder holder)
{
    using Char = char;
    constexpr std::string_view formatStr = holder();
    fixed_string<formatStr.length()> result = {};
    constexpr size_t nVars = countVarNames(formatStr.begin());
    std::array<std::string_view, nVars> varNames;

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
            result.data[dst++] = '{';
            result.data[dst++] = '0' + argId++;
            if (*it == '}')
                throw std::runtime_error("must specify var name");
         
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
    return ParseResult<formatStr.length(), nVars>{result, varNames};
}

#define PARSE_STRING(text) \
parseString([](){return text;})


#define FORMAT(text) [](){return text;}

#define LOG(STR, ...)

template <typename StringHolder, typename... Args>
auto logTest(StringHolder holder, Args... args)
{
    auto result = parseString(holder);
    std::ostringstream os;
    os << result.formatStr.view() << std::endl;
    for (auto s : result.varNames)
        os << s << ", ";
    
    os << std::endl;
    ((os << ctti::nameof<decltype(args)>() << " " << args  << std::endl), ...);
    return os.str();
}

//#define LOGID(X) type_id<X>

int main2()
{
    std::string w = "the end";
    int x = 1;
    float y = 2.3;
    bool z = true;

    
    auto result = logTest(FORMAT("{one}, {two} {three} -- {four}"), w, x, y, z);
    std::cout << result;
    /*
    constexpr auto result = log("{a}, {j} {bc} -- {dd}", w, x, y, z);
    //constexpr auto expr2 = parseString(expr1.view().begin());
    std::cout << result.formatStr.view() << std::endl; // ((y) + (z))
    for (auto s : result.varNames)
    {
        std::cout << s << ", ";
    }
     */
    
    return 0;
}


void test()
{
    LOGID("abc");
    LOGID("efg");
}

int main()
{

    std::cout << "Registry\n";
    for (const auto& [id, data] : log::getRegistry())
    {
        std::cout << "id: " << id << ", file: " << data.file << ", line: " << data.line << " -- ";
        std::cout << "formatStr: " << data.format << "\n";
    }
    std::cout << "\n";

    return 0;
}
