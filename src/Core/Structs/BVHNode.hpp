#pragma once

#include "Classes/AABB/AABB.hpp"

namespace nihil 
{
    struct BVHNode
    {
        AABB bound;
        size_t nextLeaf;
        size_t left, right, parent;
        size_t primitiveIndex;
        uint8_t leafCount;
    };
}