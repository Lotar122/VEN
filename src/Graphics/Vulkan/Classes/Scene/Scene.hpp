#pragma once

#include <tuple>
#include <pair>

namespace nihil::graphics
{
    class Scene
    {
        Engine* engine = nullptr;
    public:
        std::vector<Object*> objects;
        std::vector<std::pair<ObjectUsage, ObjectPlacement>> objectsUsages;
    };
}