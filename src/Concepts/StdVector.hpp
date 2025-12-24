#pragma once

#include <vector>
#include <type_traits>

template<typename T>
struct is_std_vector_impl : std::false_type {};

template<typename T, typename Alloc>
struct is_std_vector_impl<std::vector<T, Alloc>> : std::true_type {};

template<typename T>
concept StdVector = is_std_vector_impl<std::remove_cvref_t<T>>::value;