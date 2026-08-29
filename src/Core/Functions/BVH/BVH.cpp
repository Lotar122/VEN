#include "BVH.hpp"
#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include "Structs/BVHNode.hpp"

#include <cmath>
#include <format>
#include <glm/ext/quaternion_geometric.hpp>
#include <immintrin.h>
#include <limits>
#include <mutex>
#include <stack>
#include <xmmintrin.h>
#include <bit> // std::countr_zero

using namespace nihil;

size_t nihil::buildBVH4(std::vector<nihil::graphics::Object *> &primitives, std::vector<size_t> &indices, size_t start, size_t end, size_t parent, Carbo::ECSAllocator<BVH4Node> &allocator, Carbo::ECSAllocator<BVH4LeafNode>& leafAllocator, Carbo::ECSAllocator<BVH4ColdNode>& coldAllocator)
{
    size_t node = allocator.allocate();
    size_t nodeCold = coldAllocator.allocate();

    assert(node == nodeCold);

    coldAllocator.at(node).parent = parent;

    AABB bound;
    AABB centroidBounds;
    for (int i = start; i < end; i++) 
    {
        const AABB& transformedAABB = primitives[indices[i]]->_transformedAABB();
        bound.expand(transformedAABB);
        centroidBounds.expand(transformedAABB._centroid());
    }

    int count = (int)end - (int)start;

    assert(count > 0);

    if (count <= 4) 
    {
        //Build the leafs
        // parent node
        BVH4Node& nodeRef = allocator.at(node);
        BVH4ColdNode& nodeColdRef = coldAllocator.at(node);
        nodeColdRef.bound = bound;
        nodeColdRef.leafCount = static_cast<uint8_t>(count);
        nodeColdRef.originalSurfaceArea = bound.surfaceArea();

        // first leaf node
        size_t firstLeaf = leafAllocator.allocate();
        nodeColdRef.firstLeaf = firstLeaf;

        size_t current = firstLeaf;
        leafAllocator.at(current).leafCount = count;
        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            BVH4LeafNode& currentRef = leafAllocator.at(current);
            currentRef.parent = node;
            currentRef.primitiveIndex = primIndex;
            currentRef.bound = primitives[primIndex]->_transformedAABB();

            primitives[primIndex]->BVHParentIndex = node;

            nodeRef.minX[i] = currentRef.bound.min.x;
            nodeRef.minY[i] = currentRef.bound.min.y;
            nodeRef.minZ[i] = currentRef.bound.min.z;

            nodeRef.maxX[i] = currentRef.bound.max.x;
            nodeRef.maxY[i] = currentRef.bound.max.y;
            nodeRef.maxZ[i] = currentRef.bound.max.z;

            if (i < count - 1)
            {
                size_t next = leafAllocator.allocate();
                leafAllocator.at(current).nextLeaf = next;
                current = next;
            }
        }

        return node;
    }

    size_t axis1 = centroidBounds.longestAxis<0>();
    size_t axis2 = centroidBounds.longestAxis<1>();

    const size_t mid  = start + count / 2;
    const size_t midA = start + count / 4;
    const size_t midB = start + (count * 3) / 4;

    assert(start < midA);
    assert(midA < mid);
    assert(mid < midB);
    assert(midB < end);

    auto compareAxis = [&](size_t axis)
    {
        return [axis, &primitives](size_t a, size_t b)
        {
            return primitives[a]->_transformedAABB()._centroid()[axis] <
                primitives[b]->_transformedAABB()._centroid()[axis];
        };
    };

    // First split: [start, end) -> [start, mid) + [mid, end)
    std::nth_element(
        indices.begin() + start,
        indices.begin() + mid,
        indices.begin() + end,
        compareAxis(axis1)
    );

    // Second split: left half
    std::nth_element(
        indices.begin() + start,
        indices.begin() + midA,
        indices.begin() + mid,
        compareAxis(axis2)
    );

    // Second split: right half
    std::nth_element(
        indices.begin() + mid,
        indices.begin() + midB,
        indices.begin() + end,
        compareAxis(axis2)
    );

    allocator.at(node).children[0] = buildBVH4(primitives, indices, start, midA, node, allocator, leafAllocator, coldAllocator);
    allocator.at(node).children[1] = buildBVH4(primitives, indices, midA, mid, node, allocator, leafAllocator, coldAllocator);
    allocator.at(node).children[2] = buildBVH4(primitives, indices, mid, midB, node, allocator, leafAllocator, coldAllocator);
    allocator.at(node).children[3] = buildBVH4(primitives, indices, midB, end, node, allocator, leafAllocator, coldAllocator);

    BVH4Node& nodeRef = allocator.at(node);
    BVH4ColdNode& nodeColdRef = coldAllocator.at(node);


    for (size_t i = 0; i < 4; ++i)
    {
        const BVH4ColdNode& childCold = coldAllocator.at(nodeRef.children[i]);

        nodeRef.minX[i] = childCold.bound.min.x;
        nodeRef.maxX[i] = childCold.bound.max.x;

        nodeRef.minY[i] = childCold.bound.min.y;
        nodeRef.maxY[i] = childCold.bound.max.y;

        nodeRef.minZ[i] = childCold.bound.min.z;
        nodeRef.maxZ[i] = childCold.bound.max.z;
    }

    nodeColdRef.bound = bound;
    nodeColdRef.leafCount = 0;
    nodeColdRef.originalSurfaceArea = bound.surfaceArea();

    return node;
}

