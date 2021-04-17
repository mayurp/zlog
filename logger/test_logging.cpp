//#include "logging/log.hpp"
//#include "catch_include.hpp"

#include "log.hpp"
//#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
//#if __cplusplus < 201703L
//#error yes
//#endif

// Compile time checks
void formatParsing()
{
    static constexpr std::string_view str = "{argName}";
    static_assert(ParseFormatString<str>::format == "{}");
    static_assert(ParseFormatString<str>::argNames.size() == 1);
    static_assert(ParseFormatString<str>::argNames[0] == "argName");

    // TODO make these passs

    // handle escaped braces
    //static constexpr std::string_view str = "{{someText}} {argName}";
    //static_assert(ParseFormatString<str>::format == "{{someText}} {}");
    //static_assert(ParseFormatString<str>::argNames.size() == 1);
    //static_assert(ParseFormatString<str>::argNames[0] == "argName");

    // handle fmt specifiers
    //static constexpr std::string_view str = "{argName:d}";
    //static_assert(ParseFormatString<str>::format == "{:d}");
    //static_assert(ParseFormatString<str>::argNames.size() == 1);
    //static_assert(ParseFormatString<str>::argNames[0] == "argName");
}

TEST_CASE("test logging", "[etw][logging]")
{
    //static constexpr std::string_view f = "some {0}\n";
    //console << fmt::format(FMT_STRING(f), 1);
    std::cout << "---------------------\n";
    std::cout << logging::generateEventsYaml();
    std::cout << "---------------------\n";
    
    LOGI("This is some logging with value: {value}", 1);
} 
