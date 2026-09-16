#include <gtest/gtest.h>

#include "Functions/BVH/BVH.hpp"
#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include "Classes/AtomicBumpAllocator/AtomicBumpAllocator.hpp"
#include "Structs/BVHNode.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace
{
    using nihil::AABB;
    using nihil::BVH2Node;
    using nihil::BVH4Node;
    using nihil::Plane;

    constexpr float EPSILON = 1e-5f;

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    AABB makeAABB(
        glm::vec3 min,
        glm::vec3 max)
    {
        return AABB{min, max};
    }

    Plane makePlane(
        glm::vec3 normal,
        float d)
    {
        Plane plane{};
        plane.normal = normal;
        plane.d = d;
        return plane;
    }

    /*
     * Frustum:
     *
     *   -X <= x <= +X
     *   -Y <= y <= +Y
     *   -Z <= z <= +Z
     *
     * Plane convention used by the BVH code is:
     *
     *     dot(p.normal, point) + p.d >= 0
     *
     * for a point inside the plane.
     */
    std::array<Plane, 6> makeBoxFrustum(
        float x,
        float y,
        float z)
    {
        return {
            makePlane({ 1.0f,  0.0f,  0.0f}, x),  // x >= -x
            makePlane({-1.0f,  0.0f,  0.0f}, x),  // x <= +x

            makePlane({ 0.0f,  1.0f,  0.0f}, y),  // y >= -y
            makePlane({ 0.0f, -1.0f,  0.0f}, y),  // y <= +y

            makePlane({ 0.0f,  0.0f,  1.0f}, z),  // z >= -z
            makePlane({ 0.0f,  0.0f, -1.0f}, z)   // z <= +z
        };
    }

    BVH4Node makeBVH4Node(
        const std::array<AABB, 4>& bounds)
    {
        BVH4Node node{};

        node.leafMask = 0x0F;

        for (size_t i = 0; i < 4; ++i)
        {
            node.minX[i] = bounds[i].min.x;
            node.minY[i] = bounds[i].min.y;
            node.minZ[i] = bounds[i].min.z;

            node.maxX[i] = bounds[i].max.x;
            node.maxY[i] = bounds[i].max.y;
            node.maxZ[i] = bounds[i].max.z;

            node.children[i] = static_cast<uint32_t>(i);
        }

        return node;
    }

    BVH2Node makeBVH2LeafContainer(
        const AABB& bound,
        size_t firstLeaf,
        size_t leafCount)
    {
        BVH2Node node{};

        node.bound = bound;
        node.nextLeaf = static_cast<uint32_t>(firstLeaf);
        node.leafCount = static_cast<uint8_t>(leafCount);
        node.parent = 0;
        node.left = 0;
        node.right = 0;

        node.originalSurfaceArea = bound._surfaceArea();

        return node;
    }

    BVH2Node makeBVH2Leaf(
        const AABB& bound,
        size_t primitiveIndex,
        size_t nextLeaf,
        size_t parent)
    {
        BVH2Node node{};

        node.bound = bound;
        node.primitiveIndex = static_cast<uint32_t>(primitiveIndex);
        node.nextLeaf = static_cast<uint32_t>(nextLeaf);
        node.parent = static_cast<uint32_t>(parent);
        node.leafCount = 0;

        node.originalSurfaceArea = bound._surfaceArea();

        return node;
    }
}


// =============================================================================
// BVH4 visibility classification
// =============================================================================
//
// These functions are currently implementation-local in BVH.cpp:
//
//     testBVH4Node_Scalar
//     testBVH4Node_SSE2
//     testBVH4Node_SSE2_FMA
//     testBVH4Node_SSE41_FMA
//
// Add these declarations to a small test-only header, or expose the scalar
// classifier through the public BVH interface.
//
// The declarations below are enough if BVH.cpp already exports the functions.
// =============================================================================

uint16_t testBVH4Node_Scalar(
    const nihil::BVH4Node&,
    const std::array<nihil::Plane, 6>&);

uint16_t testBVH4Node_SSE2(
    const nihil::BVH4Node&,
    const std::array<nihil::Plane, 6>&);

uint16_t testBVH4Node_SSE2_FMA(
    const nihil::BVH4Node&,
    const std::array<nihil::Plane, 6>&);

