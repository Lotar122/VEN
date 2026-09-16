#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Classes/AABB/AABB.hpp"

using namespace nihil;

namespace
{
    constexpr float EPSILON = 1e-5f;

    void expectVec3Near(
        const glm::vec3& actual,
        const glm::vec3& expected,
        float epsilon = EPSILON)
    {
        EXPECT_NEAR(actual.x, expected.x, epsilon);
        EXPECT_NEAR(actual.y, expected.y, epsilon);
        EXPECT_NEAR(actual.z, expected.z, epsilon);
    }

    std::array<Plane, 6> makeUnitFrustum()
    {
        // Cube/frustum:
        //
        //       -1 <= x <= 1
        //       -1 <= y <= 1
        //       -1 <= z <= 1
        //
        // Plane convention:
        //     dot(normal, point) + d >= 0 => inside

        std::array<Plane, 6> planes{};

        // x >= -1
        planes[0].normal = { 1.0f, 0.0f, 0.0f };
        planes[0].d = 1.0f;

        // x <= 1
        planes[1].normal = { -1.0f, 0.0f, 0.0f };
        planes[1].d = 1.0f;

        // y >= -1
        planes[2].normal = { 0.0f, 1.0f, 0.0f };
        planes[2].d = 1.0f;

        // y <= 1
        planes[3].normal = { 0.0f, -1.0f, 0.0f };
        planes[3].d = 1.0f;

        // z >= -1
        planes[4].normal = { 0.0f, 0.0f, 1.0f };
        planes[4].d = 1.0f;

        // The implementation currently skips plane 5, treating it as
        // the far plane.
        planes[5].normal = { 0.0f, 0.0f, -1.0f };
        planes[5].d = 1.0f;

        return planes;
    }
}

// -----------------------------------------------------------------------------
// Constructors
// -----------------------------------------------------------------------------

TEST(AABB, DefaultConstructorCreatesEmptyBounds)
{
    nihil::AABB box;

    EXPECT_EQ(box.min.x, std::numeric_limits<float>::max());
    EXPECT_EQ(box.min.y, std::numeric_limits<float>::max());
    EXPECT_EQ(box.min.z, std::numeric_limits<float>::max());

    EXPECT_EQ(box.max.x, std::numeric_limits<float>::lowest());
    EXPECT_EQ(box.max.y, std::numeric_limits<float>::lowest());
    EXPECT_EQ(box.max.z, std::numeric_limits<float>::lowest());
}

TEST(AABB, ConstructorFromMinAndMax)
{
    const glm::vec3 min(-1.0f, -2.0f, -3.0f);
    const glm::vec3 max(4.0f, 5.0f, 6.0f);

    nihil::AABB box(min, max);

    expectVec3Near(box.min, min);
    expectVec3Near(box.max, max);
}

// -----------------------------------------------------------------------------
// computeFromMesh
// -----------------------------------------------------------------------------

TEST(AABB, ComputeFromMesh)
{
    nihil::AABB box;

    // Layout:
    // x, y, z, tx, ty, nx, ny, nz
    const std::vector<float> vertices =
    {
        -1.0f,  2.0f,  3.0f, 0, 0, 0, 0, 1,
         4.0f, -2.0f,  1.0f, 0, 0, 0, 0, 1,
         2.0f,  5.0f, -6.0f, 0, 0, 0, 0, 1,
         0.0f,  1.0f,  2.0f, 0, 0, 0, 0, 1
    };

    box.computeFromMesh(vertices);

    expectVec3Near(
        box.min,
        glm::vec3(-1.0f, -2.0f, -6.0f));

    expectVec3Near(
        box.max,
        glm::vec3(4.0f, 5.0f, 3.0f));
}

TEST(AABB, ComputeFromEmptyMeshCreatesZeroBounds)
{
    nihil::AABB box;

    box.computeFromMesh({});

    expectVec3Near(box.min, glm::vec3(0.0f));
    expectVec3Near(box.max, glm::vec3(0.0f));
}

// -----------------------------------------------------------------------------
// expand
// -----------------------------------------------------------------------------

TEST(AABB, ExpandByAABB)
{
    nihil::AABB box(
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f));

    nihil::AABB other(
        glm::vec3(-4.0f, 0.0f, -2.0f),
        glm::vec3(2.0f, 5.0f, 3.0f));

    box.expand(other);

    expectVec3Near(
        box.min,
        glm::vec3(-4.0f, -1.0f, -2.0f));

    expectVec3Near(
        box.max,
        glm::vec3(2.0f, 5.0f, 3.0f));
}

TEST(AABB, ExpandByPoint)
{
    nihil::AABB box(
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f));

    box.expand(glm::vec3(4.0f, -3.0f, 2.0f));

    expectVec3Near(
        box.min,
        glm::vec3(-1.0f, -3.0f, -1.0f));

    expectVec3Near(
        box.max,
        glm::vec3(4.0f, 1.0f, 2.0f));
}

// -----------------------------------------------------------------------------
// centroid / extent
// -----------------------------------------------------------------------------

TEST(AABB, Centroid)
{
    nihil::AABB box(
        glm::vec3(-2.0f, -4.0f, 2.0f),
        glm::vec3(4.0f, 6.0f, 8.0f));

    expectVec3Near(
        box._centroid(),
        glm::vec3(1.0f, 1.0f, 5.0f));

    expectVec3Near(
        nihil::AABB::centroid(box.min, box.max),
        glm::vec3(1.0f, 1.0f, 5.0f));
}

