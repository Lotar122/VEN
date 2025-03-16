#pragma once

#include "Classes/Swapchain/Swapchain.hpp"
#include <vulkan/vulkan.hpp>

namespace nihil::graphics
{
    enum class RenderPassAttachmentType
    {
        ColorAttachment,
        DepthAttachment
    };

    //in the future create a specialized struct
    struct RenderPassAttachment
    {
        RenderPassAttachmentType type;

        //sample count
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
        std::vector<RenderPassAttachment> attachments;
        std::vector<vk::SubpassDescription> subpasses;
        std::vector<vk::AttachmentDescription> colorAttachmentDescriptors;
        std::vector<vk::AttachmentDescription> depthAttachmentDescriptors;

        Swapchain* swapchain = nullptr;
        vk::Device device;
        vk::RenderPass renderPass;

    public:
        inline vk::RenderPass _renderPass() const { return renderPass; };

        RenderPass(std::vector<RenderPassAttachment>& _attachments, Swapchain* _swapchain, vk::Device _device);

        ~RenderPass();
    };
}