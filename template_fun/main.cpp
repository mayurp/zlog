#include "somewhere.hpp"

#include "log.hpp"
#include "etw.hpp"
#include "reflection.hpp"

#include <array>
#include <type_traits>
#include <string_view>
#include <string>
#include <iostream>
#include <ostream>
#include <sstream>

#include <ctti/type_id.hpp>
#define FMT_ENFORCE_COMPILE_STRING
#include <fmt/format.h>
#include "fmt_helpers.hpp"


struct Vec3
{
    float x, y, z;
};
REFLECT_WITH_FORMATTER(Vec3, x, y, z)

struct X
{
    X() { std::cout << "construct\n";}
    X(const X&) { std::cout << "copy construct\n";}
    X(X&&) { std::cout << "move construct\n";}
    X& operator=(const X&) { std::cout << "copy assign\n"; return *this;}
    X& operator=(X&&) { std::cout << "move assign\n"; return *this;}
    
    int a = 0;
    bool b = true;
};
REFLECT_WITH_FORMATTER(X, a, b)


class Config {
public:
    std::string_view name;
    int id;
    bool data;
    X x;
    
    const X& calc() const
    {
        return x;
    }

public:
   Config(const std::string_view& _name, int _id, bool _data)
    :name(_name), id(_id), data(_data){}
    
    // TODO how do we get rid of this?
    template <std::size_t I> friend decltype(auto) get(const Config&);
};
REFLECT_WITH_FORMATTER(Config, name, id, x, calc)


struct SomeTask
{
    static constexpr std::array keywords = {"12", "lokok"};
};

template<typename T, typename = std::enable_if_t<reflection::is_reflected_v<T>>>
std::ostream& operator<<(std::ostream& os, const T& t)
{
    os << "{\n";
    reflection::for_each(t, [&](std::string_view name, const auto& v)
    {
        os << name << ": " << v << "\n";
    });
    os << "}";
    return os;
}

void testReflection()
{
    std::cout << "\n------- testReflection --------\n";

    Config config("abc", 22, true);

    std::cout << "before get\n";
    using ZZ = reflection::reflect_members<std::string>;
    
    using R = reflection::reflect_members<Config>;
    const X& x = R::get<2>(config);
    printf("\ncall ref member function: %d\n", x.a);

    std::cout << "before for_each\n";

    static_assert(reflection::is_reflected_v<Config>, "oh edear");

    std::cout << config << "\n";
    
    std::cout << "\n------- end testReflection --------\n";

}

void testTypeRegistry()
{
    std::cout << "\n------- testTypeRegistry --------\n";
    for (const auto& type: reflection::getTypeRegistry())
    {
        std::cout << "struct " << type.name << "\n{\n";
        for (const auto& field: type.fields)
        {
            std::cout << "  " << field.name << ":\t" << field.typeName << "\n";
        }
        std::cout << "}\n\n";
    }
    std::cout << "\n------- end testTypeRegistry --------\n";

}

void testFormat()
{
    X x;
    std::vector<X> xs = {X(), X() , X()};
    Config config("abc", 22, true);

    std::cout << "---\n";
    std::cout << fmt::format(
        FMT_STRING("Some Vec3: {}\n some X: {}\n some vector: {}\n some Config: {}\n"),
        Vec3{3, 4, 5}, x, xs, config);
    std::cout << "---\n";
}

void testLogRegistry()
{
    std::cout << "Log Registry\n";
    for (const auto& metaData : logging::getRegistry())
    {
        const logging::LogMacroData& macroData = metaData.macroData;
        std::cout << "-------------------------\n";
        std::cout << "taskId: " << metaData.taskId << "\ntaskName: " << metaData.taskName << "\n";
        std::cout << "logId: " << metaData.logId << "\nfile: " << macroData.file << "\nline: " << macroData.line << "\n";
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

void testLogs()
{
    std::cout << "\n------- testLogs --------\n";

    X x;
    std::vector<X> xs = {X(), X() , X()};
    Config config("abc", 22, true);

    std::cout << "\n------- log calls start --------\n";

    LOG("First line: {name} is {length}cm long", "jie", 12.f);
    LOGTASK(SomeTask, "x: {x} is {config}", x, config);
    LOGTASK(SomeTask, "v: {position}", Vec3{2, 10, -1});

    std::cout << "\n------- log calls end --------\n";
    std::cout << "\n------- testLogs --------\n";
}

int main()
{
//    testLogRegistry();
//    testEtw();
//
//    testReflection();
//
//    testTypeRegistry();
//
//    testFormat();

    testLogs();

    return 0;
}
