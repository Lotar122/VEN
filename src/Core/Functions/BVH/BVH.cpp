#include "BVH.hpp"
#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include "Structs/BVHNode.hpp"

#include <cmath>
#include <format>
#include <glm/ext/quaternion_geometric.hpp>
#include <limits>
#include <mutex>
#include <stack>

using namespace nihil;

//This shouldn't be used as refitting wont work.
size_t nihil::buildBVH(std::vector<AABB>& primitives, std::vector<size_t>& indices, size_t start, size_t end, size_t parent, Carbo::ECSAllocator<BVHNode>& allocator)
{
    size_t node = allocator.allocate();
    allocator.at(node).parent = parent;

    AABB bound;
    for (int i = start; i < end; i++) bound.expand(primitives[indices[i]]);

    int count = end - start;

    if (count <= 8) 
    {
        //Build the leafs
        // parent node
        BVHNode& nodeRef = allocator.at(node);
        nodeRef.bound = bound;
        nodeRef.leafCount = count;

        // first leaf node
        size_t firstLeaf = allocator.allocate();
        nodeRef.nextLeaf = firstLeaf;

        size_t current = firstLeaf;
        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            BVHNode& currentRef = allocator.at(current);
            currentRef.parent = parent;
            currentRef.primitiveIndex = primIndex;
            currentRef.bound = primitives[primIndex];
            currentRef.leafCount = 0;

            if (i < count - 1)
            {
                size_t next = allocator.allocate();
                allocator.at(current).nextLeaf = next;
                current = next;
            }
        }

        return node;
    }

    AABB centroidBounds;
    for (int i = start; i < end; i++)
        centroidBounds.expand(primitives[indices[i]]._centroid());

    size_t axis = centroidBounds.longestAxis();

    size_t mid = (start + end) / 2;

    std::nth_element(
        indices.begin() + start,
        indices.begin() + mid,
        indices.begin() + end,
        [axis, &primitives](size_t a, size_t b)
        {
            return primitives[a]._centroid()[axis]
                < primitives[b]._centroid()[axis];
        }
    );

    allocator.at(node).left = buildBVH(primitives, indices, start, mid, node, allocator);
    allocator.at(node).right = buildBVH(primitives, indices, mid, end, node, allocator);
    allocator.at(node).bound = bound;
    allocator.at(node).leafCount = 0;

    return node;
}