uint16_t testBVH4Node(const BVH4Node& node, const std::array<Plane, 6>& planes)
{
    //Clever bit trick for abs, it sets the sign bit to 0 when & with a float
    const __m128 absMask = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
        
    __m128 centerX;
    __m128 centerY;
    __m128 centerZ;
    __m128 extentX;
    __m128 extentY;
    __m128 extentZ;

    const __m128 half = _mm_set1_ps(0.5f);

    __m128 minX = _mm_load_ps(node.minX.data());
    __m128 minY = _mm_load_ps(node.minY.data());
    __m128 minZ = _mm_load_ps(node.minZ.data());

    __m128 maxX = _mm_load_ps(node.maxX.data());
    __m128 maxY = _mm_load_ps(node.maxY.data());
    __m128 maxZ = _mm_load_ps(node.maxZ.data());

    centerX = _mm_mul_ps(_mm_add_ps(minX, maxX), half);
    centerY = _mm_mul_ps(_mm_add_ps(minY, maxY), half);
    centerZ = _mm_mul_ps(_mm_add_ps(minZ, maxZ), half);

    extentX = _mm_mul_ps(_mm_sub_ps(maxX, minX), half);
    extentY = _mm_mul_ps(_mm_sub_ps(maxY, minY), half);
    extentZ = _mm_mul_ps(_mm_sub_ps(maxZ, minZ), half);

    __m128 normalX;
    __m128 normalY;
    __m128 normalZ;

    __m128 absNormalX;
    __m128 absNormalY;
    __m128 absNormalZ;

    __m128 d;

    //Skip far plane

    int outsideMask = 0, insideMask = 0b1111;

    for(int i = 0; i < 5; i++)
    {
        const Plane& p = planes[i];

        normalX = _mm_set1_ps(p.normal.x);
        normalY = _mm_set1_ps(p.normal.y);
        normalZ = _mm_set1_ps(p.normal.z);
        d = _mm_set1_ps(p.d);

        absNormalX = _mm_and_ps(normalX, absMask);
        absNormalY = _mm_and_ps(normalY, absMask);
        absNormalZ = _mm_and_ps(normalZ, absMask);

        __m128 dist = _mm_mul_ps(centerX, normalX);
        dist = _mm_fmadd_ps(centerY, normalY, dist);
        dist = _mm_fmadd_ps(centerZ, normalZ, dist);
        dist = _mm_add_ps(dist, d);

        __m128 radius = _mm_mul_ps(extentX, absNormalX);
        radius = _mm_fmadd_ps(extentY, absNormalY, radius);
        radius = _mm_fmadd_ps(extentZ, absNormalZ, radius);

        const __m128 outside = _mm_cmplt_ps(_mm_add_ps(dist, radius), _mm_setzero_ps());
        const __m128 inside = _mm_cmpge_ps(_mm_sub_ps(dist, radius), _mm_setzero_ps());

        outsideMask |= _mm_movemask_ps(outside);
        insideMask  &= _mm_movemask_ps(inside);

        if(outsideMask == 0b1111) break;
    }

    int intersectMask = (~outsideMask & ~insideMask) & 0b1111;

    uint16_t result = 0 | outsideMask;
    result |= (insideMask << 4);
    result |= (intersectMask << 8);

    return result;
}

