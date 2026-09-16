#include <gtest/gtest.h>

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Classes/Plane/Plane.hpp"

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

    void expectPlaneNear(
        const nihil::Plane& actual,
        const glm::vec3& normal,
        float d,
        float epsilon = EPSILON)
    {
        expectVec3Near(actual.normal, normal, epsilon);
        EXPECT_NEAR(actual.d, d, epsilon);
    }
}

TEST(FrustumPlanes, IdentityMatrix)
{
    const auto planes = nihil::extractFrustumPlanes(glm::mat4(1.0f));

    // For the identity matrix, the extracted clip-space planes are:
    //
    // Left   :  x + 1 >= 0
    // Right  : -x + 1 >= 0
    // Bottom :  y + 1 >= 0
    // Top    : -y + 1 >= 0
    // Near   :  z + 1 >= 0
    // Far    : -z + 1 >= 0

    expectPlaneNear(
        planes[0],
        glm::vec3(1.0f, 0.0f, 0.0f),
        1.0f);

    expectPlaneNear(
        planes[1],
        glm::vec3(-1.0f, 0.0f, 0.0f),
        1.0f);

    expectPlaneNear(
        planes[2],
        glm::vec3(0.0f, 1.0f, 0.0f),
        1.0f);

    expectPlaneNear(
        planes[3],
        glm::vec3(0.0f, -1.0f, 0.0f),
        1.0f);

    expectPlaneNear(
        planes[4],
        glm::vec3(0.0f, 0.0f, 1.0f),
        1.0f);

    expectPlaneNear(
        planes[5],
        glm::vec3(0.0f, 0.0f, -1.0f),
        1.0f);
}

TEST(FrustumPlanes, PlanesAreNormalized)
{
    const glm::mat4 matrix =
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(10.0f, 20.0f, 30.0f));

    const auto planes = nihil::extractFrustumPlanes(matrix);

    for (const auto& plane : planes)
    {
        EXPECT_NEAR(
            glm::length(plane.normal),
            1.0f,
            EPSILON);
    }
}

TEST(FrustumPlanes, PerspectiveProjection)
{
    const glm::mat4 projection =
        glm::perspective(
            glm::radians(90.0f),
            1.0f,
            1.0f,
            100.0f);

    const auto planes = nihil::extractFrustumPlanes(projection);

    for (const auto& plane : planes)
    {
        EXPECT_NEAR(
            glm::length(plane.normal),
            1.0f,
            EPSILON);
    }
}

TEST(FrustumPlanes, ExtractedPlanesHaveExpectedDirections)
{
    const glm::mat4 projection =
        glm::perspective(
            glm::radians(90.0f),
            1.0f,
            1.0f,
            100.0f);

    const auto planes = nihil::extractFrustumPlanes(projection);

    // The normals should point toward the inside of the frustum.
    //
    // Test representative points known to be inside the frustum.
    const glm::vec3 inside(0.0f, 0.0f, -10.0f);

    for (const auto& plane : planes)
    {
        const float distance =
            glm::dot(plane.normal, inside) + plane.d;

        EXPECT_GE(distance, -EPSILON);
    }
}