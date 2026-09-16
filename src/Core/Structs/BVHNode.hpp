#pragma once

#include "Classes/AABB/AABB.hpp"

#include <array>
#include <limits>

namespace nihil 
{
    ///The node for BVH2 trees
    struct BVH2Node
    {
        ///The nodes bound
        AABB bound;
        ///The next leaf, if the node is a node with leaves
        uint32_t nextLeaf;
        ///The left node from this one
        uint32_t left;
        ///The right node from this one
        uint32_t right;
        ///The parent of this node
        uint32_t parent;
        ///The primitive index for a leaf node
        uint32_t primitiveIndex;
        ///The original surface area of the bound
        float originalSurfaceArea = 0.0f;
        ///The leaf count if the node has leaves
        uint8_t leafCount;
    };

    // struct alignas(16) BVH4Node
    // {
    //     std::array<float, 4> minX;
    //     std::array<float, 4> minY;
    //     std::array<float, 4> minZ;

    //     std::array<float, 4> maxX;
    //     std::array<float, 4> maxY;
    //     std::array<float, 4> maxZ;

    //     std::array<uint32_t, 4> children;
    // };

    // struct BVH4LeafNode
    // {
    //     // AABB bound;
    //     uint32_t primitiveIndex;
    //     uint32_t nextLeaf;
    //     // uint32_t parent;
    // };

    // struct BVH4ColdNode
    // {
    //     AABB bound;
    //     uint32_t parent;
    //     uint32_t primitiveIndex;
    //     uint32_t firstLeaf;
    //     float originalSurfaceArea;
    //     uint8_t leafCount;
    // };

    ///The node for BVH4 trees
    struct alignas(16) BVH4Node
    {
        ///The smallest x coordinates of the 4 childrens' bounds
        std::array<float, 4> minX;
        ///The smallest y coordinates of the 4 childrens' bounds
        std::array<float, 4> minY;
        ///The smallest z coordinates of the 4 childrens' bounds
        std::array<float, 4> minZ;

        ///The biggest x coordinates of the 4 childrens' bounds 
        std::array<float, 4> maxX;
        ///The biggest y coordinates of the 4 childrens' bounds
        std::array<float, 4> maxY;
        ///The biggest z coordinates of the 4 childrens' bounds
        std::array<float, 4> maxZ;

        ///The 4 childrens indices. If this node is a leaf node these are primitive indices, otherwise internal node indices
        std::array<uint32_t, 4> children;
        
        ///The parent of this node
        uint32_t parent;

        // uint32_t nextLeafNodeOrFirstLeafNode;

        // uint32_t primIndex;

        ///The original surface area of the nodes bounds
        float originalSurfaceArea;

        ///The mask for leaf count
        uint8_t leafMask = 0;
    };
}