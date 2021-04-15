#include "log.hpp"
#include "etw.hpp"
#include "reflection.hpp"

#include <array>
#include <type_traits>
#include <set>
#include <map>
#include <string_view>
#include <string>
#include <iostream>
#include <ostream>
#include <sstream>

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
    X(int x) { std::cout << "construct\n";}
    X(const X&) { std::cout << "copy construct\n";}
    X(X&&) { std::cout << "move construct\n";}
    X& operator=(const X&) { std::cout << "copy assign\n"; return *this;}
    X& operator=(X&&) { std::cout << "move assign\n"; return *this;}
    
    int a = 0;
    bool b = true;
};
REFLECT_WITH_FORMATTER(X, a, b)

bool operator<(const X& lhs, const X& rhs)
{
    return lhs.a < rhs.a;
}

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
    :name(_name), id(_id), data(_data), x(1){}
    
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
    X x(1);
    std::vector<X> xs = {X(1), X(2) , X(3)};
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

    X x(1);
    std::vector<X> xs = {X(1), X(2) , X(3)};

    std::array xs_a =  {X(1), X(2) , X(3)};
    std::set<X> xs_s(xs.begin(), xs.end());
    std::map<std::string, X> xs_map = {{"a", X(1)}};

    static_assert(is_range_v<decltype(xs_map), char>, "ss");
    std::tuple<char, int, float> t{'a', 1, 2.0f};

    Config config("abc", 22, true);

    std::cout << "\n------- log calls start --------\n";

    LOG("1st line -- {name} is {length}cm long", "jie", 12.f);
    LOGTASK(SomeTask, "2nd Line -- x: {x} is {config}", x, config);
    LOGTASK(SomeTask, "3rd Line -- v: {position}", Vec3{2, 10, -1});
    LOGTASK(SomeTask, "4th Line -- xs vector: {xs}", xs);
    LOGTASK(SomeTask, "5th Line -- xs array: {xs_a}", xs_a);
    LOGTASK(SomeTask, "6th Line -- xs set: {xs_a}", xs_s);
    //LOGTASK(SomeTask, "7th Line -- xs map: {xs_map}", xs_map);
    LOGTASK(SomeTask, "8th Line -- tuple: {t}", t);

    std::cout << "\n------- log calls end --------\n";
    std::cout << "\n------- testLogs --------\n";
}

int main()
{
    std::cout << "main start\n";
//    testLogRegistry();
//    testEtw();
//
//    testReflection();
//
    testTypeRegistry();
//
//    testFormat();

    testLogs();

    std::cout << "main end\n";
    return 0;
}