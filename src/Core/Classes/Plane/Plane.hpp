#pragma once

#include <array>
#include <glm/glm.hpp>

namespace nihil
{
    ///The representation of a plane in 3D space
    struct Plane
    {
        ///The planes normal
        glm::vec3 normal;
        ///The distance from origin across the normal
        float d;
        //glm::vec3 absNormal; // precomputed
    };

    ///The function to extract frustum planes from a camera matrix m
    //
    ///@param m The matrix from which the planes will be extracted
    std::array<Plane, 6> extractFrustumPlanes(const glm::mat4& m);
}