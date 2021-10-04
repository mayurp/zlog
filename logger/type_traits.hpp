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

template<typename T>
inline constexpr bool is_string_v = std::disjunction_v<
                        std::is_same<char *, std::decay_t<T>>,
                        std::is_same<const char *, std::decay_t<T>>,
                        std::is_same<std::string, std::decay_t<T>>,
                        std::is_same<std::string_view, std::decay_t<T>>
                        >;

template<typename T>
struct explicit_int_type
{
    //static_assert(std::is_integral_v<T>);
    static constexpr auto deduce()
    {
       if constexpr (std::is_signed_v<T>)
       {
           if constexpr (sizeof(T) == 1) return int8_t();
           if constexpr (sizeof(T) == 2) return int16_t();
           if constexpr (sizeof(T) == 4) return int32_t();
           if constexpr (sizeof(T) == 8) return int64_t();
       }
       else
       {
           if constexpr (sizeof(T) == 1) return uint8_t();
           if constexpr (sizeof(T) == 2) return uint16_t();
           if constexpr (sizeof(T) == 4) return uint32_t();
           if constexpr (sizeof(T) == 8) return uint64_t();
       }
           
    };
    using type = decltype(deduce());
};

template<typename T>
using explicit_int_type_t = typename explicit_int_type<T>::type;

#endif /* type_traits_hpp */
