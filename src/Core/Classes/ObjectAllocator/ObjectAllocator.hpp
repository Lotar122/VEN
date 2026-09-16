#pragma once

#include "Classes/BlockAllocator/BlockAllocator.hpp"
#include "Classes/ECSAllocator/ECSAllocator.hpp"
#include <glm/glm.hpp>
#include "Classes/AABB/AABB.hpp"

namespace nihil
{
    ///Just a bundle for the ECS style allocated part of graphics::Object
    class ObjectAllocator
    {
    public:
        Carbo::ECSAllocator<glm::mat4> modelMatricies;
        Carbo::ECSAllocator<AABB> AABBs;
        Carbo::ECSAllocator<AABB> transformedAABBs;
    };
}