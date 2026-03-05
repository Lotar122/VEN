#pragma once

#include <glm/glm.hpp>
#include <limits>

#include "Classes/Plane/Plane.hpp"

class AABB
{
public:
    glm::vec3 min;
    glm::vec3 max;

    bool isAABBVisible(const std::array<Plane, 6>& planes)
    {
        for (int i = 0; i < 6; i++)
        {
            const Plane& p = planes[i];

            glm::vec3 positive;

            positive.x = (p.normal.x >= 0.0f) ? max.x : min.x;
            positive.y = (p.normal.y >= 0.0f) ? max.y : min.y;
            positive.z = (p.normal.z >= 0.0f) ? max.z : min.z;

            if (glm::dot(p.normal, positive) + p.d < 0.0f) return false; // Outside this plane
        }

        return true;
    }

    //Computes a bounding box for any mesh layout: (vx, vy, vz, tx, ty, nx, ny, nz)
    void computeFromMesh(const std::vector<float>& vertices)
    {
        assert((vertices.size() % 8) == 0);
        if(vertices.empty()) [[unlikely]] 
        { 
            min = glm::vec3(0.0f);
            max = glm::vec3(0.0f); 

            return;
        }

        max = glm::vec3(std::numeric_limits<float>::max()); 
        min = glm::vec3(std::numeric_limits<float>::lowest());

        for(size_t i = 0; i < vertices.size(); i += 8)
        {
            glm::vec3 pos(vertices[i], vertices[i + 1], vertices[i + 2]);

            min = glm::min(min, pos);
            max = glm::max(max, pos);
        }
    }

    AABB getTransformed(const glm::mat4& M)
    {
        glm::vec3 corners[8] =
        {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {min.x, max.y, min.z},
            {max.x, max.y, min.z},
            {min.x, min.y, max.z},
            {max.x, min.y, max.z},
            {min.x, max.y, max.z},
            {max.x, max.y, max.z}
        };

        glm::vec3 newMin(std::numeric_limits<float>::max());
        glm::vec3 newMax(std::numeric_limits<float>::lowest());

        for (int i = 0; i < 8; i++)
        {
            glm::vec3 p = glm::vec3(M * glm::vec4(corners[i], 1.0));

            newMin = glm::min(newMin, p);
            newMax = glm::max(newMax, p);
        }

        return { newMin, newMax };
    }
};
