//
//  tests.cpp
//
//
//  Created by Mayur Patel on 27/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#include "ctf_writer.hpp"

//#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>




// TODO make unit test
static_assert(barectf::payload_size(1llu, true) == 9);
//std::cout << barectf::payload_size(s1) << "\n";
//std::cout << barectf::payload_size(s2) << "\n";
//std::cout << barectf::payload_size(s1, 54.f, 64., s2) << "\n";
