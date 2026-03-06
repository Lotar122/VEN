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
        allocator.at(node).bound = bound;
        allocator.at(node).leafCount = count;

        // first leaf node
        size_t firstLeaf = allocator.allocate();
        allocator.at(node).nextLeaf = firstLeaf;

        size_t current = firstLeaf;
        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            allocator.at(current).primitiveIndex = primIndex;
            allocator.at(current).bound = primitives[primIndex];
            allocator.at(current).leafCount = 0;

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
        centroidBounds.expand(primitives[indices[i]].centroid());

    size_t axis = centroidBounds.longestAxis();

    std::sort(indices.begin() + start, indices.begin() + end,
        [axis, &primitives](const size_t& a, const size_t& b) {
            return primitives[a].centroid()[axis] < primitives[b].centroid()[axis];
    });

    size_t mid = (start + end) / 2;

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
        allocator.at(node).bound = bound;
        allocator.at(node).leafCount = count;

        // first leaf node
        size_t firstLeaf = allocator.allocate();
        allocator.at(node).nextLeaf = firstLeaf;

        size_t current = firstLeaf;
        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            allocator.at(current).primitiveIndex = primIndex;
            allocator.at(current).bound = primitives[primIndex]->_transformedAABB();
            allocator.at(current).leafCount = 0;

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
        centroidBounds.expand(primitives[indices[i]]->_transformedAABB().centroid());

    size_t axis = centroidBounds.longestAxis();

    std::sort(indices.begin() + start, indices.begin() + end,
        [axis, &primitives](const size_t& a, const size_t& b) {
            return primitives[a]->_transformedAABB().centroid()[axis] < primitives[b]->_transformedAABB().centroid()[axis];
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
            if (node.leafCount > 0) // leaf
            {
                //Traverse to add to visible
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
            //Traverse fully inside to set the visibility
            size_t currentNode;
            size_t prevStackSize = stack.size();
            stack.push_back(node.nextLeaf);

            while(stack.size() > prevStackSize)
            {
                currentNode = stack.back();
                stack.pop_back();

                if(allocator.at(currentNode).leafCount > 0)
                {
                    for (size_t i = 0; i < allocator.at(currentNode).leafCount; i++)
                    {
                        const BVHNode& leaf = allocator.at(currentNode);

                        visible.push_back(leaf.primitiveIndex);

                        currentNode = leaf.nextLeaf;
                    }
                }
                else
                {
                    stack.push_back(allocator.at(currentNode).left);
                    stack.push_back(allocator.at(currentNode).right);
                }
            }
        }
    }
}