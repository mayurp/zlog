#include "logging/log.hpp"

#include "catch_include.hpp"

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

void testLogRegistry()
{
    std::cout << "Log Registry\n";
    for (const auto& metaData : logging::getRegistry())
    {
        const logging::LogMacroData& macroData = metaData.macroData;
        std::cout << "-------------------------\n";
        std::cout << "taskId: " << metaData.taskId << "\ntaskName: " << metaData.taskName << "\n";
        std::cout << "logId: " << metaData.eventId << "\nfile: " << macroData.file << "\nline: " << macroData.line << "\n";
        std::cout << "formatStr: " << macroData.format << "\n";
        for (int i = 0; i < metaData.fieldNames.size(); ++i)
            std::cout << metaData.fieldNames[i] << " : " << metaData.fieldTypes[i] << "\n";

        std::cout << "keywords: ";
        for (const auto& k : metaData.keywords)
            std::cout << k << ", ";
        std::cout << "\n-------------------------\n";

    }
    std::cout << "\n";
}

TEST_CASE("test logging", "[etw][logging]")
{
    //static constexpr std::string_view f = "some {0}\n";
    //console << fmt::format(FMT_STRING(f), 1);
    testLogRegistry();

    LOG("This is some logging with value: {value}", 1);
} 
