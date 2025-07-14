#pragma once

#include <concepts>

template<typename T>
concept Integer = std::is_integral_v<T>;