uint16_t testBVH4Node_SSE41_FMA(
    const nihil::BVH4Node&,
    const std::array<nihil::Plane, 6>&);


// =============================================================================
// BVH4Node classification
// =============================================================================

TEST(BVH4NodeTest, FullyInsideChildrenAreMarkedInside)
{
    const std::array<AABB, 4> bounds = {
        makeAABB({-1, -1, -1}, {1, 1, 1}),
        makeAABB({ 1, -1, -1}, {3, 1, 1}),
        makeAABB({-1,  1, -1}, {1, 3, 1}),
        makeAABB({-1, -1,  1}, {1, 1, 3})
    };

    const BVH4Node node = makeBVH4Node(bounds);

    const auto planes = makeBoxFrustum(100.0f, 100.0f, 100.0f);

    const uint16_t result =
        testBVH4Node_Scalar(node, planes);

    const uint16_t outside =
        result & 0x0F;

    const uint16_t inside =
        (result >> 4) & 0x0F;

    const uint16_t intersect =
        (result >> 8) & 0x0F;

    EXPECT_EQ(outside, 0);
    EXPECT_EQ(inside, 0x0F);
    EXPECT_EQ(intersect, 0);
}


TEST(BVH4NodeTest, FullyOutsideChildrenAreMarkedOutside)
{
    const std::array<AABB, 4> bounds = {
        makeAABB({100, 0, 0}, {101, 1, 1}),
        makeAABB({100, 2, 0}, {101, 3, 1}),
        makeAABB({100, 4, 0}, {101, 5, 1}),
        makeAABB({100, 6, 0}, {101, 7, 1})
    };

    const BVH4Node node = makeBVH4Node(bounds);

    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    const uint16_t result =
        testBVH4Node_Scalar(node, planes);

    EXPECT_EQ(result & 0x0F, 0x0F);
    EXPECT_EQ((result >> 4) & 0x0F, 0);
    EXPECT_EQ((result >> 8) & 0x0F, 0);
}


TEST(BVH4NodeTest, IntersectingChildrenAreMarkedIntersecting)
{
    const std::array<AABB, 4> bounds = {
        makeAABB({ 9, -1, -1}, {11, 1, 1}),
        makeAABB({-11, -1, -1}, {-9, 1, 1}),
        makeAABB({-1, 9, -1}, {1, 11, 0}),
        makeAABB({-1, -11, -1}, {1, -9, 0})
    };

    const BVH4Node node = makeBVH4Node(bounds);

    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    const uint16_t result =
        testBVH4Node_Scalar(node, planes);

    EXPECT_EQ(result & 0x0F, 0);
    EXPECT_EQ((result >> 4) & 0x0F, 0);
    EXPECT_EQ((result >> 8) & 0x0F, 0x0F);
}


TEST(BVH4NodeTest, ClassificationPartitionsEveryChild)
{
    const std::array<AABB, 4> bounds = {
        makeAABB({-1, -1, -1}, {1, 1, 1}),       // inside
        makeAABB({100, 0, 0}, {101, 1, 1}),       // outside
        makeAABB({9, -1, -1}, {11, 1, 1}),        // intersect
        makeAABB({-1, -1, -1}, {1, 1, 1})         // inside
    };

    const BVH4Node node = makeBVH4Node(bounds);

    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    const uint16_t result =
        testBVH4Node_Scalar(node, planes);

    const uint16_t outside = result & 0x0F;
    const uint16_t inside = (result >> 4) & 0x0F;
    const uint16_t intersect = (result >> 8) & 0x0F;

    EXPECT_EQ(outside, 0b0010);
    EXPECT_EQ(inside,  0b1001);
    EXPECT_EQ(intersect, 0b0100);

    EXPECT_EQ(
        outside | inside | intersect,
        0b1111);
}


// =============================================================================
// SIMD implementations must agree with scalar implementation
// =============================================================================

TEST(BVH4NodeTest, SSE2MatchesScalar)
{
    const std::array<AABB, 4> bounds = {
        makeAABB({-5, -2, -1}, {1, 2, 1}),
        makeAABB({8, -2, -1}, {12, 2, 1}),
        makeAABB({-2, 8, -1}, {2, 12, 1}),
        makeAABB({-2, -2, 8}, {2, 2, 12})
    };

    const BVH4Node node = makeBVH4Node(bounds);
    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    const uint16_t expected =
        testBVH4Node_Scalar(node, planes);

    const uint16_t actual =
        testBVH4Node_SSE2(node, planes);

    EXPECT_EQ(actual, expected);
}


