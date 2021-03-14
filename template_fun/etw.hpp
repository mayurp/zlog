//
//  etw.hpp
//  template_fun
//
//  Created by Mayur Patel on 14/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef etw_h
#define etw_h

struct Event
{
    void* data = nullptr;
    size_t size = 0;
};


class ReflectionType
{
public:
    uint32_t hash()
    {
        return 38383838;
    }
    char buf[256];
};


template <typename Arg>
void pushArg(Event& event, Arg&& arg)
{
    event.data = (void*) &arg;
    event.size = sizeof(arg);
}

template <typename... Args>
void logEtwImpl(Args&&... args)
{
    constexpr int N = sizeof...(Args);
    Event events[N];
    int i = 0;
    (pushArg(events[i++], std::forward<Args>(args)), ...);
    for (const Event& e : events)
    {
        std::cout << "Event data: " << e.data << ", size: " << e.size << std::endl;
    }
}

template<typename T>
decltype(auto) convertT(T&& value)
{
    using UT = std::remove_cv_t<
        std::remove_reference_t<decltype(value)>
    >;
    if constexpr (std::is_enum_v<UT>)
        return static_cast<uint32_t>(value);
    else if constexpr (std::is_same_v<UT, bool>)
        return static_cast<uint32_t>(value);
    else if constexpr (std::is_same_v<UT, ReflectionType>)
        return value.hash();
    else
        return std::forward<decltype(value)>(value);
};

template <typename... Args>
void logEtw(Args&&... args)
{
    auto tuple = std::forward_as_tuple(convertT(std::forward<Args>(args))... );
    std::apply([](auto &&... args) { logEtwImpl(args...); }, tuple);
}

int someCalc()
{
    return 45;
}

void testEtw()
{
    enum Teanum : uint8_t
    {
        one,
        two,
        three
    };
    
    struct X
    {
        X() { std::cout << "construct\n";}
        X(const X&) { std::cout << "copy construct\n";}
        X(X&&) { std::cout << "move construct\n";}
        X& operator=(const X&) { std::cout << "copy assign\n"; return *this;}
        X& operator=(X&&) { std::cout << "move assign\n"; return *this;}
    };
    Teanum t = three;
    ReflectionType f;
    X x;
    std::vector<X> v = {X()};
    std::cout << "Start log\n";
    logEtw(23, true, "hello", someCalc(), t, x, v, f);
}

#endif /* etw_h */