TEST(AABB, Extent)
{
    nihil::AABB box(
        glm::vec3(-2.0f, -4.0f, 2.0f),
        glm::vec3(4.0f, 6.0f, 8.0f));

    expectVec3Near(
        box._extent(),
        glm::vec3(6.0f, 10.0f, 6.0f));

    expectVec3Near(
        nihil::AABB::extent(box.min, box.max),
        glm::vec3(6.0f, 10.0f, 6.0f));
}

// -----------------------------------------------------------------------------
// longestAxis
// -----------------------------------------------------------------------------

TEST(AABB, LongestAxis)
{
    nihil::AABB box(
        glm::vec3(0.0f),
        glm::vec3(2.0f, 5.0f, 3.0f));

    EXPECT_EQ(box.longestAxis<0>(), 1);
}

TEST(AABB, SecondLongestAxis)
{
    nihil::AABB box(
        glm::vec3(0.0f),
        glm::vec3(2.0f, 5.0f, 3.0f));

    EXPECT_EQ(box.longestAxis<1>(), 2);
}

TEST(AABB, ShortestAxis)
{
    nihil::AABB box(
        glm::vec3(0.0f),
        glm::vec3(2.0f, 5.0f, 3.0f));

    EXPECT_EQ(box.longestAxis<2>(), 0);
}

TEST(AABB, LongestAxisWithEqualDimensions)
{
    nihil::AABB box(
        glm::vec3(0.0f),
        glm::vec3(5.0f, 5.0f, 2.0f));

    // Tie between X and Y is resolved in favor of X.
    EXPECT_EQ(box.longestAxis<0>(), 0);
}

// -----------------------------------------------------------------------------
// surfaceArea
// -----------------------------------------------------------------------------

TEST(AABB, SurfaceArea)
{
    nihil::AABB box(
        glm::vec3(0.0f),
        glm::vec3(2.0f, 3.0f, 4.0f));

    // 2 * (2*3 + 2*4 + 3*4) = 52
    EXPECT_FLOAT_EQ(box._surfaceArea(), 52.0f);

    EXPECT_FLOAT_EQ(
        nihil::AABB::surfaceArea(box.min, box.max),
        52.0f);
}

TEST(AABB, SurfaceAreaOfPointIsZero)
{
    nihil::AABB box(
        glm::vec3(5.0f),
        glm::vec3(5.0f));

    EXPECT_FLOAT_EQ(box._surfaceArea(), 0.0f);
}

// -----------------------------------------------------------------------------
// getTransformed
// -----------------------------------------------------------------------------

TEST(AABB, Translated)
{
    nihil::AABB box(
        glm::vec3(-1.0f, -2.0f, -3.0f),
        glm::vec3(1.0f, 2.0f, 3.0f));

    const glm::mat4 transform =
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(10.0f, 20.0f, 30.0f));

    const nihil::AABB transformed = box.getTransformed(transform);

    expectVec3Near(
        transformed.min,
        glm::vec3(9.0f, 18.0f, 27.0f));

    expectVec3Near(
        transformed.max,
        glm::vec3(11.0f, 22.0f, 33.0f));
}

TEST(AABB, Scaled)
{
    nihil::AABB box(
        glm::vec3(-1.0f, -2.0f, -3.0f),
        glm::vec3(1.0f, 2.0f, 3.0f));

    const glm::mat4 transform =
        glm::scale(
            glm::mat4(1.0f),
            glm::vec3(2.0f, 3.0f, 4.0f));

    const nihil::AABB transformed = box.getTransformed(transform);

    expectVec3Near(
        transformed.min,
        glm::vec3(-2.0f, -6.0f, -12.0f));

    expectVec3Near(
        transformed.max,
        glm::vec3(2.0f, 6.0f, 12.0f));
}

TEST(AABB, IdentityTransformDoesNotChangeBounds)
{
    nihil::AABB box(
        glm::vec3(-2.0f, -3.0f, -4.0f),
        glm::vec3(5.0f, 6.0f, 7.0f));

    const nihil::AABB transformed =
        box.getTransformed(glm::mat4(1.0f));

    expectVec3Near(transformed.min, box.min);
    expectVec3Near(transformed.max, box.max);
}

// -----------------------------------------------------------------------------
// Frustum visibility
// -----------------------------------------------------------------------------

TEST(AABB, FrustumContainsAABB)
{
    const auto planes = makeUnitFrustum();

    nihil::AABB box(
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f, 0.5f, 0.5f));

    EXPECT_EQ(
        nihil::AABB::isAABBVisible(box, planes),
        VisibilityQueryResult::Inside);
}

TEST(AABB, FrustumRejectsAABBOutside)
{
    const auto planes = makeUnitFrustum();

    nihil::AABB box(
        glm::vec3(5.0f, 5.0f, 5.0f),
        glm::vec3(6.0f, 6.0f, 6.0f));

    EXPECT_EQ(
        nihil::AABB::isAABBVisible(box, planes),
        VisibilityQueryResult::Outside);
}

TEST(AABB, FrustumDetectsIntersectingAABB)
{
    const auto planes = makeUnitFrustum();

    nihil::AABB box(
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(1.5f, 0.5f, 0.5f));

    EXPECT_EQ(
        nihil::AABB::isAABBVisible(box, planes),
        VisibilityQueryResult::Intersection);
}