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

struct X
{
    X() { std::cout << "construct\n";}
    X(const X&) { std::cout << "copy construct\n";}
    X(X&&) { std::cout << "move construct\n";}
    X& operator=(const X&) { std::cout << "copy assign\n"; return *this;}
    X& operator=(X&&) { std::cout << "move assign\n"; return *this;}
    
    int a;
    bool b;
};

REFLECT(X, a, b);


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

struct SomeTask
{
    static constexpr std::array keywords = {"12", "lokok"};
};

void test()
{
    LOG("abc {length} is {name}", 12.f, "jie");
    LOGTASK(SomeTask, "abc {length} is {name}", 12.f, "jie");
    LOGTASK(SomeTask, "efg {something}", true);
}

REFLECT(Config, name, id, x, calc);


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

void testMacro()
{
    Config config("abc", 22, true);

    std::cout << "before get\n";
    using ZZ = reflection::reflect_members<std::string>;
    
    using R = reflection::reflect_members<Config>;
    const X& x = R::get<2>(config);
    printf("\ncall ref member function: %d\n", x.a);

    std::cout << "before for_each\n";

    static_assert(reflection::is_reflected_v<Config>, "oh edear");

    std::cout << config << "\n";
}

int main()
{

    std::cout << "Registry\n";
    for (const auto& metaData : log::getRegistry())
    {
        const log::LogMacroData& macroData = metaData.macroData;
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
    
    std::cout << "Logging\n";
    
    testEtw();

    testMacro();

    return 0;
}
