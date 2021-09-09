//
//  type_name.hpp
//  template_fun
//
//  Created by Mayur Patel on 11/04/2021.
//  Copyright © 2021 Mayur Patel. All rights reserved.
//

#ifndef type_name_h
#define type_name_h

#include <string_view>

template<typename T>
struct type_name
{
private:
    static constexpr auto get() noexcept
    {
        constexpr std::string_view full_name{ __PRETTY_FUNCTION__ };
        constexpr std::string_view left_marker{ "[T = " };
        constexpr std::string_view right_marker{ "]" };

        constexpr auto left_marker_index = full_name.find(left_marker);
        static_assert(left_marker_index != std::string_view::npos);
        constexpr auto start_index = left_marker_index + left_marker.size();
        constexpr auto end_index = full_name.find(right_marker, left_marker_index);
        static_assert(end_index != std::string_view::npos);
        constexpr auto length = end_index - start_index;

        return full_name.substr(start_index, length);
    }

public:
    using value_type = std::string_view;
    static constexpr value_type value{ get() };

    constexpr operator value_type() const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
};

template<typename T>
inline constexpr auto type_name_v = type_name<T>::value;

#define STRIGIFY(X) #X

#define TYPE_NAME_VALUE(TYPE, NAME)\
template<>          \
struct type_name<TYPE> \
{                   \
    using value_type = std::string_view;      \
    static constexpr value_type value{ NAME }; \
    constexpr operator value_type() const noexcept { return value; }  \
    constexpr value_type operator()() const noexcept { return value; }\
};\

#define TYPE_NAME(TYPE) TYPE_NAME_VALUE(TYPE, STRIGIFY(TYPE))


TYPE_NAME(uint32_t)
TYPE_NAME_VALUE(std::string, "string")
TYPE_NAME_VALUE(std::string_view, "string")
TYPE_NAME_VALUE(char*, "string")
TYPE_NAME_VALUE(const char*, "string")
TYPE_NAME_VALUE(int, "int64_t")


constexpr std::string_view className(const std::string_view& prettyFuntion)
{
    const size_t colonsPos = prettyFuntion.rfind("::");
    if (colonsPos == std::string_view::npos)
        return "";
    return prettyFuntion.substr(0, colonsPos);
}

constexpr std::string_view funcName(const std::string_view& prettyFuntion)
{
    const size_t colonsPos = prettyFuntion.rfind("::");
    if (colonsPos == std::string_view::npos)
        return prettyFuntion;
    return prettyFuntion.substr(colonsPos+2);
}


#endif /* type_name_h */
