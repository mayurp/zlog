//
//  tests.cpp
//
//  Created by Mayur Patel on 27/03/2021.
//

#include "log.hpp"
#include "ctf_writer.hpp"

#define CATCH_CONFIG_MAIN
//#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <list>

// Integral types
static_assert(barectf::payload_size(1llu) == 8);
static_assert(barectf::payload_size(1llu, true) == 9);
static_assert(barectf::payload_size(1u, true) == 5);
static_assert(barectf::payload_size(1u, true) == 5);


// TODO move to separate test file
void test_type_traits()
{
    std::array sarr = {1, 2, 3};
    static_assert(is_iterable_v<decltype(sarr)>);

    int arr[3] = {1, 2, 3};
    static_assert(is_iterable_v<decltype((arr))>);

    std::vector<int> vec = {1, 2, 3};
    static_assert(is_iterable_v<decltype(vec)>);

    std::vector<bool> vecBools = {true, true, true};
    static_assert(is_iterable_v<decltype(vecBools)>);

    std::list<int> list = {1, 2, 3};
    static_assert(is_iterable_v<decltype(list)>);
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

TEST_CASE("barectf::payload_size static arrays")
{
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

TEST_CASE("barectf::payload_size dynamic containers")
{
    // extra 4 bytes for storing size
    SECTION("std::vector")
    {
        const std::vector<float> floats = {1.f, 2.f};
        CHECK(barectf::payload_size(floats) == 12);

        const std::vector<bool> bools = {true, false, true};
        CHECK(barectf::payload_size(bools) == 7);
        
        const std::vector<std::string> strings = {"a", "ab", "abc"};
        CHECK(barectf::payload_size(strings) == 13);
    }
}

// Compile time checks
struct NonCopyable
{
    NonCopyable(int _a) : a(_a) {}
    NonCopyable(const NonCopyable& x) = delete; // : a(x.a) { std::cout << "copy construct\n";}
    NonCopyable(NonCopyable&& x) = delete; // : a(x.a) { std::cout << "move construct\n";}
    NonCopyable& operator=(const NonCopyable& x)  = delete; //  { a = x.a; std::cout << "copy assign\n"; return *this;}
    NonCopyable& operator=(NonCopyable&& x )  = delete; //  { a = x.a; std::cout << "move assign\n"; return *this;}
    
    int a = 0;
};
REFLECT(NonCopyable, a)

TEST_CASE("logging nocopy")
{
    NonCopyable n(1);
    LOGD("test no copy lvalue: {value}", n);
    LOGD("test no copy rvalue: {value}", NonCopyable(1));
}
