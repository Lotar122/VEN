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
    // struct MortonPrimitive
    // {
    //     uint32_t morton;
    //     size_t index;
    // };

    // // ---------------- Morton helpers ----------------
    // static inline uint32_t expandBits(uint32_t v)
    // {
    //     v &= 0x000003ff;
    //     v = (v | v << 16) & 0x030000FF;
    //     v = (v | v << 8)  & 0x0300F00F;
    //     v = (v | v << 4)  & 0x030C30C3;
    //     v = (v | v << 2)  & 0x09249249;
    //     return v;
    // }

    // static inline uint32_t morton3D(const glm::vec3& p)
    // {
    //     glm::vec3 c = glm::clamp(p, glm::vec3(0.0f), glm::vec3(0.999999f));

    //     uint32_t x = expandBits((uint32_t)(c.x * 1023.0f));
    //     uint32_t y = expandBits((uint32_t)(c.y * 1023.0f));
    //     uint32_t z = expandBits((uint32_t)(c.z * 1023.0f));

    //     return x | (y << 1) | (z << 2);
    // }

    // static inline int commonPrefix(uint32_t a, uint32_t b)
    // {
    //     if (a == b) return 32;
    //     return __builtin_clz(a ^ b);
    // }

    // static size_t findSplit(
    //     const std::vector<MortonPrimitive>& morton,
    //     size_t first,
    //     size_t last)
    // {
    //     uint32_t firstCode = morton[first].morton;
    //     uint32_t lastCode  = morton[last].morton;

    //     if (firstCode == lastCode)
    //         return (first + last) >> 1;

    //     int common = commonPrefix(firstCode, lastCode);

    //     size_t split = first;
    //     size_t step = last - first;

    //     do
    //     {
    //         step = (step + 1) >> 1;
    //         size_t newSplit = split + step;

    //         if (newSplit < last)
    //         {
    //             int prefix = commonPrefix(firstCode, morton[newSplit].morton);
    //             if (prefix > common)
    //                 split = newSplit;
    //         }
    //     } while (step > 1);

    //     return split;
    // }

    // // ============================================================
    // // PLUG-AND-PLAY REPLACEMENT
    // // ============================================================
    // inline static size_t buildBVH(std::vector<nihil::graphics::Object*>& primitives,
    //                 std::vector<size_t>& indices,
    //                 size_t start,
    //                 size_t end,
    //                 Carbo::ECSAllocator<BVHNode>& allocator)
    // {
    //     const size_t count = end - start;

    //     // --------------------------------------------------------
    //     // Build scene bounds
    //     // --------------------------------------------------------
    //     AABB sceneBounds;
    //     for (size_t i = start; i < end; i++)
    //         sceneBounds.expand(primitives[indices[i]]->_transformedAABB());

    //     glm::vec3 extent = sceneBounds.max - sceneBounds.min;
    //     extent = glm::max(extent, glm::vec3(1e-6f));

    //     // --------------------------------------------------------
    //     // Build Morton list
    //     // --------------------------------------------------------
    //     std::vector<MortonPrimitive> morton(count);

    //     for (size_t i = 0; i < count; i++)
    //     {
    //         size_t idx = indices[start + i];

    //         glm::vec3 c = primitives[idx]->_transformedAABB()._centroid();
    //         glm::vec3 n = (c - sceneBounds.min) / extent;

    //         morton[i].morton = morton3D(n);
    //         morton[i].index  = idx;
    //     }

    //     // --------------------------------------------------------
    //     // Sort Morton codes (CRITICAL)
    //     // --------------------------------------------------------
    //     std::sort(morton.begin(), morton.end(),
    //               [](const MortonPrimitive& a, const MortonPrimitive& b)
    //               {
    //                   return a.morton < b.morton;
    //               });

    //     for (size_t i = 0; i < count; i++)
    //         indices[start + i] = morton[i].index;

    //     // --------------------------------------------------------
    //     // Recursive LBVH build (top-down structure only)
    //     // --------------------------------------------------------
    //     std::function<size_t(size_t, size_t)> buildNode =
    //     [&](size_t first, size_t last) -> size_t
    //     {
    //         size_t node = allocator.allocate();
    //         BVHNode& n = allocator.at(node);

    //         size_t rangeCount = last - first;

    //         // ---------------- Leaf ----------------
    //         if (rangeCount <= 8)
    //         {
    //             AABB bound;

    //             size_t firstLeaf = allocator.allocate();
    //             n.nextLeaf = firstLeaf;
    //             n.leafCount = rangeCount;

    //             size_t current = firstLeaf;

    //             for (size_t i = 0; i < rangeCount; i++)
    //             {
    //                 size_t prim = indices[start + first + i];

    //                 auto aabb = primitives[prim]->_transformedAABB();
    //                 bound.expand(aabb);

    //                 BVHNode& leaf = allocator.at(current);
    //                 leaf.primitiveIndex = prim;
    //                 leaf.bound = aabb;
    //                 leaf.leafCount = 0;

    //                 if (i + 1 < rangeCount)
    //                 {
    //                     size_t next = allocator.allocate();
    //                     leaf.nextLeaf = next;
    //                     current = next;
    //                 }
    //             }

    //             n.bound = bound;
    //             return node;
    //         }

    //         // ---------------- Internal LBVH split ----------------
    //         size_t split = findSplit(morton, first, last - 1);

    //         size_t left  = buildNode(first, split + 1);
    //         size_t right = buildNode(split + 1, last);

    //         n.left = left;
    //         n.right = right;

    //         n.bound = allocator.at(left).bound;
    //         n.bound.expand(allocator.at(right).bound);

    //         n.leafCount = 0;

    //         return node;
    //     };

    //     return buildNode(0, count);
    // }


    size_t buildBVH(std::vector<AABB>& primitives, std::vector<size_t>& indices, size_t start, size_t end, Carbo::ECSAllocator<BVHNode>& allocator);
    size_t buildBVH(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, Carbo::ECSAllocator<BVHNode>& allocator);
    void cullBVH(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVHNode>& allocator, std::vector<size_t>& visible);
}