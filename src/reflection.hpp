//
//  reflection.hpp
//  template_fun
//
//  Created by Mayur Patel on 27/03/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef reflection_h
#define reflection_h

#include "map_macro.hpp"
#include "type_name.hpp"
#include "type_traits.hpp"

#include "magic_enum.hpp"

#include <vector>
#include <variant>


namespace reflection
{

// Retrieve the nth value in a list of values
template <size_t n, auto... vs> struct nth_value;
template <size_t n, auto v1, auto... vs>
struct nth_value<n, v1, vs...>
{
    static constexpr auto value = nth_value<n - 1, vs...>::value;
};
template <auto v1, auto... vs>
struct nth_value<0, v1, vs...>
{
    static constexpr auto value = v1;
};

template<typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <size_t I, typename T, auto... members>
constexpr decltype(auto) getImpl(const T& t)
{
    constexpr auto member = nth_value<I, members...>::value;
    using Member = decltype(member);
    static_assert(std::is_member_pointer_v<Member>, "only reflection of members is supported");
    if constexpr (std::is_member_object_pointer_v<Member>)
        return t.*member;
    else // (std::is_member_function_pointer_v<Member>)
        return (t.*member)();
}

template<typename T>
struct reflect_members
{};

// helper to pass member type info to for_each callback
template<typename T, size_t Idx>
struct MemberInfo
{
    using ValueType = remove_cvref_t<T>;
    static constexpr size_t Index = Idx;
};

template <typename T, typename F, std::size_t... Idx>
constexpr void for_each(T&& t, F&& f, std::index_sequence<Idx...>)
{
    // TODO is this needed?
    using R = reflect_members<remove_cvref_t<T>>;
    (f(R::names[Idx], R:: template get<Idx>(t)), ...);
}

template<typename T, typename F>
constexpr void for_each(T&& t, F&& f)
{
    // TODO is this needed?
    using R = reflect_members<remove_cvref_t<T>>;
    for_each(std::forward<T>(t), std::forward<F>(f), std::make_index_sequence<R::N>{});
}

template <typename T, typename F, std::size_t... Idx>
constexpr void for_each(F&& f, std::index_sequence<Idx...>)
{
    // TODO is this needed?
    using R = reflect_members<remove_cvref_t<T>>;
    (f(R::names[Idx], MemberInfo<decltype(R::template get<Idx>(std::declval<T>())), Idx>()), ...);
}

template<typename T, typename F>
constexpr void for_each(F&& f)
{
    // TODO is this needed?
    using R = reflect_members<remove_cvref_t<T>>;
    for_each<T>(std::forward<F>(f), std::make_index_sequence<R::N>{});
}

template <typename T, typename = void>
constexpr bool is_reflected_v = false;

template <typename T>
constexpr bool is_reflected_v
    <T, std::void_t<decltype(reflection::reflect_members<T>::is_reflected)>> = true;

// Type registry populated on static initialisation

struct Primitive
{
    std::string_view name;
};

struct Enum
{
    std::string_view integerType;
    struct Field
    {
        std::string_view name;
        int value;
    };
    std::string_view name;
    std::vector<Field> fields;
};

struct Clazz;
struct Array;
struct Map;
struct Tuple;

template<typename T>
struct recursive_wrapper
{
  // construct from an existing object
  //recursive_wrapper(T t_) { t.emplace_back(std::move(t_)); }
  recursive_wrapper(const T& t_) { t.emplace_back(t_); }
  recursive_wrapper(const recursive_wrapper& w) = default;
  recursive_wrapper& operator=(const recursive_wrapper &) = default;
  recursive_wrapper& operator=(const T& t_) { t.emplace_back(t_); };

