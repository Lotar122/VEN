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

#include "Classes/CPUFeatures/CPUFeatures.hpp"
#include "Functions/Prefetch/Prefetch.hpp"
#include "Classes/AtomicBumpAllocator/AtomicBumpAllocator.hpp"

using namespace nihil;

size_t nihil::buildBVH4(std::vector<nihil::graphics::Object*> &primitives, std::vector<size_t> &indices, size_t start, size_t end, size_t parent, Carbo::AtomicBumpAllocator<alignof(BVH4Node)>& allocator, std::vector<glm::vec3>& centroidCache)
{
    auto _mm_horizontalmin_ps = [](__m128 v) -> float
    {
        v = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
        v = _mm_min_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 0, 3, 2)));
        return _mm_cvtss_f32(v);
    };

    auto _mm_horizontalmax_ps = [](__m128 v) -> float
    {
        v = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1)));
        v = _mm_max_ps(v, _mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 0, 3, 2)));
        return _mm_cvtss_f32(v);
    };

    BVH4Node* node = allocator.allocate<BVH4Node>();
    size_t nodeIndex = node - reinterpret_cast<BVH4Node*>(allocator._data());

    node->leafMask = 0;

    node->parent = parent;

    size_t count = end - start;

    assert((int)end - (int)start > 0);

    if (count <= 4) 
    {
        //Build the leafs

        node->leafMask = (1u << count) - 1;

        for (size_t i = 0; i < count; i++)
        {
            size_t primIndex = indices[start + i];
            primitives[primIndex]->BVHParentIndex = nodeIndex;

            //in future make indices uint32_t so that you can do one simd load and store to do this loops work
            node->children[i] = primIndex;

            AABB bound = primitives[primIndex]->_transformedAABB();

            node->minX[i] = bound.min.x;
            node->minY[i] = bound.min.y;
            node->minZ[i] = bound.min.z;

            node->maxX[i] = bound.max.x;
            node->maxY[i] = bound.max.y;
            node->maxZ[i] = bound.max.z;   
        }

        node->originalSurfaceArea = AABB::surfaceArea(
            {_mm_horizontalmin_ps(_mm_load_ps(node->minX.data())), _mm_horizontalmin_ps(_mm_load_ps(node->minY.data())), _mm_horizontalmin_ps(_mm_load_ps(node->minZ.data()))},
            {_mm_horizontalmax_ps(_mm_load_ps(node->maxX.data())), _mm_horizontalmax_ps(_mm_load_ps(node->maxY.data())), _mm_horizontalmax_ps(_mm_load_ps(node->maxZ.data()))}
        );

        return nodeIndex;
    }

    AABB centroidBounds;
    for (int i = start; i < end; i++) 
    {
        centroidBounds.expand(centroidCache[indices[i]]);
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

    // Decorate: gather (key, index) into a contiguous buffer so nth_element's
    // comparisons hit sequential memory instead of chasing indices -> centroidCache.
    std::vector<std::pair<float, size_t>> keyed(count);
    for (size_t i = 0; i < count; ++i) {
        const size_t idx = indices[start + i];
        keyed[i] = { centroidCache[idx][axis1], idx };
    }

    auto cmp = [](const auto& a, const auto& b) noexcept {
        return a.first < b.first;
    };

    // Split on axis1 first, over the whole buffer.
    std::nth_element(keyed.begin(), keyed.begin() + count / 2, keyed.end(), cmp);

    // Re-key both halves for axis2 — still a single sequential pass.
    for (auto& kv : keyed) kv.first = centroidCache[kv.second][axis2];

    // These two ranges are disjoint, so they're safe to run concurrently
    // if count is large enough to make the thread overhead worth it.
    std::nth_element(keyed.begin(),            keyed.begin() + count / 4,       keyed.begin() + count / 2, cmp);
    std::nth_element(keyed.begin() + count / 2, keyed.begin() + (count * 3) / 4, keyed.end(),               cmp);

    // Undecorate: write the resulting permutation back into indices.
    for (size_t i = 0; i < count; ++i) indices[start + i] = keyed[i].second;

    if(nodeIndex == 0) [[unlikely]]
    {
        std::thread t1([&]() {node->children[0] = buildBVH4(primitives, indices, start, midA, nodeIndex, allocator, centroidCache);});
        std::thread t2([&]() {node->children[1] = buildBVH4(primitives, indices, midA, mid, nodeIndex, allocator, centroidCache);});
        std::thread t3([&]() {node->children[2] = buildBVH4(primitives, indices, mid, midB, nodeIndex, allocator, centroidCache);});
        std::thread t4([&]() {node->children[3] = buildBVH4(primitives, indices, midB, end, nodeIndex, allocator, centroidCache);});

        if(t1.joinable()) t1.join();
        if(t2.joinable()) t2.join();
        if(t3.joinable()) t3.join();
        if(t4.joinable()) t4.join();
    }
    else
    {
        node->children[0] = buildBVH4(primitives, indices, start, midA, nodeIndex, allocator, centroidCache);
        node->children[1] = buildBVH4(primitives, indices, midA, mid, nodeIndex, allocator, centroidCache);
        node->children[2] = buildBVH4(primitives, indices, mid, midB, nodeIndex, allocator, centroidCache);
        node->children[3] = buildBVH4(primitives, indices, midB, end, nodeIndex, allocator, centroidCache);
    }


    BVH4Node& child0 = *(reinterpret_cast<BVH4Node*>(allocator._data()) + node->children[0]);
    BVH4Node& child1 = *(reinterpret_cast<BVH4Node*>(allocator._data()) + node->children[1]);
    BVH4Node& child2 = *(reinterpret_cast<BVH4Node*>(allocator._data()) + node->children[2]);
    BVH4Node& child3 = *(reinterpret_cast<BVH4Node*>(allocator._data()) + node->children[3]);

    __m128 minX = _mm_set_ps(
        _mm_horizontalmin_ps(_mm_load_ps(child3.minX.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child2.minX.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child1.minX.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child0.minX.data()))
    );

    __m128 maxX = _mm_set_ps(
        _mm_horizontalmax_ps(_mm_load_ps(child3.maxX.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child2.maxX.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child1.maxX.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child0.maxX.data()))
    );

    __m128 minY = _mm_set_ps(
        _mm_horizontalmin_ps(_mm_load_ps(child3.minY.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child2.minY.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child1.minY.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child0.minY.data()))
    );

    __m128 maxY = _mm_set_ps(
        _mm_horizontalmax_ps(_mm_load_ps(child3.maxY.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child2.maxY.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child1.maxY.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child0.maxY.data()))
    );

    __m128 minZ = _mm_set_ps(
        _mm_horizontalmin_ps(_mm_load_ps(child3.minZ.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child2.minZ.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child1.minZ.data())),
        _mm_horizontalmin_ps(_mm_load_ps(child0.minZ.data()))
    );

    __m128 maxZ = _mm_set_ps(
        _mm_horizontalmax_ps(_mm_load_ps(child3.maxZ.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child2.maxZ.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child1.maxZ.data())),
        _mm_horizontalmax_ps(_mm_load_ps(child0.maxZ.data()))
    );

    _mm_store_ps(node->minX.data(), minX);
    _mm_store_ps(node->maxX.data(), maxX);
    
    _mm_store_ps(node->minY.data(), minY);
    _mm_store_ps(node->maxY.data(), maxY);

    _mm_store_ps(node->minZ.data(), minZ);
    _mm_store_ps(node->maxZ.data(), maxZ);

    node->originalSurfaceArea = AABB::surfaceArea(
        {_mm_horizontalmin_ps(minX), _mm_horizontalmin_ps(minY), _mm_horizontalmin_ps(minZ)},
        {_mm_horizontalmax_ps(maxX), _mm_horizontalmax_ps(maxY), _mm_horizontalmax_ps(maxZ)}
    );

    return nodeIndex;
}

