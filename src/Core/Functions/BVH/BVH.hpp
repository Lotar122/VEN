#pragma once

#include <cstddef>
#include <algorithm>
#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include "Classes/AABB/AABB.hpp"
#include "Structs/BVHNode.hpp"
#include "Classes/Object/Object.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace nihil
{
    size_t buildBVH2(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, size_t parent, Carbo::ECSAllocator<BVH2Node>& allocator);
    void cullBVH2(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVH2Node>& allocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack = nullptr);

    size_t buildBVH4(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, size_t parent, Carbo::ECSAllocator<BVH4Node>& allocator, Carbo::ECSAllocator<BVH4LeafNode>& leafAllocator);
    void cullBVH4(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVH4Node>& allocator, Carbo::ECSAllocator<BVH4LeafNode>& leafAllocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack = nullptr);

    //returns the "cost" or how bad the tree becomes after this
    float refitBVH2(const std::vector<graphics::Object*>& primitives, graphics::Object* object, Carbo::ECSAllocator<BVH2Node>& allocator);
}