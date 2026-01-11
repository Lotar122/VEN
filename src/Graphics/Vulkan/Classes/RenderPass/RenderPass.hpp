#pragma once

#include "Classes/Swapchain/Swapchain.hpp"
#include <vulkan/vulkan.hpp>

namespace nihil::graphics
{
    enum class RenderPassAttachmentType
    {
        ColorAttachment,
        ResolveAttachment,
        DepthAttachment
    };

    //in the future create a specialized struct
    struct RenderPassAttachment
    {
        RenderPassAttachmentType type;

        vk::SampleCountFlagBits sampleCount;
        vk::AttachmentLoadOp loadOp;
        vk::AttachmentStoreOp storeOp;

        vk::AttachmentLoadOp stencilLoadOp;
        vk::AttachmentStoreOp stencilStoreOp;

        vk::ImageLayout initialImageLayout;
        vk::ImageLayout finalImageLayout;

        uint32_t refIndex;
    };

    class RenderPass
    {
        Resource<vk::RenderPass> renderPass;

        std::vector<RenderPassAttachment> attachments;
        std::vector<vk::SubpassDescription> subpasses;
        std::vector<vk::AttachmentDescription> colorAttachmentDescriptors;
        std::vector<vk::AttachmentDescription> resolveAttachmentDescriptos;
        std::vector<vk::AttachmentDescription> depthAttachmentDescriptors;

        Swapchain* swapchain = nullptr;
        vk::Device device;

    public:
        inline vk::RenderPass _renderPass() { return renderPass.getRes(); };

        RenderPass(std::vector<RenderPassAttachment>& _attachments, Swapchain* _swapchain, vk::Device _device);

        ~RenderPass();
    };
}