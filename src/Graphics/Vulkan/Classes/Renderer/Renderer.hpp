#pragma once

#include <vulkan/vulkan.hpp>
#include "Logger.hpp"

#include "Classes/PushConstants/PushConstants.hpp"

#include "Classes/Listeners/Listeners.hpp"

#include "Classes/Camera/Camera.hpp"

namespace nihil::graphics
{
    class Engine;
    class Pipeline;
    class RenderPass;
    template<typename T, auto usageT, auto propertiesT>
    class Buffer;
    class Scene;
    class DescriptorAllocator;

    class Renderer : public onResizeListener
    {   
        friend class Engine;
        Engine* engine = nullptr;

        vk::Viewport viewport;
        vk::Rect2D scissor;

        //! REMOVE
        uint64_t debugCounter = 0;

    public:
        const inline vk::Viewport& _viewport() const { return viewport; };
        const inline vk::Rect2D& _scissor() const { return scissor; };

        Renderer(Engine* _engine);

        //Add draw queueing in the future

        void Render(
            RenderPass* renderPass, 
            Scene* scene,
            Camera* camera,
            DescriptorAllocator* descriptorAllocator = nullptr
        );

    private:
        void onResize() final override;
    };
}