TEST(BVH4NodeTest, SSE2FMAMatchesScalar)
{
    const std::array<AABB, 4> bounds = {
        makeAABB({-5, -2, -1}, {1, 2, 1}),
        makeAABB({8, -2, -1}, {12, 2, 1}),
        makeAABB({-2, 8, -1}, {2, 12, 1}),
        makeAABB({-2, -2, 8}, {2, 2, 12})
    };

    const BVH4Node node = makeBVH4Node(bounds);
    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    const uint16_t expected =
        testBVH4Node_Scalar(node, planes);

    const uint16_t actual =
        testBVH4Node_SSE2_FMA(node, planes);

    EXPECT_EQ(actual, expected);
}


TEST(BVH4NodeTest, SSE41FMAMatchesScalar)
{
    const std::array<AABB, 4> bounds = {
        makeAABB({-5, -2, -1}, {1, 2, 1}),
        makeAABB({8, -2, -1}, {12, 2, 1}),
        makeAABB({-2, 8, -1}, {2, 12, 1}),
        makeAABB({-2, -2, 8}, {2, 2, 12})
    };

    const BVH4Node node = makeBVH4Node(bounds);
    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    const uint16_t expected =
        testBVH4Node_Scalar(node, planes);

    const uint16_t actual =
        testBVH4Node_SSE41_FMA(node, planes);

    EXPECT_EQ(actual, expected);
}


// =============================================================================
// BVH2 culling
// =============================================================================

TEST(BVH2Test, LeafContainerReturnsVisiblePrimitives)
{
    Carbo::ECSAllocator<BVH2Node> allocator;

    const size_t root = allocator.allocate(
        makeBVH2LeafContainer(
            makeAABB({-1, -1, -1}, {1, 1, 1}),
            1,
            2));

    allocator.allocate(
        makeBVH2Leaf(
            makeAABB({-1, -1, -1}, {0, 0, 0}),
            10,
            2,
            root));

    allocator.allocate(
        makeBVH2Leaf(
            makeAABB({0, 0, 0}, {1, 1, 1}),
            20,
            0,
            root));

    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    std::vector<size_t> visible;

    nihil::cullBVH2(
        root,
        planes,
        allocator,
        visible);

    ASSERT_EQ(visible.size(), 2u);

    EXPECT_NE(
        std::find(visible.begin(), visible.end(), 10),
        visible.end());

    EXPECT_NE(
        std::find(visible.begin(), visible.end(), 20),
        visible.end());
}


TEST(BVH2Test, OutsideLeafIsCulled)
{
    Carbo::ECSAllocator<BVH2Node> allocator;

    const size_t root = allocator.allocate(
        makeBVH2LeafContainer(
            makeAABB({100, 0, 0}, {101, 1, 1}),
            1,
            1));

    allocator.allocate(
        makeBVH2Leaf(
            makeAABB({100, 0, 0}, {101, 1, 1}),
            42,
            0,
            root));

    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    std::vector<size_t> visible;

    nihil::cullBVH2(
        root,
        planes,
        allocator,
        visible);

    EXPECT_TRUE(visible.empty());
}


TEST(BVH2Test, IntersectingLeafIsReturned)
{
    Carbo::ECSAllocator<BVH2Node> allocator;

    const size_t root = allocator.allocate(
        makeBVH2LeafContainer(
            makeAABB({9, -1, -1}, {11, 1, 1}),
            1,
            1));

    allocator.allocate(
        makeBVH2Leaf(
            makeAABB({9, -1, -1}, {11, 1, 1}),
            123,
            0,
            root));

    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    std::vector<size_t> visible;

    nihil::cullBVH2(
        root,
        planes,
        allocator,
        visible);

    ASSERT_EQ(visible.size(), 1u);
    EXPECT_EQ(visible[0], 123u);
}