size_t nihil::buildBVH(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, size_t parent, Carbo::ECSAllocator<BVHNode>& allocator)
{
    size_t node = allocator.allocate();
    allocator.at(node).parent = parent;

    AABB bound;
    for (int i = start; i < end; i++) bound.expand(primitives[indices[i]]->_transformedAABB());

    int count = end - start;

    if (count <= 8) 
    {
        //Build the leafs
        // parent node
        BVHNode& nodeRef = allocator.at(node);
        nodeRef.bound = bound;
        nodeRef.leafCount = count;
        nodeRef.originalSurfaceArea = bound.surfaceArea();

        // first leaf node
        size_t firstLeaf = allocator.allocate();
        nodeRef.nextLeaf = firstLeaf;

        size_t current = firstLeaf;
        allocator.at(current).leafCount = count;
        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            BVHNode& currentRef = allocator.at(current);
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

    AABB centroidBounds;
    for (int i = start; i < end; i++)
        centroidBounds.expand(primitives[indices[i]]->_transformedAABB()._centroid());

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

    allocator.at(node).left = buildBVH(primitives, indices, start, mid, node, allocator);
    allocator.at(node).right = buildBVH(primitives, indices, mid, end, node, allocator);
    allocator.at(node).bound = bound;
    allocator.at(node).leafCount = 0;
    allocator.at(node).originalSurfaceArea = bound.surfaceArea();

    return node;
}

template<typename Func, typename... Args>
requires std::invocable<Func, BVHNode&, Args...>
void traverseLeafNode(Carbo::ECSAllocator<BVHNode>& allocator, const BVHNode& node, Func&& operation, Args&&... args)
{
    // Traverse the leaf chain and test each primitive individually.
    size_t current = node.nextLeaf;

    for (size_t i = 0; i < node.leafCount; i++)
    {
        BVHNode& leaf = allocator.at(current);

        operation(leaf, std::forward<Args>(args)...);

        current = leaf.nextLeaf;
    }
}

template<typename Func, typename... Args>
requires std::invocable<Func, const BVHNode&, Args...>
void traverseLeafNodeConst(const Carbo::ECSAllocator<BVHNode>& allocator, const BVHNode& node, Func&& operation, Args&&... args)
{
    // Traverse the leaf chain and test each primitive individually.
    size_t current = node.nextLeaf;

    for (size_t i = 0; i < node.leafCount; i++)
    {
        const BVHNode& leaf = allocator.at(current);

        operation(leaf, std::forward<Args>(args)...);

        current = leaf.nextLeaf;
    }
}

void nihil::cullBVH(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVHNode>& allocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack)
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

        const BVHNode& node = allocator.at(nodeIndex);

        VisibilityQueryResult visibilityQuery = AABB::isAABBVisible(node.bound, planes);

        if (visibilityQuery == VisibilityQueryResult::Outside)
            continue;
        else if(visibilityQuery == VisibilityQueryResult::Intersection)
        {
            if (node.leafCount > 0) // leaf container
            {
                traverseLeafNodeConst(allocator, node, [](const BVHNode& leaf, const std::array<Plane, 6>& planes, std::vector<size_t>& visible) {
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
                traverseLeafNodeConst(allocator, node, [](const BVHNode& leaf, std::vector<size_t>& visible) {
                    visible.push_back(leaf.primitiveIndex);
                }, visible);
            }
            else
            {
                // Entire subtree is inside, so both children can be accepted without more plane tests.

                size_t nodeIndex;
                size_t borderNode = stack.back();
                stack.push_back(node.left);
                stack.push_back(node.right);
                while(stack.back() != borderNode)
                {
                    nodeIndex = stack.back();
                    stack.pop_back();

                    const BVHNode& node = allocator.at(nodeIndex);

                    if(node.leafCount > 0)
                    {
                        traverseLeafNodeConst(allocator, node, [](const BVHNode& leaf, std::vector<size_t>& visible) {
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
float nihil::refitBVH(const std::vector<graphics::Object*>& primitives, graphics::Object* object, Carbo::ECSAllocator<BVHNode>& allocator)
{
    if(object->BVHParentIndex == std::numeric_limits<size_t>::max()) Carbo::Logger::Exception("The object: {:p} has an invalid BVHIndex.", reinterpret_cast<void*>(object));

    BVHNode& nodeRef = allocator.at(object->BVHParentIndex);
    float originalSurfaceArea = nodeRef.originalSurfaceArea;

    nodeRef.bound = AABB();

    size_t current = nodeRef.nextLeaf;

    for (size_t i = 0; i < nodeRef.leafCount; i++)
    {
        BVHNode& leaf = allocator.at(current);

        //in the future only grab the modified nodes transformedAABB since random acces hurts the hardware prefetcher and the current AABB layout is cache firendly for leafs.
        // leaf.bound = primitives[leaf.primitiveIndex]->_transformedAABB();
        leaf.bound = primitives[leaf.primitiveIndex] == object ? primitives[leaf.primitiveIndex]->_transformedAABB() : leaf.bound;
        nodeRef.bound.expand(leaf.bound);

        current = leaf.nextLeaf;
    }

    current = nodeRef.parent;
    while(true)
    {
        BVHNode& parentRef = allocator.at(current);
        const BVHNode& left = allocator.at(parentRef.left);
        const BVHNode& right = allocator.at(parentRef.right);
        
        parentRef.bound.min = glm::min(left.bound.min, right.bound.min);
        parentRef.bound.max = glm::max(left.bound.max, right.bound.max);

        if(current == 0 && parentRef.parent == 0) break;

        current = parentRef.parent;
    }

    float newSurfaceArea = nodeRef.bound.surfaceArea();

    //Magic number 7, idk why but everything works best when it's 7.
    return (newSurfaceArea / originalSurfaceArea) * 7.0f;
}