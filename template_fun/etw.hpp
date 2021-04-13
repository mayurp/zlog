//
//  etw.hpp
//  template_fun
//
//  Created by Mayur Patel on 14/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef etw_h
#define etw_h

#include <type_traits>
#include <utility>

#define FMT_ENFORCE_COMPILE_STRING
#include <fmt/format.h>



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

template<typename T>
decltype(auto) convertT(T&& value)
{
    using UT = std::remove_cv_t<
        std::remove_reference_t<decltype(value)>
    >;
    if constexpr (std::is_enum_v<UT>)
    {
        static_assert(sizeof(UT) <= 4, "enum larger than 4 bytes not supported by ETW");
        // TODO: Enums are signed by default, does ETW really not support them?
        // Casting to unsgined int might be fine
        static_assert(std::is_unsigned_v<std::underlying_type_t<UT>>, "signed enums not supported by ETW");
        return static_cast<uint32_t>(value);
    }
    else if constexpr (std::is_same_v<UT, bool>)
        return static_cast<uint32_t>(value);
    else if constexpr (std::is_same_v<UT, ReflectionType>)
        return value.hash();
    else
        return std::forward<decltype(value)>(value);
};

template <typename Arg>
void pushArg(Event& event, Arg&& arg)
{
    event.data = (void*) &arg;
    event.size = sizeof(arg);
}

template <typename Args, size_t... Is>
void logEtwImpl(Args&& args, std::index_sequence<Is...>)
{
    Event events[sizeof... (Is)];
    (pushArg(events[Is], std::get<Is>(args)), ...);
    for (const Event& e : events)
    {
        //fmt::print(FMT_STRING("Event data: {}, size: {}\n"), e.data, e.size);
        //std::cout << "Event data: " << e.data << ", size: " << e.size << std::endl;
    }
}

template <typename... Args>
void logEtw(Args&&... args)
{
    const auto argsTuple = std::forward_as_tuple(convertT(std::forward<Args>(args))... );
    logEtwImpl(argsTuple, std::index_sequence_for<Args...>{});
}

void testEtw();

#endif /* etw_h */
