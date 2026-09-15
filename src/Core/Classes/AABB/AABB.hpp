#pragma once

#include <glm/glm.hpp>
#include <limits>
#include <vector>

#include "Classes/Plane/Plane.hpp"
#include "Enums/VisibilityQueryResult.hpp"
#include "Classes/Logger/Logger.hpp"

namespace nihil
{
    ///Represents an axis-aligner bounding box
    class AABB
    {
    public:
        ///The smallest coordinate point for AABB
        glm::vec3 min;
        ///The biggest coordinate point for AABB
        glm::vec3 max;

        ///Default constructor
        AABB()
        {
            min = glm::vec3(std::numeric_limits<float>::max());
            max = glm::vec3(std::numeric_limits<float>::lowest());
        }

        ///Constructs from glm::vec3
        //
        ///@param _min The smallest coordinate point for AABB
        ///@param _max The biggest coordinate point for AABB
        ///@return An AABB from points
        AABB(const glm::vec3& _min, const glm::vec3& _max)
        {
            min = _min;
            max = _max;
        }

        ///Checks whether the AABB is inside the view frustum.
        //
        ///@param box The AABB to be tested
        ///@param planes The view frustum defined by 6 planes
        ///@return a VisibilityQueryResult for the AABB
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

        ///Computes a bounding box for a mesh
        //
        ///@param vertices The mesh as a flat std::vector of floats layout: (vx, vy, vz, tx, ty, nx, ny, nz)
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

        ///Returns the AABB transformed by a matrix
        //
        ///@param M The matrix containing the transformations
        ///@return The transfromed AABB
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

        ///Expands the AABB
        //
        ///@param bounds Another AABB that the AABB needs to fit
        void expand(const AABB& bounds)
        {
            min = glm::min(bounds.min, min);
            max = glm::max(bounds.max, max);
        }

        ///Expands the AABB
        //
        ///@param p A point which the AABB needs to fit
        void expand(const glm::vec3& p)
        {
            min = glm::min(min, p);
            max = glm::max(max, p);
        }

        ///Computes the centroid of this AABB
        //
        ///@return The centroid point
        inline const glm::vec3 _centroid() const
        {
            return (min + max) * 0.5f;
        }

        ///Computes the centroid of an AABB described by two points.
        //
        ///@param min The smallest coordinate point for AABB
        ///@param max The biggest coordinate point for AABB
        ///@return The centroid point
        inline static const glm::vec3 centroid(const glm::vec3& min, const glm::vec3& max)
        {
            return (min + max) * 0.5f;
        }

        ///Computes the extent of this AABB
        //
        ///@return The extent of this AABB
        inline const glm::vec3 _extent() const
        {
            return max - min;
        }

        ///Computes an extent of an AABB described by two points
        //
        ///@param min The smallest coordinate point for AABB
        ///@param max The biggest coordinate point for AABB
        ///@return The extent of an AABB
        inline static const glm::vec3 extent(const glm::vec3& min, const glm::vec3& max)
        {
            return max - min;
        }

        ///Computes the nth longest axis
        //
        ///@tparam n which axis to compute
        template<size_t n = 0>
        size_t longestAxis() const
        {
            const glm::vec3& e = _extent();

            if constexpr (n == 0)
            {
                if (e.x >= e.y && e.x >= e.z) return 0;
                if (e.y >= e.z) return 1;
                return 2;
            }
            else if constexpr (n == 1)
            {
                if (e.x >= e.y && e.x <= e.z) return 0;
                if (e.y >= e.x && e.y <= e.z) return 1;
                return 2;
            }
            else
            {
                if (e.x <= e.y && e.x <= e.z) return 0;
                if (e.y <= e.z) return 1;
                return 2;
            }
        }

        ///Computes the surface area of this AABB
        //
        ///@return The surface area of this AABB
        inline float _surfaceArea() const
        {
            const glm::vec3 e = _extent();
            return 2.0f * (e.x*e.y + e.x*e.z + e.y*e.z);
        }

        ///Computes the surface area of an AABB described by two points
        //
        ///@param min The smallest coordinate point for AABB
        ///@param max The biggest coordinate point for AABB
        ///@return The surface area
        inline static float surfaceArea(const glm::vec3& min, const glm::vec3& max)
        {
            const glm::vec3 e = extent(min, max);
            return 2.0f * (e.x*e.y + e.x*e.z + e.y*e.z);
        }
    };
}
