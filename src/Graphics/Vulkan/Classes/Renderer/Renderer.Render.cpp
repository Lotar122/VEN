#include "Classes/Renderer/Renderer.hpp"

#include "Classes/Engine/Engine.hpp"
#include "Classes/Swapchain/Swapchain.hpp"
#include "Classes/Pipeline/Pipeline.hpp"
#include "Classes/RenderPass/RenderPass.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Scene/Scene.hpp"

#include <thread>
#include <chrono>

using namespace nihil::graphics;

void Renderer::Render(
	RenderPass* renderPass, 
	Scene* scene,
	Camera* camera
)
{
	uint32_t imageIndex = 0, frameIndex = 0;
	Swapchain::Frame* frameP = engine->swapchain->acquireNextFrame(&imageIndex, &frameIndex);
	if(!frameP) return;
	Swapchain::Frame& frame = *frameP;

	vk::CommandBuffer& commandBuffer = engine->swapchain->_frames()[frameIndex].commandBuffer;

	commandBuffer.reset();

	vk::CommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	commandBuffer.begin(commandBufferBeginInfo);

	//record commands;

	//Dynamic States

	//bind the dynamic states
	commandBuffer.setViewport(0, 1, &viewport);
	commandBuffer.setScissor(0, 1, &scissor);

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

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    
	//Wait for transfers before starting recording commands
	vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);

	//record commands from the scene
    scene->recordCommands(commandBuffer, camera);

	commandBuffer.endRenderPass();

	commandBuffer.end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = engine->_swapchain()->_frames()[frameIndex].imageAvailable.getResP();
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.pWaitDstStageMask = waitStages; // Wait until color attachment is ready

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = engine->_swapchain()->_frames()[frameIndex].renderFinished.getResP();

	discardResult = engine->_renderQueue().submit(1, &submitInfo, engine->_swapchain()->_frames()[frameIndex].inFlightFence);

	engine->swapchain->presentFrame(frame, imageIndex);
}