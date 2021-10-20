//
//  ctstring.hpp
//  logger
//
//  Created by Mayur Patel on 10/09/2021.
//

#ifndef ctstring_hpp
#define ctstring_hpp

#include <array>
#include <string_view>


// statically allocated string - not null terminated
template <size_t N>
struct FixedString
{
    constexpr std::string_view view() const { return { data, size }; }
    char data[N];
    size_t size = N; // default to entire buffer
};

#endif /* ctstring_hpp */