void nihil::cullBVH4(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVH4Node>& allocator, Carbo::ECSAllocator<BVH4LeafNode>& leafAllocator, Carbo::ECSAllocator<BVH4ColdNode>& coldAllocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack)
{
    alignas(std::vector<size_t>) std::byte stackMemory[sizeof(std::vector<size_t>)];
    if(!reusableStack) new (stackMemory) std::vector<size_t>();
    else reusableStack->clear();
    std::vector<size_t>& stack = *(reusableStack ? reusableStack : reinterpret_cast<std::vector<size_t>*>(stackMemory));
    stack.reserve(128);
    stack.push_back(root);

    while (!stack.empty())
    {
        const size_t nodeIndex = stack.back();
        stack.pop_back();

        const BVH4Node& node = allocator.at(nodeIndex);

        //Perform visibility query
        //TODO: Add a scalar fallback later
        const uint16_t result = testBVH4Node(node, planes);
        const uint32_t insideMask    = (result >> 4) & 0b1111;
        const uint32_t intersectMask = (result >> 8) & 0b1111;
        // any bit not set in either mask => that child is fully outside, skip it.

        uint32_t liveMask = insideMask | intersectMask;
        while (liveMask)
        {
            const int i = std::countr_zero(liveMask);
            liveMask &= liveMask - 1; // clear lowest set bit

            const BVH4Node& child = allocator.at(node.children[i]);
            const BVH4ColdNode& childCold = coldAllocator.at(node.children[i]);

            if (intersectMask & (1u << i))
            {
                if (childCold.leafCount > 0)
                {
                    // Straddling node: test its up-to-4 leaves individually.
                    const uint16_t resultLeaves = testBVH4Node(child, planes);
                    const uint32_t insideMaskLeaves    = (resultLeaves >> 4) & 0b1111;
                    const uint32_t intersectMaskLeaves = (resultLeaves >> 8) & 0b1111;
                    const uint32_t visibleLeaves = insideMaskLeaves | intersectMaskLeaves;

                    size_t leaf = childCold.firstLeaf;
                    for (uint8_t j = 0; j < childCold.leafCount; ++j)
                    {
                        if (visibleLeaves & (1u << j))
                            visible.push_back(leafAllocator.at(leaf).primitiveIndex);

                        leaf = leafAllocator.at(leaf).nextLeaf;
                    }
                }
                else
                {
                    stack.push_back(node.children[i]);
                }
            }
            else // insideMask bit set: child is fully inside every plane, accept whole subtree
            {
                if (childCold.leafCount > 0)
                {
                    size_t leaf = childCold.firstLeaf;
                    for (uint8_t j = 0; j < childCold.leafCount; ++j)
                    {
                        visible.push_back(leafAllocator.at(leaf).primitiveIndex);
                        leaf = leafAllocator.at(leaf).nextLeaf;
                    }
                }
                else
                {
                    // Drain exactly the subtree pushed here, tracked by stack size
                    // rather than the old "sentinel value" trick (which could read
                    // stack.back() on an empty stack, and could swallow unrelated
                    // pending nodes already sitting below it on the stack).
                    const size_t baseSize = stack.size();
                    stack.push_back(node.children[i]);

                    while (stack.size() > baseSize)
                    {
                        const size_t current = stack.back();
                        stack.pop_back();

                        const BVH4Node& innerNode = allocator.at(current);
                        const BVH4ColdNode& innerNodeCold = coldAllocator.at(current);
                        if (innerNodeCold.leafCount > 0)
                        {
                            size_t leaf = innerNodeCold.firstLeaf;
                            for (uint8_t j = 0; j < innerNodeCold.leafCount; ++j)
                            {
                                visible.push_back(leafAllocator.at(leaf).primitiveIndex);
                                leaf = leafAllocator.at(leaf).nextLeaf;
                            }
                        }
                        else
                        {
                            stack.push_back(innerNode.children[0]);
                            stack.push_back(innerNode.children[1]);
                            stack.push_back(innerNode.children[2]);
                            stack.push_back(innerNode.children[3]);
                        }
                    }
                }
            }
        }
    }

    if(!reusableStack) stack.~vector();
}

