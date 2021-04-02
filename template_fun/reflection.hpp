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
#include "ctti_helpers.hpp"


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
    (f(R::names[Idx], decltype(R::template get<Idx>(std::declval<T>())){}), ...);
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
struct Type
{
    struct Field
    {
        std::string_view name;
        std::string_view typeName;
    };

    std::string_view name;
    std::vector<Field> fields;
};

static inline std::vector<Type>& getTypeRegistry()
{
    static std::vector<Type> registry;
    return registry;
}

template<typename T>
struct RegisterType
{
    RegisterType()
    {
        static_assert(reflection::is_reflected_v<T>, "Type is not reflected");
        Type type;
        type.name = cstring2view(ctti::nameof<T>());
        reflection::for_each<T>([&](const std::string_view& name, const auto& value)
        {
            using Value = remove_cvref_t<decltype(value)>;
            type.fields.push_back({name, cstring2view(ctti::nameof<Value>())});
        });
        getTypeRegistry().push_back(type);
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
