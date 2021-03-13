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



#define LOG(STR, ...)

template <typename StringHolder, typename... Args>
auto logTest(StringHolder holder, Args&&... args)
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
    LOGID("abc {length} is {name}", 12.f, "jie");
    LOGID("efg {something}", true);
}

int main()
{

    std::cout << "Registry\n";
    for (const auto& [id, metaData] : log::getRegistry())
    {
        const log::LogMacroData& macroData = metaData.macroData;
        std::cout << "-------------------------\n";
        std::cout << "id: " << id << "\nfile: " << macroData.file << "\nline: " << macroData.line << "\n";
        std::cout << "formatStr: " << macroData.format << "\n";
        for (int i = 0; i < metaData.fieldNames.size(); ++i)
        {
            std::cout << metaData.fieldNames[i] << " : " << metaData.fieldTypes[i] << "\n";
        }
        std::cout << "-------------------------\n";

    }
    std::cout << "\n";
    
    std::cout << "Logging\n";

    test();

    return 0;
}
