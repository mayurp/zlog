#include "log.hpp"
//#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>


// Compile time checks

static constexpr std::string_view str1  = "{argName}";
static_assert(ParseFormatString<str1>::format == "{}");
static_assert(ParseFormatString<str1>::argNames.size() == 1);
static_assert(ParseFormatString<str1>::argNames[0] == "argName");

// handle escaped braces
static constexpr std::string_view str2 = "{{someText}} {argName}";
static_assert(ParseFormatString<str2>::format == "{{someText}} {}");
static_assert(ParseFormatString<str2>::argNames.size() == 1);
static_assert(ParseFormatString<str2>::argNames[0] == "argName");

// handle fmt specifiers
static constexpr std::string_view str3 = "{argName:d}";
static_assert(ParseFormatString<str3>::format == "{:d}");
static_assert(ParseFormatString<str3>::argNames.size() == 1);
static_assert(ParseFormatString<str3>::argNames[0] == "argName");

static constexpr std::string_view str4 = "{argName:d} {b}";
static_assert(ParseFormatString<str4>::format == "{:d} {}");
static_assert(ParseFormatString<str4>::argNames.size() == 2);
static_assert(ParseFormatString<str4>::argNames[0] == "argName");
static_assert(ParseFormatString<str4>::argNames[1] == "b");
