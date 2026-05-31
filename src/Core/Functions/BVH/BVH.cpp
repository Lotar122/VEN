#include "BVH.hpp"
#include "Structs/BVHNode.hpp"

using namespace nihil;

size_t nihil::buildBVH(std::vector<AABB>& primitives, std::vector<size_t>& indices, size_t start, size_t end, Carbo::ECSAllocator<BVHNode>& allocator)
{
    size_t node = allocator.allocate();

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
                currentRef.nextLeaf = next;
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

    allocator.at(node).left = buildBVH(primitives, indices, start, mid, allocator);
    allocator.at(node).right = buildBVH(primitives, indices, mid, end, allocator);
    allocator.at(node).bound = bound;
    allocator.at(node).leafCount = 0;

    return node;
}

size_t nihil::buildBVH(std::vector<nihil::graphics::Object*>& primitives, std::vector<size_t>& indices, size_t start, size_t end, Carbo::ECSAllocator<BVHNode>& allocator)
{
    size_t node = allocator.allocate();

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

            if (i < count - 1)
            {
                size_t next = allocator.allocate();
                currentRef.nextLeaf = next;
                current = next;
            }
        }

        return node;
    }

    AABB centroidBounds;
    for (int i = start; i < end; i++)
        centroidBounds.expand(primitives[indices[i]]->_transformedAABB()._centroid());

    size_t axis = centroidBounds.longestAxis();

    std::sort(indices.begin() + start, indices.begin() + end,
        [axis, &primitives](const size_t& a, const size_t& b) {
            return primitives[a]->_transformedAABB()._centroid()[axis] < primitives[b]->_transformedAABB()._centroid()[axis];
    });

    size_t mid = (start + end) / 2;

    allocator.at(node).left = buildBVH(primitives, indices, start, mid, allocator);
    allocator.at(node).right = buildBVH(primitives, indices, mid, end, allocator);
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
                stack.push_back(node.left);
                stack.push_back(node.right);
            }
        }
    }
}
