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

namespace Carbo
{
    template<size_t>
    class AtomicBumpAllocator;
}

namespace nihil
{
    ///The BVH2 tree builder
    //
    ///@param primitives The graphics::Object array that the tree needs to contain
    ///@param indices The indices to the primitives array
    ///@param start The begining of the subset of indices that the builder works on
    ///@param end The end of the subset of indices that the builder works on
    ///@param parent The parent nodes index (for root pass 0)
    ///@param allocator The allocator for the nodes (has to be empty)
    size_t buildBVH2(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, size_t parent, Carbo::ECSAllocator<BVH2Node>& allocator);
    
    ///The BVH2 culling algorithm - culls the tree against a view frustum
    //
    ///@param root The root node of the BVH2 to be culled
    ///@param planes The 6 planes of the view frustum
    ///@param allocator The allocator used for tree construction
    ///@param visible The vector to which the visible objects indices will be appended
    ///@param reusableStack The pointer to a reusable stack for culling, can be nullptr
    void cullBVH2(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVH2Node>& allocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack = nullptr);

    ///The BVH4 tree builder
    //
    ///@param primitives The graphics::Object array that the tree needs to contain
    ///@param indices The indices to the primitives array
    ///@param start The begining of the subset of indices that the builder works on
    ///@param end The end of the subset of indices that the builder works on
    ///@param parent The parent nodes index (for root pass 0)
    ///@param allocator The allocator for the nodes (has to be empty)
    ///@param centroidCache The cached centroids of all primitives
    size_t buildBVH4(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, size_t parent, Carbo::AtomicBumpAllocator<alignof(BVH4Node)>& allocator, std::vector<glm::vec3>& centroidCache);
    
    ///The BVH4 culling algorithm - culls the tree against a view frustum
    //
    ///@param root The root node of the BVH2 to be culled
    ///@param planes The 6 planes of the view frustum
    ///@param allocator The allocator used for tree construction
    ///@param visible The vector to which the visible objects indices will be appended
    ///@param reusableStack The pointer to a reusable stack for culling, can be nullptr
    void cullBVH4(size_t root, const std::array<Plane, 6>& planes, Carbo::AtomicBumpAllocator<alignof(BVH4Node)>& allocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack = nullptr);

    ///The function to refit the BVH4 tree around an object that changed positions
    //
    ///@param object The object which changed
    ///@param allocator The allocator used for tree construction
    float refitBH4(graphics::Object* object, Carbo::AtomicBumpAllocator<alignof(BVH4Node)>& allocator);

    ///The function that gathers the changes of objects
    //
    ///@param object The object which changed
    ///@param allocator The allocator used for tree construction
    ///@param parentIndices The array to which the indices that need refitting will be inserted 
    ///@return The refit "cost" for this object
    float refitBVH4Gather(graphics::Object* object, Carbo::AtomicBumpAllocator<alignof(BVH4Node)>& allocator, std::vector<size_t>& parentIndices);

    ///The function that actually refits the tree around indices selected for refitting
    //
    ///@param parentIndices The indices selected for refitting
    ///@param allocator The allocator used for tree construction
    void refitBH4Finalize(std::vector<size_t>& parentIndices, Carbo::AtomicBumpAllocator<alignof(BVH4Node)>& allocator);

    ///The function to refit the BVH4 tree around an object that changed positions
    //
    ///@param object The object which changed
    ///@param allocator The allocator used for tree construction
    ///@return The refit "cost"
    float refitBVH2(const std::vector<graphics::Object*>& primitives, graphics::Object* object, Carbo::ECSAllocator<BVH2Node>& allocator);
}