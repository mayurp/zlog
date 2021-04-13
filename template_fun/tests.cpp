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

