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

size_t nihil::buildBVH4(std::vector<nihil::graphics::Object *> &primitives, std::vector<size_t> &indices, size_t start, size_t end, size_t parent, Carbo::ECSAllocator<BVH4Node> &allocator, Carbo::ECSAllocator<BVH4LeafNode>& leafAllocator)
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
        BVH4Node& nodeRef = allocator.at(node);
        nodeRef.bound = bound;
        nodeRef.leafCount = static_cast<uint8_t>(count);
        nodeRef.originalSurfaceArea = bound.surfaceArea();

        // first leaf node
        size_t firstLeaf = leafAllocator.allocate();
        allocator.at(node).nextLeaf = firstLeaf;

        size_t current = firstLeaf;
        leafAllocator.at(current).leafCount = count;
        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            BVH4LeafNode& currentRef = leafAllocator.at(current);
            currentRef.parent = parent;
            currentRef.primitiveIndex = primIndex;
            currentRef.bound = primitives[primIndex]->_transformedAABB();

            primitives[primIndex]->BVHParentIndex = node;

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

    const size_t mid = (start + end) / 2;
    const size_t midA = (start + mid) / 2;
    const size_t midB = (mid + end) / 2;

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

    allocator.at(node).children[0] = buildBVH4(primitives, indices, start, midA, node, allocator, leafAllocator);
    allocator.at(node).children[1] = buildBVH4(primitives, indices, midA, mid, node, allocator, leafAllocator);
    allocator.at(node).children[2] = buildBVH4(primitives, indices, mid, midB, node, allocator, leafAllocator);
    allocator.at(node).children[3] = buildBVH4(primitives, indices, midB, end, node, allocator, leafAllocator);

    BVH4Node& nodeRef = allocator.at(node);

    for (size_t i = 0; i < 4; ++i)
    {
        const BVH4Node& child = allocator.at(nodeRef.children[i]);

        nodeRef.minX[i] = child.bound.min.x;
        nodeRef.maxX[i] = child.bound.max.x;

        nodeRef.minY[i] = child.bound.min.y;
        nodeRef.maxY[i] = child.bound.max.y;

        nodeRef.minZ[i] = child.bound.min.z;
        nodeRef.maxZ[i] = child.bound.max.z;
    }

    nodeRef.bound = bound;
    nodeRef.leafCount = 0;
    nodeRef.originalSurfaceArea = bound.surfaceArea();

    return node;
}

void nihil::cullBVH4(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVH4Node>& allocator, Carbo::ECSAllocator<BVH4LeafNode>& leafAllocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack)
{
    alignas(std::vector<size_t>) std::byte stackMemory[sizeof(std::vector<size_t>)];
    if(!reusableStack) new (stackMemory) std::vector<size_t>();
    else reusableStack->clear();
    std::vector<size_t>& stack = *(reusableStack ? reusableStack : reinterpret_cast<std::vector<size_t>*>(stackMemory));
    stack.reserve(128);
    stack.push_back(root);

    
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