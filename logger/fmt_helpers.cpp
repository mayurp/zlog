//
//  format_helpers.cpp
//  template_fun
//
//  Created by Mayur Patel on 12/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#include "fmt_helpers.hpp"

namespace fmt_helpers
{

// append to char array if not nullpt, otherwise just increment dst
constexpr void append(char arr[], int& dst, const std::string_view& str)
{
    for (char c : str)
    {
        if (arr)
            arr[dst] = c;
        ++dst;
    }
}

}
