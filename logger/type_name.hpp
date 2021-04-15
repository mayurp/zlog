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

#if (_MSC_VER < 1921)
#	error When using Microsoft (R) C/C++ Optimizing Compiler, at least version 19.21 is required.
#endif

template<typename T>
struct type_name
{
private:
	static constexpr auto get() noexcept
	{
		constexpr std::string_view full_name{ __FUNCSIG__ };
		constexpr std::string_view left_marker{ "type_name<" };
		constexpr std::string_view right_marker{ ">::get(" };

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

#endif /* type_name_h */