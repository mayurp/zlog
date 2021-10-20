//
//  tests.cpp
//
//  Created by Mayur Patel on 27/03/2021.
//

#include "ctf_writer.hpp"

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <list>

// Integral types
static_assert(barectf::payload_size(1llu) == 8);
static_assert(barectf::payload_size(1llu, true) == 9);
static_assert(barectf::payload_size(1u, true) == 5);
static_assert(barectf::payload_size(1u, true) == 5);


// TODO move to separate test file

void test()
{
    {
        std::array arr = {1, 2, 3};
        static_assert(is_iterable_v<decltype(arr)>);
    }
    
    {
        int arr[3] = {1, 2, 3};
        static_assert(is_iterable_v<decltype((arr))>);

        std::vector<int> vec = {1, 2, 3};
        static_assert(is_iterable_v<decltype(vec)>);
        
        std::list<int> list = {1, 2, 3};
        static_assert(is_iterable_v<decltype(list)>);
    }
}

TEST_CASE("ctf payload_size strings")
{
    const char* cstr = "abc";
    CHECK(barectf::payload_size(cstr) == 4);

    const char str_array[10] = "abc";
    CHECK(barectf::payload_size(str_array) == 4); // based on null terminated size

    const char str_array_u[] = "abc";
    CHECK(barectf::payload_size(str_array_u) == 4);

    const std::string str = "abc";
    CHECK(barectf::payload_size(str) == 4);

    const std::string_view strv = "abc";
    CHECK(barectf::payload_size(strv) == 4);

    //std::cout << barectf::payload_size(s2) << "\n";
    //std::cout << barectf::payload_size(s1, 54.f, 64., s2) << "\n";
}

TEST_CASE("ctf payload_size static arrays")
{
    // TODO: Ideally these should evaluate as constexpr and we can use static_assert instead

    SECTION("std::array")
    {
        const std::array floats = {1.f, 2.f};
        CHECK(barectf::payload_size(floats) == 8);

        const std::array bools = {true, false, true};
        CHECK(barectf::payload_size(bools) == 3);
        
        const std::array strings = {"a", "ab", "abc"};
        CHECK(barectf::payload_size(strings) == 9);
    }

    SECTION("array[]")
    {
        const float floats[] = {1.f, 2.f};
        CHECK(barectf::payload_size(floats) == 8);
    
        const bool bools[] = {true, false, true};
        CHECK(barectf::payload_size(bools) == 3);
        
        const std::string strings[]  = {"a", "ab", "abc"};
        CHECK(barectf::payload_size(strings) == 9);

    }
}