  // cast back to wrapped type
  operator const T &() const { return t.front(); }
  // store the value
  std::vector<T> t;
};

using Type = std::variant<Primitive, Enum, recursive_wrapper<Clazz>, recursive_wrapper<Array>,
                            recursive_wrapper<Map>, recursive_wrapper<Tuple>
                            >;


struct Clazz
{
    struct Field
    {
        std::string_view name;
        Type type;
    };
    std::string_view name;
    std::vector<Field> fields;
};

struct Array
{
    Type valueType;
    bool isDynamic = false;
    uint32_t size = 0; // only used for static arrays
};

struct Map
{
    Type keyType;
    Type valueType;
};

struct Tuple
{
    std::vector<Type> types;
};

using RegisteredType = std::variant<Clazz, Enum>;

std::vector<RegisteredType>& get_type_registry();


// transform type to one suitable for serialisation
// e.g int -> int32_t
//     SomeType& -> SomeType
template<typename T>
struct serialised_type
{
private:
    using U = remove_cvref_t<T>;
public:
    using type = typename std::conditional_t<
        std::is_integral_v<U>,
        explicit_int_type_t<U>, U>;
};

template<typename T>
using serialised_type_t = typename serialised_type<T>::type;

template<typename T>
reflection::Type make_reflection_type()
{
    using U = serialised_type_t<T>;
    static constexpr std::string_view name = type_name_v<U>;
    if constexpr (std::is_enum_v<U>)
    {
        reflection::Enum e;
        e.name = name;
        e.integerType = type_name_v<typename magic_enum::underlying_type<T>::type>;
        for (const auto value : magic_enum::enum_values<T>())
        {
            e.fields.push_back({ magic_enum::enum_name(value), magic_enum::enum_integer(value) });
        }
        // Only enums will get auto registered when they are used my LOG macros
        // Classes need to be explicitly registered
        // TODO: ensure enums are only registered once
        reflection::get_type_registry().push_back(e);
        return e;
    }
    else if constexpr (std::is_arithmetic_v<U> || is_string_v<U>)
    {
        reflection::Primitive prim{name};
        return prim;
    }
    // std::array
    else if constexpr (is_std_array_v<U>)
    {
        reflection::Array array;
        array.isDynamic = false;
        using VT = serialised_type_t<typename U::value_type>;
        array.valueType = make_reflection_type<VT>();
        array.size = array_size<U>::size;
        return array;
    }
    // [] array
    else if constexpr (std::is_array_v<U>)
    {
        reflection::Array array;
        array.isDynamic = false;
        using VT = serialised_type_t<std::remove_all_extents_t<U>>;
        array.valueType = make_reflection_type<VT>();
        array.size = std::extent_v<U>;
        return array;
    }
    // dynamic map
    else if constexpr (is_map_v<U>)
    {
        reflection::Map map;
        using KT = serialised_type_t<typename U::key_type>;
        using VT = serialised_type_t<typename U::mapped_type>;
        map.keyType = make_reflection_type<KT>();
        map.valueType = make_reflection_type<VT>();
        return map;
    }
    // dynamic arrays, lists etc
    else if constexpr (is_iterable_v<U>)
    {
        reflection::Array array;
        array.isDynamic = true;
        using VT = serialised_type_t<typename U::value_type>;
        array.valueType = make_reflection_type<VT>();
        return array;
    }
    else if constexpr (is_pair_v<U>)
    {
        reflection::Tuple tuple;
        using T1 = serialised_type_t<typename U::first_type>;
        using T2 = serialised_type_t<typename U::second_type>;
        tuple.types.emplace_back(make_reflection_type<T1>());
        tuple.types.emplace_back(make_reflection_type<T2>());
        return tuple;
    }
    else
    {
        // Fields not populated here since we only need the typename
        // TODO: make this nicer
        reflection::Clazz clazz{name};
        return clazz;
    }
}

template<typename T>
struct RegisterType
{
    RegisterType()
    {
        static_assert(reflection::is_reflected_v<T>, "Type is not reflected");
        Clazz type;
        type.name = type_name<T>();
        reflection::for_each<T>([&](const std::string_view& memberName, auto memberInfo)
        {
            using ValueType = typename decltype(memberInfo)::ValueType;
            type.fields.push_back({memberName, make_reflection_type<ValueType>()});
        });
        get_type_registry().push_back(type);
    }
};

} // namespace reflection

#define FIELD(x) x
#define STRING(x) #x

#define REFLECT(STRUCT_NAME, ...)          \
namespace reflection {\
template<>\
struct reflect_members<STRUCT_NAME>\
{\
    static constexpr bool is_reflected = true;\
    static constexpr std::string_view name = #STRUCT_NAME; \
    static constexpr std::array names = {MAP_LIST(STRING, __VA_ARGS__)}; \
    static constexpr size_t N = names.size(); \
    template <size_t I>                        \
    static constexpr decltype(auto) get(const STRUCT_NAME& c)  \
    {                                          \
        return getImpl<I, STRUCT_NAME, MAP_LIST(&STRUCT_NAME::FIELD, __VA_ARGS__)>(c);  \
    }\
};\
RegisterType<STRUCT_NAME> reg_##STRUCT_NAME ;\
}\

#endif /* reflection_h */