size_t nihil::buildBVH2(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, size_t parent, Carbo::ECSAllocator<BVH2Node>& allocator)
{
    size_t node = allocator.allocate();
    allocator.at(node).parent = parent;

    AABB bound;
    AABB centroidBounds;
    for (int i = start; i < end; i++) 
    {
        const AABB& transformedAABB = primitives[indices[i]]->_transformedAABB();
        bound.expand(transformedAABB);
        centroidBounds.expand(transformedAABB._centroid());
    }

    size_t count = end - start;

    if (count <= 8) 
    {
        //Build the leafs
        // parent node
        BVH2Node& nodeRef = allocator.at(node);
        nodeRef.bound = bound;
        nodeRef.leafCount = static_cast<uint8_t>(count);
        nodeRef.originalSurfaceArea = bound.surfaceArea();

        // first leaf node
        size_t firstLeaf = allocator.allocate();
        allocator.at(node).nextLeaf = firstLeaf;

        size_t current = firstLeaf;
        allocator.at(current).leafCount = count;
        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            BVH2Node& currentRef = allocator.at(current);
            currentRef.parent = parent;
            currentRef.primitiveIndex = primIndex;
            currentRef.bound = primitives[primIndex]->_transformedAABB();
            currentRef.originalSurfaceArea = currentRef.bound.surfaceArea();
            currentRef.leafCount = 0;
            primitives[primIndex]->BVHParentIndex = node;

            if (i < count - 1)
            {
                size_t next = allocator.allocate();
                allocator.at(current).nextLeaf = next;
                current = next;
            }
        }

        return node;
    }

    size_t axis = centroidBounds.longestAxis();

    size_t mid = (start + end) / 2;

    std::nth_element(
        indices.begin() + start,
        indices.begin() + mid,
        indices.begin() + end,
        [axis, &primitives](size_t a, size_t b)
        {
            return primitives[a]->_aabb()._centroid()[axis]
                < primitives[b]->_aabb()._centroid()[axis];
        }
    );

    allocator.at(node).left = buildBVH2(primitives, indices, start, mid, node, allocator);
    allocator.at(node).right = buildBVH2(primitives, indices, mid, end, node, allocator);
    allocator.at(node).bound = bound;
    allocator.at(node).leafCount = 0;
    allocator.at(node).originalSurfaceArea = bound.surfaceArea();

    return node;
}

template<typename Func, typename... Args>
requires std::invocable<Func, BVH2Node&, Args...>
void traverseLeafNodeBVH2(Carbo::ECSAllocator<BVH2Node>& allocator, const BVH2Node& node, Func&& operation, Args&&... args)
{
    // Traverse the leaf chain and test each primitive individually.
    size_t current = node.nextLeaf;

    for (size_t i = 0; i < node.leafCount; i++)
    {
        BVH2Node& leaf = allocator.at(current);

        operation(leaf, std::forward<Args>(args)...);

        current = leaf.nextLeaf;
    }
}

template<typename Func, typename... Args>
requires std::invocable<Func, const BVH2Node&, Args...>
void traverseLeafNodeBVH2Const(const Carbo::ECSAllocator<BVH2Node>& allocator, const BVH2Node& node, Func&& operation, Args&&... args)
{
    // Traverse the leaf chain and test each primitive individually.
    size_t current = node.nextLeaf;

    for (size_t i = 0; i < node.leafCount; i++)
    {
        const BVH2Node& leaf = allocator.at(current);

        operation(leaf, std::forward<Args>(args)...);

        current = leaf.nextLeaf;
    }
}

