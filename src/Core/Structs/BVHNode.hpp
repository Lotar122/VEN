#pragma once

#include "Classes/AABB/AABB.hpp"

namespace nihil 
{
    struct BVHNode
    {
        AABB bound;
        size_t nextLeaf;
        size_t left, right;
        size_t leafCount;
        size_t primitiveIndex;
    };
}