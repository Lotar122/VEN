#pragma once

#include <concepts>

template<auto Mask, auto Flag>
concept HasFlag = (static_cast<uint32_t>(Mask) & static_cast<uint32_t>(Flag)) != 0;