uint16_t testBVH4Node_SSE41_FMA(const BVH4Node& node, const std::array<Plane, 6>& planes)
{
    const __m128 zero = _mm_setzero_ps();
    const __m128 signMask = _mm_set1_ps(-0.0f);

    const __m128 minX = _mm_load_ps(node.minX.data());
    const __m128 minY = _mm_load_ps(node.minY.data());
    const __m128 minZ = _mm_load_ps(node.minZ.data());

    const __m128 maxX = _mm_load_ps(node.maxX.data());
    const __m128 maxY = _mm_load_ps(node.maxY.data());
    const __m128 maxZ = _mm_load_ps(node.maxZ.data());

    int outsideMask = 0;
    int insideMask = 0xF;

    for (int i = 0; i < 5; ++i)
    {
        const Plane& p = planes[i];

        const __m128 nx = _mm_set1_ps(p.normal.x);
        const __m128 ny = _mm_set1_ps(p.normal.y);
        const __m128 nz = _mm_set1_ps(p.normal.z);
        const __m128 d  = _mm_set1_ps(p.d);

        // Sign mask of each normal.
        const __m128 sx = _mm_and_ps(nx, signMask);
        const __m128 sy = _mm_and_ps(ny, signMask);
        const __m128 sz = _mm_and_ps(nz, signMask);

        // Select the vertex furthest in the direction of the normal.
        //
        // normal >= 0 -> max
        // normal <  0 -> min
        //
        const __m128 px = _mm_blendv_ps(maxX, minX, sx);
        const __m128 py = _mm_blendv_ps(maxY, minY, sy);
        const __m128 pz = _mm_blendv_ps(maxZ, minZ, sz);

        // Select the vertex closest in the direction of the normal.
        const __m128 nxv = _mm_blendv_ps(minX, maxX, sx);
        const __m128 nyv = _mm_blendv_ps(minY, maxY, sy);
        const __m128 nzv = _mm_blendv_ps(minZ, maxZ, sz);

        // Distance of positive vertex from plane.
        __m128 positiveDistance = _mm_mul_ps(px, nx);
        positiveDistance = _mm_fmadd_ps(py, ny, positiveDistance);
        positiveDistance = _mm_fmadd_ps(pz, nz, positiveDistance);
        positiveDistance = _mm_add_ps(positiveDistance, d);

        // Distance of negative vertex from plane.
        __m128 negativeDistance = _mm_mul_ps(nxv, nx);
        negativeDistance = _mm_fmadd_ps(nyv, ny, negativeDistance);
        negativeDistance = _mm_fmadd_ps(nzv, nz, negativeDistance);
        negativeDistance = _mm_add_ps(negativeDistance, d);

        // Entire AABB is outside.
        const __m128 outside =
            _mm_cmplt_ps(positiveDistance, zero);

        // Entire AABB is inside.
        const __m128 inside =
            _mm_cmpge_ps(negativeDistance, zero);

        outsideMask |= _mm_movemask_ps(outside);
        insideMask  &= _mm_movemask_ps(inside);

        if (outsideMask == 0xF)
            break;
    }

    const int intersectMask =
        ~(outsideMask | insideMask) & 0xF;

    uint16_t result = outsideMask;
    result |= insideMask << 4;
    result |= intersectMask << 8;

    return result;
}

