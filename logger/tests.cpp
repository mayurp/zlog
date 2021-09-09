//
//  tests.cpp
//  template_fun
//
//  Created by Mayur Patel on 27/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#define CATCH_CONFIG_MAIN

#include "etw.hpp"

#include <iostream>
#include <tuple>
#include <vector>
#include "reflection.hpp"
#include "barectf/logger.hpp"


//#include <catch2/catch.hpp>
//
//
//TEST_CASE("parseString")
//{
//    
//}

static int someCalc()
{
    return 45;
}

void testEtw()
{
    enum class Teanum : uint32_t
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
    Teanum t = Teanum::three;
    ReflectionType f;
    X x;
    std::vector<X> v = {X()};
    std::cout << "Start etw log\n";
    logEtw(23, true, "hello", someCalc(), t, x, v, f);
    std::cout << "Stop etw log\n";
}


// TODO make unit test
static_assert(barectf::payload_size(1llu, true) == 9);
/*
std::cout << barectf::payload_size(1llu, true) << "\n";
std::cout << barectf::payload_size("hello") << "\n";
std::cout << barectf::payload_size(s1) << "\n";
std::cout << barectf::payload_size(s2) << "\n";
std::cout << barectf::payload_size(s1, 54.f, 64., s2) << "\n";
*/
