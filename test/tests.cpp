//
//  tests.cpp
//  template_fun
//
//  Created by Mayur Patel on 27/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#define CATCH_CONFIG_MAIN

#include <iostream>
#include <tuple>
#include <vector>
#include "reflection.hpp"
#include "ctf/ctf_logger.hpp"


// TODO make unit test
static_assert(barectf::payload_size(1llu, true) == 9);
//std::cout << barectf::payload_size(s1) << "\n";
//std::cout << barectf::payload_size(s2) << "\n";
//std::cout << barectf::payload_size(s1, 54.f, 64., s2) << "\n";
