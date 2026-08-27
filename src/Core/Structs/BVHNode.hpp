#pragma once

#include "Classes/AABB/AABB.hpp"

#include <array>

namespace nihil 
{
    struct BVH2Node
    {
        AABB bound;
        uint32_t nextLeaf;
        uint32_t left, right, parent;
        uint32_t primitiveIndex;
        float originalSurfaceArea = 0.0f;
        uint8_t leafCount;
    };

    struct BVH4Node
    {
        AABB bound;

        std::array<float, 4> minX;
        std::array<float, 4> minY;
        std::array<float, 4> minZ;

        std::array<float, 4> maxX;
        std::array<float, 4> maxY;
        std::array<float, 4> maxZ;

        std::array<uint32_t, 4> children;

        uint32_t parent;
        uint32_t primitiveIndex;
        uint32_t nextLeaf;
        float originalSurfaceArea;
        uint8_t leafCount;
    };

    struct BVH4LeafNode
    {
        AABB bound;
        uint32_t primitiveIndex;
        uint32_t nextLeaf;
        uint32_t parent;
        uint8_t leafCount;
    };
}