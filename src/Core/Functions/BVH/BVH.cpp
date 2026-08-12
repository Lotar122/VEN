#include "BVH.hpp"
#include "Structs/BVHNode.hpp"

#include <cmath>
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

        // first leaf node
        size_t firstLeaf = allocator.allocate();
        nodeRef.nextLeaf = firstLeaf;

        size_t current = firstLeaf;
        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            BVHNode& currentRef = allocator.at(current);
            currentRef.primitiveIndex = primIndex;
            currentRef.bound = primitives[primIndex]->_transformedAABB();
            currentRef.leafCount = 0;
            primitives[primIndex]->BVHParentIndex = current;

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

    return node;
}

void nihil::cullBVH(size_t root, const std::array<Plane, 6>& planes, Carbo::ECSAllocator<BVHNode>& allocator, std::vector<size_t>& visible)
{
    std::vector<size_t> stack;
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
                // Traverse the leaf chain and test each primitive individually.
                size_t current = node.nextLeaf;

                for (size_t i = 0; i < node.leafCount; i++)
                {
                    const BVHNode& leaf = allocator.at(current);

                    if (AABB::isAABBVisible(leaf.bound, planes) != VisibilityQueryResult::Outside)
                        visible.push_back(leaf.primitiveIndex);

                    current = leaf.nextLeaf;
                }
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
                size_t current = node.nextLeaf;

                for (size_t i = 0; i < node.leafCount; i++)
                {
                    const BVHNode& leaf = allocator.at(current);

                    visible.push_back(leaf.primitiveIndex);

                    current = leaf.nextLeaf;
                }
            }
            else
            {
                // Entire subtree is inside, so both children can be accepted without more plane tests.

                size_t nodeIndex;
                std::vector<size_t> toTraverse;
                toTraverse.push_back(node.left);
                toTraverse.push_back(node.right);
                while(!toTraverse.empty())
                {
                    nodeIndex = toTraverse.back();
                    toTraverse.pop_back();

                    const BVHNode& node = allocator.at(nodeIndex);

                    if(node.leafCount > 0)
                    {
                        size_t current = node.nextLeaf;

                        for (size_t i = 0; i < node.leafCount; i++)
                        {
                            const BVHNode& leaf = allocator.at(current);

                            visible.push_back(leaf.primitiveIndex);

                            current = leaf.nextLeaf;
                        }
                    }
                    else
                    {
                        toTraverse.push_back(node.left);
                        toTraverse.push_back(node.right);
                    }
                }
            }
        }
    }
}

float refit(graphics::Object* object, Carbo::ECSAllocator<BVHNode>& allocator)
{
    if(object->BVHParentIndex == std::numeric_limits<size_t>::max()) Carbo::Logger::Exception("The object: {:p} has an invalid BVHIndex.", reinterpret_cast<void*>(object));

    BVHNode& nodeRef = allocator.at(object->BVHParentIndex);
    float oldSurfaceArea = nodeRef.bound.surfaceArea();

    nodeRef.bound.max = glm::vec3(std::numeric_limits<float>::lowest());
    nodeRef.bound.min = glm::vec3(std::numeric_limits<float>::max());

    size_t current = nodeRef.nextLeaf;

    for (size_t i = 0; i < nodeRef.leafCount; i++)
    {
        const BVHNode& leaf = allocator.at(current);

        nodeRef.bound.expand(leaf.bound);

        current = leaf.nextLeaf;
    }

    current = nodeRef.parent;
    while(current > 0)
    {
        BVHNode& parentRef = allocator.at(current);
        parentRef.bound = AABB();

        parentRef.bound.expand(allocator.at(parentRef.left).bound);
        parentRef.bound.expand(allocator.at(parentRef.right).bound);

        current = parentRef.parent;
    }

    float newSurfaceArea = nodeRef.bound.surfaceArea();

    return newSurfaceArea / oldSurfaceArea;
}