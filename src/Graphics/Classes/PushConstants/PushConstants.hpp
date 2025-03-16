#pragma once

#include <glm/glm.hpp>

namespace nihil::graphics
{
    struct PushConstants
    {
        //The view and projection matricies are premultiplied on the CPU to save space in the push constants
        glm::mat4 vp;
        //The model matrix, unused in a instanced rendering setup, might be mat4(1.0f)
        glm::mat4 model;
    };
}