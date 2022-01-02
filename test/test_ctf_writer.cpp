//
//  tests.cpp
//
//  Created by Mayur Patel on 27/03/2021.
//

#include "ctf_writer.hpp"

#define CATCH_CONFIG_MAIN
//#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>


// Integral types
static_assert(barectf::payload_size(1llu) == 8);
static_assert(barectf::payload_size(1llu, true) == 9);
static_assert(barectf::payload_size(1u, true) == 5);
static_assert(barectf::payload_size(1u, true) == 5);


TEST_CASE("ctf payload_size strings", "[ctf]")
{
    // ctf strings are null terminated

    const char* cstr = "abc";
    CHECK(barectf::payload_size(cstr) == 4);

    const char str_array[10] = "abc";
    CHECK(barectf::payload_size(str_array) == 4);

    const char str_array_u[] = "abc";
    CHECK(barectf::payload_size(str_array_u) == 4);

    const std::string str = "abc";
    CHECK(barectf::payload_size(str) == 4);

    const std::string_view strv = "abc";
    CHECK(barectf::payload_size(strv) == 4);

    //std::cout << barectf::payload_size(s2) << "\n";
    //std::cout << barectf::payload_size(s1, 54.f, 64., s2) << "\n";
}

TEST_CASE("barectf::payload_size static arrays", "[ctf]")
{
    SECTION("std::array")
    {
        const std::array floats = {1.f, 2.f};
        CHECK(barectf::payload_size(floats) == 8);

        const std::array bools = {true, false, true};
        CHECK(barectf::payload_size(bools) == 3);
        
        const std::array strings = {"a", "ab", "abc"};
        CHECK(barectf::payload_size(strings) == 9);

        const std::array string_views = {"a", "ab", "abc"};
        CHECK(barectf::payload_size(string_views) == 9);
    }

    SECTION("array[]")
    {
        const float floats[] = {1.f, 2.f};
        CHECK(barectf::payload_size(floats) == 8);
    
        const bool bools[] = {true, false, true};
        CHECK(barectf::payload_size(bools) == 3);
        
        const std::string strings[]  = {"a", "ab", "abc"};
        CHECK(barectf::payload_size(strings) == 9);

        const std::string_view string_views[]  = {"a", "ab", "abc"};
        CHECK(barectf::payload_size(string_views) == 9);
    }
}

TEST_CASE("barectf::payload_size dynamic containers", "[ctf]")
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

        const std::vector<std::string_view> string_views = {"a", "ab", "abc"};
        CHECK(barectf::payload_size(string_views) == 13);
    }
    
    SECTION("std::map")
    {
        std::map<int32_t, bool> ints_bools = { {1, true}, {2, false} };
        CHECK(barectf::payload_size(ints_bools) == 14);

        std::map<std::string, std::string> strings_bools = { {"a", "bc"}, {"d", ""}, {"efg", "hij"} };
        CHECK(barectf::payload_size(strings_bools) == 20);
    }
}

