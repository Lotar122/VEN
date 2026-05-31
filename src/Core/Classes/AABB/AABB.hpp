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
        glm::vec3 centroid;
        glm::vec3 extent;

        AABB()
        {
            min = glm::vec3(std::numeric_limits<float>::max());
            max = glm::vec3(std::numeric_limits<float>::lowest());

            centroid = (min + max) * 0.5f;
            extent = max - min;
        }

        AABB(const glm::vec3& _min, const glm::vec3& _max)
        {
            min = _min;
            max = _max;

            centroid = (min + max) * 0.5f;
            extent = max - min;
        }

        static VisibilityQueryResult isAABBVisible(const AABB& box, const std::array<Plane, 6>& planes)
        {
            glm::vec3 center = (box.min + box.max) * 0.5f;
            glm::vec3 extent = (box.max - box.min) * 0.5f;

            constexpr float epsilon = -0.01f;
            VisibilityQueryResult result = VisibilityQueryResult::Inside;

            for (int i = 0; i < 5; i++) // skip far plane
            {
                const Plane& p = planes[i];
                float dist = glm::dot(p.normal, center) + p.d;

                glm::vec3 radiusVec = extent * glm::abs(p.normal);

                float radius =
                    radiusVec.x + radiusVec.y + radiusVec.z;

                if (dist + radius < epsilon)
                    return VisibilityQueryResult::Outside;

                if (dist - radius < epsilon)
                    result = VisibilityQueryResult::Intersection;
            }

            return result;
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

            centroid = (min + max) * 0.5f;
            extent = max - min;

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

            centroid = (min + max) * 0.5f;
            extent = max - min;
        }

        void expand(const glm::vec3& p)
        {
            min = glm::min(min, p);
            max = glm::max(max, p);

            centroid = (min + max) * 0.5f;
            extent = max - min;
        }

        inline const glm::vec3& _centroid() const
        {
            return centroid;
        }

        inline const glm::vec3& _extent() const
        {
            return extent;
        }

        size_t longestAxis() const
        {
            const glm::vec3& e = extent;

            if (e.x > e.y && e.x > e.z) return 0;
            if (e.y > e.z) return 1;
            return 2;
        }

        inline float surfaceArea() const
        {
            const glm::vec3& e = extent;
            return 2.0f * (e.x*e.y + e.x*e.z + e.y*e.z);
        }
    };
}
