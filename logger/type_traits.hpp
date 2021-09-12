//
//  type_traits.hpp
//  logger
//
//  Created by Mayur Patel on 11/09/2021.
//

#include <type_traits>

#ifndef type_traits_hpp
#define type_traits_hpp


template<typename T>
struct is_std_array : std::false_type {};
  
template<typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template<typename T>
inline constexpr bool is_std_array_v = is_std_array<T>::value;


#endif /* type_traits_hpp */
