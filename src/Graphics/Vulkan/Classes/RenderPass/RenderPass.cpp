#include "RenderPass.hpp"

using namespace nihil::graphics;

RenderPass::RenderPass(std::vector<RenderPassAttachment>& _attachments, Swapchain* _swapchain, vk::Device _device)
{
	assert(_swapchain != nullptr);
	assert(_device != nullptr);

	swapchain = _swapchain;
	device = _device;
	attachments = _attachments;

	std::vector<vk::AttachmentReference> colorAttachmentRefs;
	std::vector<vk::AttachmentReference> depthAttachmentRefs;
	std::vector<vk::AttachmentReference> resolveAttachmentRefs;

	std::vector<vk::AttachmentDescription> attachmentsTransformed;
	attachmentsTransformed.reserve(attachments.size());

	uint64_t attachmentIndex = 0;
	for(auto& a : attachments)
	{
		vk::Format format = vk::Format::eUndefined;

		if (a.type == RenderPassAttachmentType::ColorAttachment) format = swapchain->imageFormat;
		else if (a.type == RenderPassAttachmentType::DepthAttachment) format = swapchain->depthFormat;
		else if (a.type == RenderPassAttachmentType::ResolveAttachment) format = swapchain->imageFormat;

		vk::AttachmentDescription attachment(
			{},
			format,
			a.sampleCount,
			a.loadOp,
			a.storeOp,
			a.stencilLoadOp,
			a.stencilStoreOp,
			a.initialImageLayout,
			a.finalImageLayout
		);

		if (a.type == RenderPassAttachmentType::ColorAttachment)
		{
			vk::AttachmentReference attachmentRef(
				attachmentIndex,
				vk::ImageLayout::eColorAttachmentOptimal
			);
			colorAttachmentRefs.push_back(attachmentRef);
		}
		else if (a.type == RenderPassAttachmentType::DepthAttachment)
		{
			vk::AttachmentReference attachmentRef(
				attachmentIndex,
				vk::ImageLayout::eDepthStencilAttachmentOptimal
			);
			depthAttachmentRefs.push_back(attachmentRef);
		}
		else if (a.type == RenderPassAttachmentType::ResolveAttachment)
		{
			vk::AttachmentReference attachmentRef(
				attachmentIndex,
				vk::ImageLayout::eColorAttachmentOptimal
			);
			resolveAttachmentRefs.push_back(attachmentRef);
		}

		attachmentsTransformed.push_back(attachment);
		attachmentIndex++;
	}

	vk::SubpassDescription subpass(
		{}, // Flags
		vk::PipelineBindPoint::eGraphics, // Graphics pipeline
		0, nullptr, // Input attachments
		colorAttachmentRefs.size(), colorAttachmentRefs.data(), // Color attachments
		resolveAttachmentRefs.data(), // Resolve attachments
		depthAttachmentRefs.data() // Depth-stencil attachments
	);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL, 0, // From external to subpass 0
		vk::PipelineStageFlagBits::eColorAttachmentOutput, // Before fragment shader writes
		vk::PipelineStageFlagBits::eColorAttachmentOutput, // After writes
		{}, // No memory dependency before
		vk::AccessFlagBits::eColorAttachmentWrite // Ensure color writes are visible
	);

	vk::RenderPassCreateInfo renderPassInfo(
		{}, // Flags
		attachmentsTransformed.size(), attachmentsTransformed.data(), // Attachments
		1, &subpass, // Subpasses
		1, &dependency // Dependencies
	);

	renderPass.assignRes(device.createRenderPass(renderPassInfo), device);
}

RenderPass::~RenderPass()
{
	renderPass.destroy();
}