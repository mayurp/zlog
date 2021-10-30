//
//  type_name.hpp
//
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
    
    static constexpr std::string_view pretty_name(std::string_view name) noexcept
    {
        for (std::size_t i = name.size(); i > 0; --i)
        {
            char c = name[i - 1];
            if (!((c >= '0' && c <= '9') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c == '_')))
            {
                name.remove_prefix(i);
                break;
            }
        }

        if (name.size() > 0 && ((name.front() >= 'a' && name.front() <= 'z') ||
                                (name.front() >= 'A' && name.front() <= 'Z') ||
                                (name.front() == '_')))
        {
            return name;
        }

        return {}; // Invalid name.
    }
    
    static constexpr auto get() noexcept
    {
#if defined(__clang__) || defined(__GNUC__)
        constexpr auto name = pretty_name({__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 2});
#elif defined(_MSC_VER)
        constexpr auto name = pretty_name({__FUNCSIG__, sizeof(__FUNCSIG__) - 17});
#else
#       error unsupported C++ compiler
#endif
        return name;
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


// Ensure we use explicit names not "int" etc
TYPE_NAME(int8_t)
TYPE_NAME(int16_t)
TYPE_NAME(int32_t)
TYPE_NAME(int64_t)

TYPE_NAME(uint8_t)
TYPE_NAME(uint16_t)
TYPE_NAME(uint32_t)
TYPE_NAME(uint64_t)


TYPE_NAME_VALUE(std::string, "string")
TYPE_NAME_VALUE(std::string_view, "string")
TYPE_NAME_VALUE(char*, "string")
TYPE_NAME_VALUE(const char*, "string")


namespace type_helper
{

constexpr std::string_view class_name(const std::string_view& prettyFunction)
{
    const size_t colonsPos = prettyFunction.rfind("::");
    if (colonsPos == std::string_view::npos)
        return "";
    return prettyFunction.substr(0, colonsPos);
}

constexpr std::string_view func_name(const std::string_view& prettyFunction)
{
    const size_t colonsPos = prettyFunction.rfind("::");
    if (colonsPos == std::string_view::npos)
        return prettyFunction;
    return prettyFunction.substr(colonsPos+2);
}

}

#endif /* type_name_h */
