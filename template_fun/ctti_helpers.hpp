//
//  ctti_helpers.h
//  template_fun
//
//  Created by Mayur Patel on 01/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef ctti_helpers_h
#define ctti_helpers_h

#include <ctti/type_id.hpp>

inline constexpr std::string_view cstring2view(const ctti::detail::cstring& str)
{
    return std::string_view(str.begin(), str.length());
}

namespace std
{
    constexpr const char* ctti_nameof(ctti::type_tag<std::string_view>)
    {
         return "std::string_view"; // instead of "std::__foobar::basic_string<char>"
    }
}

#endif /* ctti_helpers_h */