uint16_t testBVH4Node_SSE2_FMA(const BVH4Node& node, const std::array<Plane, 6>& planes)
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

uint16_t testBVH4Node_SSE2(const BVH4Node& node, const std::array<Plane, 6>& planes)
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
        dist = _mm_add_ps(dist, _mm_mul_ps(centerY, normalY));
        dist = _mm_add_ps(dist, _mm_mul_ps(centerZ, normalZ));
        dist = _mm_add_ps(dist, d);

        __m128 radius = _mm_mul_ps(extentX, absNormalX);
        radius = _mm_add_ps(radius, _mm_mul_ps(extentY, absNormalY));
        radius = _mm_add_ps(radius, _mm_mul_ps(extentZ, absNormalZ));

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

uint16_t testBVH4Node_Scalar(const BVH4Node& node, const std::array<Plane, 6>& planes)
{
    std::array<glm::vec3, 4> center;
    std::array<glm::vec3, 4> extent;

    for (int child = 0; child < 4; ++child)
    {
        const glm::vec3 min(
            node.minX[child],
            node.minY[child],
            node.minZ[child]
        );

        const glm::vec3 max(
            node.maxX[child],
            node.maxY[child],
            node.maxZ[child]
        );

        center[child] = (min + max) * 0.5f;
        extent[child] = (max - min) * 0.5f;
    }

    uint16_t outsideMask = 0;
    uint16_t insideMask = 0b1111;

    for (int i = 0; i < 5; ++i)
    {
        const Plane& p = planes[i];

        const glm::vec3 absNormal = glm::abs(p.normal);

        for (int child = 0; child < 4; ++child)
        {
            const float dist =
                glm::dot(center[child], p.normal) + p.d;

            const float radius =
                glm::dot(extent[child], absNormal);

            const uint16_t bit = uint16_t(1u << child);

            if (dist + radius < 0.0f)
                outsideMask |= bit;

            if (dist - radius >= 0.0f)
                insideMask &= uint16_t(~bit);
        }

        if (outsideMask == 0b1111)
            break;
    }

    const uint16_t intersectMask =
        (~outsideMask & ~insideMask) & 0b1111;

    return outsideMask
         | uint16_t(insideMask << 4)
         | uint16_t(intersectMask << 8);
}

