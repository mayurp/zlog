//
//  type_traits.hpp
//  logger
//
//  Created by Mayur Patel on 11/09/2021.
//

#include <array>
#include <type_traits>
#include <iterator>

#ifndef type_traits_hpp
#define type_traits_hpp


template<typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
struct is_std_array : std::false_type {};
  
template<typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template<typename T>
inline constexpr bool is_std_array_v = is_std_array<T>::value;

template<typename T>
inline constexpr bool is_static_array_v = is_std_array_v<T> || std::is_array_v<T>;

template<typename>
struct array_size;

template<typename T, size_t N>
struct array_size<std::array<T,N>>
{
    static constexpr size_t size = N;
};

template<typename T>
inline constexpr bool is_string_v = std::disjunction_v<
                        std::is_same<char *, std::decay_t<T>>,
                        std::is_same<const char *, std::decay_t<T>>,
                        std::is_same<std::string, std::decay_t<T>>,
                        std::is_same<std::string_view, std::decay_t<T>>
                        >;

template<typename T, typename _ = void>
struct is_container : std::false_type {};

template<typename... Ts>
struct is_container_helper {};

template<typename T>
struct is_container<
        T,
        std::conditional_t<
            false,
            is_container_helper<
                typename T::value_type,
                typename T::size_type,
                typename T::allocator_type,
                typename T::iterator,
                typename T::const_iterator,
                decltype(std::declval<T>().size()),
                decltype(std::declval<T>().begin()),
                decltype(std::declval<T>().end()),
                decltype(std::declval<T>().cbegin()),
                decltype(std::declval<T>().cend())
                >,
            void
            >
        > : public std::true_type {};


template<typename T>
inline constexpr bool is_container_v = is_container<T>::value;

template <typename T, typename = void>
inline constexpr bool is_iterable_v = false;
 
template <typename T>
inline constexpr bool is_iterable_v<T, std::void_t<decltype(std::begin(std::declval<T>())),
                                                   decltype(std::end(std::declval<T>()))>>
                                       = true;


template <typename T, typename = void>
inline constexpr bool is_map_v = false;
 
template <typename T>
inline constexpr bool is_map_v<T,
                                    std::void_t<decltype(std::declval<T>().begin(), std::declval<T>().end()),
                                                typename T::key_type,
                                                typename T::mapped_type
                                                >>
                                    = true;


template <typename T>
inline constexpr bool is_tuple_v = false;

template <typename... U>
inline constexpr bool is_tuple_v<std::tuple<U...>> = true;

template <typename T, typename Char, typename = void>
inline constexpr bool is_range_v = false;

template <typename T, typename Char>
inline constexpr bool is_range_v<T, Char> = is_iterable_v<T> && !std::is_convertible<T, std::basic_string<Char>>::value && !std::is_constructible<std::basic_string_view<Char>, T>::value;

template <typename T>
inline constexpr bool is_pair_v = false;

template <typename T, typename U>
inline constexpr bool is_pair_v<std::pair<T,U>> = true;

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
