#pragma once

#include "Classes/DescriptorSet/DescriptorSet.hpp"
#include "Classes/PushConstants/PushConstants.hpp"
#include "Classes/RenderPass/RenderPass.hpp"
#include "Classes/Swapchain/Swapchain.hpp"
#include "Classes/Resources/Resources.hpp"
#include "Classes/AssetUsage/AssetUsage.hpp"
#include "Classes/DescriptorAllocator/DescriptorAllocator.hpp"
#include <vulkan/vulkan.hpp>
#include "Logger.hpp"

namespace nihil::graphics
{
    using VertexBinding = vk::VertexInputBindingDescription;
    using VertexAttribute = vk::VertexInputAttributeDescription;

    struct PipelineCreateInfo
    {
        std::vector<vk::VertexInputBindingDescription>& bindingDesc;
        std::vector<vk::VertexInputAttributeDescription>& attributeDesc;

        std::vector<DescriptorSetLayoutBinding>& descriptorSetLayoutBindings;

        vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;

        vk::ShaderModule* vertexShader = nullptr;
        vk::ShaderModule* fragmentShader = nullptr;

        vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
        vk::CullModeFlags cullingMode = vk::CullModeFlagBits::eBack;
        vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;

        uint32_t depthClampEnable = VK_FALSE;
        uint32_t rasterizerDiscardEnable = VK_FALSE;
        uint32_t depthBiasEnable = VK_FALSE;

        uint32_t anistropyEnable = VK_FALSE;

        uint32_t sampleShadingEnable = VK_FALSE;
        vk::SampleCountFlagBits rasterizationSampleCount = vk::SampleCountFlagBits::e1;
    };

    class Pipeline
    {
        DescriptorSet<AssetUsage::Static> dynamicDescriptorSet;

        Resource<vk::Pipeline> pipeline;
        Resource<vk::PipelineLayout> layout;
        RenderPass* baseRenderPass = nullptr;
        Engine* engine = nullptr;

        bool destroyed = false;

    public:
        inline vk::Pipeline _pipeline() { return pipeline; };
        inline vk::PipelineLayout _layout() {return layout; };
        inline RenderPass* _baseRenderPass() { return baseRenderPass; };

        Pipeline(Engine* _engine);
        ~Pipeline();

        void destroy();

        void create(PipelineCreateInfo& info, RenderPass* _renderPass, DescriptorAllocator* descriptorAllocator = nullptr);
    };
}