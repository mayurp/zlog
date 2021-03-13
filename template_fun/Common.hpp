#pragma once

// type helpers
template <class C, typename T>
T getPointerType(T C::*v);

template <class C, typename T>
C getClassType(T C::*v);

template<class T>
struct is_shared_ptr : std::false_type {};

template<class T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

template<int N, typename... Ts> using NthTypeOf =
typename std::tuple_element<N, std::tuple<Ts...>>::type;




/// stuff this in a header somewhere
inline int type_id_seq = 0;
template< typename T > inline const int type_id = type_id_seq++;

