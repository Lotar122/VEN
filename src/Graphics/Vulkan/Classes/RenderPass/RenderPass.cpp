#include "RenderPass.hpp"

using namespace nihil::graphics;

RenderPass::RenderPass(std::vector<RenderPassAttachment>& _attachments, Swapchain* _swapchain, vk::Device _device)
{
	assert(_swapchain != nullptr);
	assert(_device != nullptr);

	swapchain = _swapchain;
	device = _device;
	attachments = _attachments;
	for(auto& a : attachments)
	{
		vk::Format format;

		if(a.type == RenderPassAttachmentType::ColorAttachment) format = swapchain->imageFormat;
		else if(a.type == RenderPassAttachmentType::DepthAttachment) format = swapchain->depthFormat;

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

		if(a.type == RenderPassAttachmentType::ColorAttachment) colorAttachmentDescriptors.push_back(attachment);
		else if(a.type == RenderPassAttachmentType::DepthAttachment) depthAttachmentDescriptors.push_back(attachment);
	}

	//manual for now, change later
	vk::AttachmentReference colorAttachmentRef(
		0, 
		vk::ImageLayout::eColorAttachmentOptimal
	);

	vk::AttachmentReference depthAttachmentRef(
		1, 
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);
	vk::AttachmentReference resolveAttachmentRef(
		2,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	vk::SubpassDescription subpass(
		{}, // Flags
		vk::PipelineBindPoint::eGraphics, // Graphics pipeline
		0, nullptr, // Input attachments
		1, &colorAttachmentRef, // Color attachments
		&resolveAttachmentRef, // Resolve attachments
		&depthAttachmentRef // Depth-stencil attachment
	);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL, 0, // From external to subpass 0
		vk::PipelineStageFlagBits::eColorAttachmentOutput, // Before fragment shader writes
		vk::PipelineStageFlagBits::eColorAttachmentOutput, // After writes
		{}, // No memory dependency before
		vk::AccessFlagBits::eColorAttachmentWrite // Ensure color writes are visible
	);

	std::vector<vk::AttachmentDescription> attachments;
	attachments.resize(3);
	attachments[0] = colorAttachmentDescriptors[0];
	attachments[1] = depthAttachmentDescriptors[0];
	attachments[2] = colorAttachmentDescriptors[1];

	vk::RenderPassCreateInfo renderPassInfo(
		{}, // Flags
		3, attachments.data(), // Attachments
		1, &subpass, // Subpasses
		1, &dependency // Dependencies
	);

	renderPass.assignRes(device.createRenderPass(renderPassInfo), device);
}

RenderPass::~RenderPass()
{
	renderPass.destroy();
}