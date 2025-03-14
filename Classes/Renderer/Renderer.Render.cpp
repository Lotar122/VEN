#include "Classes/Renderer/Renderer.hpp"

#include "Classes/Engine/Engine.hpp"
#include "Classes/Swapchain/Swapchain.hpp"
#include "Classes/Pipeline/Pipeline.hpp"
#include "Classes/RenderPass/RenderPass.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Scene/Scene.hpp"

#include <thread>

using namespace nihil::graphics;

void Renderer::Render(
	Pipeline* pipeline, RenderPass* renderPass, 
	Scene* scene,
	Camera* camera
)
{
	uint32_t imageIndex = 0;
	Swapchain::Frame* frameP = engine->swapchain->acquireNextFrame(&imageIndex);
	if(!frameP) return;
	Swapchain::Frame& frame = *frameP;

	frame.commandBuffer.reset();

	vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	frame.commandBuffer.begin(commandBufferBeginInfo);

	//record commands;

	//Dynamic States

	//bind the dynamic states
	frame.commandBuffer.setViewport(0, 1, &viewport);
	frame.commandBuffer.setScissor(0, 1, &scissor);

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo.renderPass = renderPass->_renderPass();    // Your vk::RenderPass object
	renderPassInfo.framebuffer = frame.frameBuffer;  // The framebuffer for this pass
	renderPassInfo.renderArea.offset = vk::Offset2D{0, 0};
	renderPassInfo.renderArea.extent = engine->swapchain->_extent();  // Set to match swapchain size

	// Define clear values (for color and depth attachments)
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].color = vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}}; // Black background
	clearValues[1].depthStencil = vk::ClearDepthStencilValue{1.0f, 0}; // Depth buffer clears to 1.0

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	frame.commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	scene->recordCommands(frame.commandBuffer, camera);

	frame.commandBuffer.endRenderPass();

	frame.commandBuffer.end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &frame.imageAvailable;
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.pWaitDstStageMask = waitStages; // Wait until color attachment is ready

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &frame.commandBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &frame.renderFinished;

	//vk::Result discardResult = engine->_device().resetFences(1, &frame.inFlightFence);
	vk::Result discardResult = engine->_renderQueue().submit(1, &submitInfo, frame.inFlightFence);

	engine->swapchain->presentFrame(frame, imageIndex);
}