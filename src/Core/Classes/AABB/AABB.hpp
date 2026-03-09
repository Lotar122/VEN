#pragma once

#include <glm/glm.hpp>
#include <limits>

#include "Classes/Plane/Plane.hpp"
#include "Enums/VisibilityQueryResult.hpp"
#include "Classes/Logger/Logger.hpp"

namespace nihil
{
    class AABB
    {
    public:
        glm::vec3 min;
        glm::vec3 max;

        AABB()
        {
            min = glm::vec3(std::numeric_limits<float>::max());
            max = glm::vec3(std::numeric_limits<float>::lowest());
        }

        AABB(const glm::vec3& _min, const glm::vec3& _max)
        {
            min = _min;
            max = _max;
        }

        static VisibilityQueryResult isAABBVisible(const AABB& box, const std::array<Plane, 6>& planes)
        {
            constexpr float bias = 0.01f;
            AABB expanded = box;
            expanded.min -= glm::vec3(bias);
            expanded.max += glm::vec3(bias);

            glm::vec3 center = (expanded.min + expanded.max) * 0.5f;
            glm::vec3 extent = (expanded.max - expanded.min) * 0.5f;

            bool intersect = false;

            for (int i = 0; i < 4; i++) // skip near
            {
                const Plane& p = planes[i];
                float dist = glm::dot(p.normal, center) + p.d;

                float radius =
                    extent.x * std::abs(p.normal.x) +
                    extent.y * std::abs(p.normal.y) +
                    extent.z * std::abs(p.normal.z);

                Carbo::Logger::Log("Pos:{}, Neg:{}", dist + radius, dist - radius);

                if (dist + radius < -0.01f)
                    return VisibilityQueryResult::Outside;

                if (dist - radius < -0.01f)
                    intersect = true;
            }

            return intersect ? VisibilityQueryResult::Intersection
                : VisibilityQueryResult::Inside;
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

            max = glm::vec3(std::numeric_limits<float>::lowest()); 
            min = glm::vec3(std::numeric_limits<float>::max());

            for(size_t i = 0; i < vertices.size(); i += 8)
            {
                glm::vec3 pos(vertices[i], vertices[i + 1], vertices[i + 2]);

                min = glm::min(min, pos);
                max = glm::max(max, pos);
            }

            Carbo::Logger::Log("Min : (x:{}, y:{}, z:{}) Max : (x:{}, y:{}, z:{})", min.x, min.y, min.z, max.x, max.y, max.z);
        }

        AABB getTransformed(const glm::mat4& M) const
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

        void expand(const AABB& bounds)
        {
            min = glm::min(bounds.min, min);
            max = glm::max(bounds.max, max);
        }

        void expand(const glm::vec3& p)
        {
            min.x = std::min(min.x, p.x);
            min.y = std::min(min.y, p.y);
            min.z = std::min(min.z, p.z);

            max.x = std::max(max.x, p.x);
            max.y = std::max(max.y, p.y);
            max.z = std::max(max.z, p.z);
        }

        inline glm::vec3 centroid() const
        {
            return (min + max) * 0.5f;
        }

        inline glm::vec3 extent() const
        {
            return max - min;
        }

        size_t longestAxis() const
        {
            glm::vec3 e = extent();

            if (e.x > e.y && e.x > e.z) return 0;
            if (e.y > e.z) return 1;
            return 2;
        }

        inline float surfaceArea()
        {
            glm::vec3 e = extent();
            return 2.0f * (e.x*e.y + e.x*e.z + e.y*e.z);
        }
    };
}