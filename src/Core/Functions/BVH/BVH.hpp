#pragma once

#include <cstddef>
#include <algorithm>
#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include "Classes/AABB/AABB.hpp"
#include "Structs/BVHNode.hpp"
#include "Classes/Object/Object.hpp"

namespace nihil
{
    size_t buildBVH(std::vector<AABB>& primitives, std::vector<size_t>& indices, size_t start, size_t end, Carbo::ECSAllocator<BVHNode>& allocator);
    size_t buildBVH(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, Carbo::ECSAllocator<BVHNode>& allocator);
    void cullBVH(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVHNode>& allocator, std::vector<size_t>& visible);
}