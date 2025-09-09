#pragma once

#include <concepts>
#include <type_traits>

template<typename T>
concept Integer = std::is_integral_v<T>;