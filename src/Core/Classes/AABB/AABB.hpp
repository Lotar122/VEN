#pragma once

#include <glm/glm.hpp>
#include <limits>

#include "Classes/Plane/Plane.hpp"
#include "Enums/VisibilityQueryResult.hpp"

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

        static VisibilityQueryResult isAABBVisible(const AABB& box, const std::array<Plane,6>& planes)
        {
            bool intersect = false;

            for (const Plane& plane : planes)
            {
                glm::vec3 positive = box.min;
                glm::vec3 negative = box.max;

                if (plane.normal.x >= 0) {
                    positive.x = box.max.x;
                    negative.x = box.min.x;
                }
                if (plane.normal.y >= 0) {
                    positive.y = box.max.y;
                    negative.y = box.min.y;
                }
                if (plane.normal.z >= 0) {
                    positive.z = box.max.z;
                    negative.z = box.min.z;
                }

                float dPos = glm::dot(plane.normal, positive) + plane.d;
                float dNeg = glm::dot(plane.normal, negative) + plane.d;

                if (dPos < 0)
                    return VisibilityQueryResult::Outside;

                if (dNeg < 0)
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