void nihil::cullBVH2(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVH2Node>& allocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack)
{
    alignas(std::vector<size_t>) std::byte stackMemory[sizeof(std::vector<size_t>)];
    if(!reusableStack) new (stackMemory) std::vector<size_t>();
    else reusableStack->clear();
    std::vector<size_t>& stack = *(reusableStack ? reusableStack : reinterpret_cast<std::vector<size_t>*>(stackMemory));
    stack.reserve(128);
    stack.push_back(root);

    while (!stack.empty())
    {
        size_t nodeIndex = stack.back();
        stack.pop_back();

        const BVH2Node& node = allocator.at(nodeIndex);

        VisibilityQueryResult visibilityQuery = AABB::isAABBVisible(node.bound, planes);

        if (visibilityQuery == VisibilityQueryResult::Outside)
            continue;
        else if(visibilityQuery == VisibilityQueryResult::Intersection)
        {
            if (node.leafCount > 0) // leaf container
            {
                traverseLeafNodeBVH2Const(allocator, node, [](const BVH2Node& leaf, const std::array<Plane, 6>& planes, std::vector<size_t>& visible) {
                    if (AABB::isAABBVisible(leaf.bound, planes) != VisibilityQueryResult::Outside)
                        visible.push_back(leaf.primitiveIndex);
                }, planes, visible);
            }
            else
            {
                stack.push_back(node.left);
                stack.push_back(node.right);
            }
        }
        else
        {
            if (node.leafCount > 0) // leaf container
            {
                traverseLeafNodeBVH2Const(allocator, node, [](const BVH2Node& leaf, std::vector<size_t>& visible) {
                    visible.push_back(leaf.primitiveIndex);
                }, visible);
            }
            else
            {
                // Entire subtree is inside, so both children can be accepted without more plane tests.

                size_t nodeIndex;
                size_t borderNode = !stack.empty() ? stack.back() : std::numeric_limits<size_t>::max();
                stack.push_back(node.left);
                stack.push_back(node.right);
                while(!stack.empty() && stack.back() != borderNode)
                {
                    nodeIndex = stack.back();
                    stack.pop_back();

                    const BVH2Node& node = allocator.at(nodeIndex);

                    if(node.leafCount > 0)
                    {
                        traverseLeafNodeBVH2Const(allocator, node, [](const BVH2Node& leaf, std::vector<size_t>& visible) {
                            visible.push_back(leaf.primitiveIndex);
                        }, visible);
                    }
                    else
                    {
                        stack.push_back(node.left);
                        stack.push_back(node.right);
                    }
                }
            }
        }
    }

    if(!reusableStack) stack.~vector<size_t>();
}

//In future implement batch mode
float nihil::refitBVH2(const std::vector<graphics::Object*>& primitives, graphics::Object* object, Carbo::ECSAllocator<BVH2Node>& allocator)
{
    if(object->BVHParentIndex == std::numeric_limits<size_t>::max()) Carbo::Logger::Exception("The object: {:p} has an invalid BVHIndex.", reinterpret_cast<void*>(object));

    BVH2Node& nodeRef = allocator.at(object->BVHParentIndex);
    float originalSurfaceArea = nodeRef.originalSurfaceArea;

    nodeRef.bound = AABB();

    size_t current = nodeRef.nextLeaf;

    for (size_t i = 0; i < nodeRef.leafCount; i++)
    {
        BVH2Node& leaf = allocator.at(current);

        //in the future only grab the modified nodes transformedAABB since random acces hurts the hardware prefetcher and the current AABB layout is cache firendly for leafs.
        // leaf.bound = primitives[leaf.primitiveIndex]->_transformedAABB();
        leaf.bound = primitives[leaf.primitiveIndex] == object ? primitives[leaf.primitiveIndex]->_transformedAABB() : leaf.bound;
        nodeRef.bound.expand(leaf.bound);

        current = leaf.nextLeaf;
    }

    current = nodeRef.parent;
    while(true)
    {
        BVH2Node& parentRef = allocator.at(current);
        const BVH2Node& left = allocator.at(parentRef.left);
        const BVH2Node& right = allocator.at(parentRef.right);
        
        parentRef.bound.min = glm::min(left.bound.min, right.bound.min);
        parentRef.bound.max = glm::max(left.bound.max, right.bound.max);

        if(current == 0 && parentRef.parent == 0) break;

        current = parentRef.parent;
    }

    float newSurfaceArea = nodeRef.bound.surfaceArea();

    //Magic number 7, idk why but everything works best when it's 7.
    return (newSurfaceArea / originalSurfaceArea) * 7.0f;
}