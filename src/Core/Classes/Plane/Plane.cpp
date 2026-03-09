#include "Plane.hpp"

using namespace nihil;

std::array<Plane, 6> nihil::extractFrustumPlanes(const glm::mat4& m)
{
    std::array<Plane, 6> planes;

    // Left
    planes[0].normal.x = m[0][3] + m[0][0];
    planes[0].normal.y = m[1][3] + m[1][0];
    planes[0].normal.z = m[2][3] + m[2][0];
    planes[0].d        = m[3][3] + m[3][0];

    // Right
    planes[1].normal.x = m[0][3] - m[0][0];
    planes[1].normal.y = m[1][3] - m[1][0];
    planes[1].normal.z = m[2][3] - m[2][0];
    planes[1].d        = m[3][3] - m[3][0];

    // Bottom
    planes[2].normal.x = m[0][3] + m[0][1];
    planes[2].normal.y = m[1][3] + m[1][1];
    planes[2].normal.z = m[2][3] + m[2][1];
    planes[2].d        = m[3][3] + m[3][1];

    // Top
    planes[3].normal.x = m[0][3] - m[0][1];
    planes[3].normal.y = m[1][3] - m[1][1];
    planes[3].normal.z = m[2][3] - m[2][1];
    planes[3].d        = m[3][3] - m[3][1];

    // Near
    planes[4].normal.x = m[0][3] + m[0][2];
    planes[4].normal.y = m[1][3] + m[1][2];
    planes[4].normal.z = m[2][3] + m[2][2];
    planes[4].d        = m[3][3] + m[3][2];

    // Far
    planes[5].normal.x = m[0][3] - m[0][2];
    planes[5].normal.y = m[1][3] - m[1][2];
    planes[5].normal.z = m[2][3] - m[2][2];
    planes[5].d        = m[3][3] - m[3][2];

    // Normalize planes
    for (Plane& p : planes)
    {
        float len = glm::length(p.normal);
        if (len > 0.0f)
        {
            p.normal /= len;
            p.d /= len;
        }
    }

    return planes;
}