TEST(BVH2Test, ReusableStackIsClearedAndCanBeReused)
{
    Carbo::ECSAllocator<BVH2Node> allocator;

    const size_t root = allocator.allocate(
        makeBVH2LeafContainer(
            makeAABB({-1, -1, -1}, {1, 1, 1}),
            1,
            1));

    allocator.allocate(
        makeBVH2Leaf(
            makeAABB({-1, -1, -1}, {1, 1, 1}),
            7,
            0,
            root));

    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    std::vector<size_t> stack;
    stack.push_back(999999);

    std::vector<size_t> visible;

    nihil::cullBVH2(
        root,
        planes,
        allocator,
        visible,
        &stack);

    ASSERT_EQ(visible.size(), 1u);
    EXPECT_EQ(visible[0], 7u);

    EXPECT_TRUE(stack.empty());

    visible.clear();
    stack.push_back(123456);

    nihil::cullBVH2(
        root,
        planes,
        allocator,
        visible,
        &stack);

    ASSERT_EQ(visible.size(), 1u);
    EXPECT_EQ(visible[0], 7u);

    EXPECT_TRUE(stack.empty());
}


// =============================================================================
// BVH2 internal node traversal
// =============================================================================

TEST(BVH2Test, InternalNodeTraversesBothChildren)
{
    Carbo::ECSAllocator<BVH2Node> allocator;

    BVH2Node root{};
    root.bound = makeAABB({-2, -1, -1}, {2, 1, 1});
    root.parent = 0;
    root.left = 1;
    root.right = 2;
    root.leafCount = 0;

    allocator.allocate(root);

    const size_t left = allocator.allocate(
        makeBVH2LeafContainer(
            makeAABB({-2, -1, -1}, {-0.5f, 1, 1}),
            3,
            1));

    const size_t right = allocator.allocate(
        makeBVH2LeafContainer(
            makeAABB({0.5f, -1, -1}, {2, 1, 1}),
            4,
            1));

    ASSERT_EQ(left, 1u);
    ASSERT_EQ(right, 2u);

    allocator.allocate(
        makeBVH2Leaf(
            makeAABB({-2, -1, -1}, {-0.5f, 1, 1}),
            11,
            0,
            left));

    allocator.allocate(
        makeBVH2Leaf(
            makeAABB({0.5f, -1, -1}, {2, 1, 1}),
            22,
            0,
            right));

    const auto planes = makeBoxFrustum(10.0f, 10.0f, 10.0f);

    std::vector<size_t> visible;

    nihil::cullBVH2(
        0,
        planes,
        allocator,
        visible);

    ASSERT_EQ(visible.size(), 2u);

    EXPECT_NE(
        std::find(visible.begin(), visible.end(), 11),
        visible.end());

    EXPECT_NE(
        std::find(visible.begin(), visible.end(), 22),
        visible.end());
}


// =============================================================================
// BVH2 surface area invariants
// =============================================================================

TEST(BVH2Test, LeafContainerStoresOriginalSurfaceArea)
{
    const AABB bound =
        makeAABB({-2, -3, -4}, {2, 3, 4});

    const BVH2Node node =
        makeBVH2LeafContainer(bound, 1, 1);

    EXPECT_NEAR(
        node.originalSurfaceArea,
        bound._surfaceArea(),
        EPSILON);
}


TEST(BVH4Test, LeafNodeStoresPerChildBounds)
{
    const std::array<AABB, 4> bounds = {
        makeAABB({0, 1, 2}, {3, 4, 5}),
        makeAABB({-3, -4, -5}, {-1, -2, -3}),
        makeAABB({10, 20, 30}, {40, 50, 60}),
        makeAABB({-10, -20, -30}, {-5, -10, -15})
    };

    const BVH4Node node = makeBVH4Node(bounds);

    for (size_t i = 0; i < 4; ++i)
    {
        EXPECT_FLOAT_EQ(node.minX[i], bounds[i].min.x);
        EXPECT_FLOAT_EQ(node.minY[i], bounds[i].min.y);
        EXPECT_FLOAT_EQ(node.minZ[i], bounds[i].min.z);

        EXPECT_FLOAT_EQ(node.maxX[i], bounds[i].max.x);
        EXPECT_FLOAT_EQ(node.maxY[i], bounds[i].max.y);
        EXPECT_FLOAT_EQ(node.maxZ[i], bounds[i].max.z);

        EXPECT_EQ(node.children[i], i);
    }
}