using testBVH4NodeType = uint16_t(*)(const BVH4Node& node, const std::array<Plane, 6>& planes);

testBVH4NodeType testBVH4Node = testBVH4Node_Scalar;

struct InittestBVH4Node
{
    InittestBVH4Node()
    {
        Carbo::CPUFeatures features;

        Carbo::Logger::Init();

        if(features.supports(Carbo::CPUFeatures::feature::sse41) && features.supports(Carbo::CPUFeatures::feature::fma))
        {
            Carbo::Logger::Log("Using testBVH4Node, version: SSE4.1, FMA");
            testBVH4Node = testBVH4Node_SSE41_FMA;
        }
        else if(features.supports(Carbo::CPUFeatures::feature::sse2) && features.supports(Carbo::CPUFeatures::feature::fma))
        {
            Carbo::Logger::Log("Using testBVH4Node, version: SSE2, FMA");
            testBVH4Node = testBVH4Node_SSE2_FMA;
        }
        else if(features.supports(Carbo::CPUFeatures::feature::sse2))
        {
            Carbo::Logger::Log("Using testBVH4Node, version: SSE2");
            testBVH4Node = testBVH4Node_SSE2;
        }
    }
};

InittestBVH4Node init;

void nihil::cullBVH4(size_t root, const std::array<Plane, 6>& planes, Carbo::AtomicBumpAllocator<alignof(BVH4Node)>& allocator, std::vector<size_t>& visible, std::vector<size_t>* reusableStack)
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

        const BVH4Node& node = allocator.at<BVH4Node>(nodeIndex);

        //Perform visibility query
        const uint16_t result = testBVH4Node(node, planes);
        const uint32_t insideMask    = (result >> 4) & 0b1111;
        const uint32_t intersectMask = (result >> 8) & 0b1111;
        // any bit not set in either mask => that child is fully outside, skip it.

        uint32_t liveMask = insideMask | intersectMask;
        while (liveMask)
        {
            const int i = std::countr_zero(liveMask);
            liveMask &= liveMask - 1; // clear lowest set bit

            const BVH4Node& child = allocator.at<BVH4Node>(node.children[i]);

            if (intersectMask & (1u << i))
            {
                if (child.leafMask)
                {
                    // Straddling node: test its up-to-4 leaves individually.
                    const uint16_t resultLeaves = testBVH4Node(child, planes);
                    const uint32_t insideMaskLeaves    = (resultLeaves >> 4) & 0b1111;
                    const uint32_t intersectMaskLeaves = (resultLeaves >> 8) & 0b1111;
                    const uint32_t visibleLeaves = (insideMaskLeaves | intersectMaskLeaves) & child.leafMask;

                    for(int j = 0; j < std::popcount(child.leafMask); j++)
                    {
                        if (visibleLeaves & (1u << j))
                            visible.push_back(child.children[j]);
                    }
                }
                else
                {
                    stack.push_back(node.children[i]);
                }
            }
            else // insideMask bit set: child is fully inside every plane, accept whole subtree
            {
                if (child.leafMask)
                {
                    for(int j = 0; j < std::popcount(child.leafMask); j++)
                    {
                        visible.push_back(child.children[j]);
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

                        const BVH4Node& innerNode = allocator.at<BVH4Node>(current);
                        if (innerNode.leafMask)
                        {
                            for(int j = 0; j < std::popcount(innerNode.leafMask); j++)
                            {
                                visible.push_back(innerNode.children[j]);
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
        nodeRef.originalSurfaceArea = bound._surfaceArea();

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
            currentRef.originalSurfaceArea = currentRef.bound._surfaceArea();
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
    allocator.at(node).originalSurfaceArea = bound._surfaceArea();

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

    float newSurfaceArea = nodeRef.bound._surfaceArea();

    //Magic number 7, idk why but everything works best when it's 7.
    return (newSurfaceArea / originalSurfaceArea) * 7.0f;
}