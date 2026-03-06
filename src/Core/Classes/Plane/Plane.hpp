#pragma once

#include <array>
#include <glm/glm.hpp>

namespace nihil
{
    struct Plane
    {
        glm::vec3 normal;
        float d;
    };

    std::array<Plane, 6> extractFrustumPlanes(const glm::mat4& m);
}