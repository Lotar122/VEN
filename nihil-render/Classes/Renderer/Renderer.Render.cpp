#include "Renderer.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Model/Model.hpp"

namespace nihil::graphics {
	void Renderer::Draw(Camera& camera)
	{
		if (lastFrameTime == std::chrono::steady_clock::time_point()) lastFrameTime = std::chrono::high_resolution_clock::now();
		auto frameStartTime = std::chrono::high_resolution_clock::now();
		auto elapsedTime = frameStartTime - lastFrameTime;
		engine->logicalDevice.waitForFences(1, &swapchainBundle.frames[frameNumber].inFlightFence, VK_TRUE, UINT64_MAX);
		uint32_t imageIndex{ 0 };

		try {
			vk::ResultValue acquire = engine->logicalDevice.acquireNextImageKHR(swapchainBundle.swapchain, UINT64_MAX, swapchainBundle.frames[frameNumber].imageAvailable, nullptr);
			imageIndex = acquire.value;
		}
		catch (vk::OutOfDateKHRError err) {
			RecreateSwapchain();
			return;
		}

		vk::CommandBuffer commandBuffer = swapchainBundle.frames[imageIndex].commandBuffer;

		commandBuffer.reset();

		executeCommandQueue(&engine->commandQueue, commandBuffer, imageIndex, camera);

		vk::SubmitInfo submitinfo = {};
		vk::Semaphore waitSemaphores[] = { swapchainBundle.frames[frameNumber].imageAvailable };
		vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitinfo.waitSemaphoreCount = 1;
		submitinfo.pWaitSemaphores = waitSemaphores;
		submitinfo.pWaitDstStageMask = waitStages;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &swapchainBundle.frames[frameNumber].commandBuffer;
		vk::Semaphore signalSemaphores[] = { swapchainBundle.frames[frameNumber].renderFinished };
		submitinfo.signalSemaphoreCount = 1;
		submitinfo.pSignalSemaphores = signalSemaphores;

		engine->logicalDevice.resetFences(1, &swapchainBundle.frames[frameNumber].inFlightFence);
		try {
			graphicsQueue.submit(submitinfo, swapchainBundle.frames[frameNumber].inFlightFence);
		}
		catch (vk::SystemError err) {
			throw std::exception(err.what());
		}
		vk::PresentInfoKHR presentInfo = {};
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		vk::SwapchainKHR swapchains[] = { swapchainBundle.swapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		vk::Result presentResult;
		try {
			presentResult = presentQueue.presentKHR(presentInfo);
		}
		catch (vk::OutOfDateKHRError err) {
			presentResult = vk::Result::eErrorOutOfDateKHR;
		}
		if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
		{
			RecreateSwapchain();
			return;
		}

		frameNumber = (frameNumber + 1) % maxFramesInFlight;

		// Measure frame end time and calculate the time to sleep
		auto frameEndTime = std::chrono::high_resolution_clock::now();
		auto frameTime = frameEndTime - frameStartTime;

		if (frameTime < engine->frameDuration) {
			//std::this_thread::sleep_for(engine->frameDuration - frameTime);
			std::cout << engine->frameDuration - frameTime << std::endl;
		}

		lastFrameTime = frameStartTime;
	}
	uint64_t count;
	void Renderer::executeCommandQueue(std::vector<DrawCommand>* commandQueue, vk::CommandBuffer& commandBuffer, uint32_t imageIndex, Camera& camera)
	{
		vk::CommandBufferBeginInfo beginInfo = {};
		try {
			commandBuffer.begin(beginInfo);
		}
		catch (vk::SystemError err) {
			throw std::exception(err.what());
		}

		for (DrawCommand& command : *commandQueue)
		{
			vk::RenderPassBeginInfo passInfo = {};
			passInfo.renderPass = engine->getPipeline(command.targetPipeline)->renderPass;
			passInfo.framebuffer = swapchainBundle.frames[imageIndex].frameBuffer;
			passInfo.renderArea.offset.x = 0;
			passInfo.renderArea.offset.y = 0;
			passInfo.renderArea.extent = swapchainBundle.extent;
			vk::ClearValue clearValues[2];
			clearValues[0].color = { 0.0f,0.0f,0.0f,1.0f };
			clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0.0f);
			passInfo.clearValueCount = 2;
			passInfo.pClearValues = clearValues;

			commandBuffer.beginRenderPass(passInfo, vk::SubpassContents::eInline);

			if (command.commandType == CommandType::DrawInstanced)
			{
				InstancedDrawData data = *((InstancedDrawData*)command.data);
				vk::Pipeline pipeline = engine->getPipeline(command.targetPipeline)->pipeline;
				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

				if (*engine->app->get->width != viewport.width || *engine->app->get->height != viewport.height)
				{
					viewport = vk::Viewport(
						0.0f, 0.0f,
						static_cast<float>(*engine->app->get->width), static_cast<float>(*engine->app->get->height),
						0.0f, 1.0f
					);

					scissor = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(*engine->app->get->width, *engine->app->get->height));
				}

				commandBuffer.setViewport(0, 1, &viewport);
				commandBuffer.setScissor(0, 1, &scissor);

				drawInstanced(data.model, commandBuffer, data.instanceBuffer, engine->getPipeline(command.targetPipeline), &camera);
			}
			if (command.commandType == CommandType::DrawBuffer)
			{
				BufferDrawData data = *((BufferDrawData*)command.data);
				vk::Pipeline pipeline = engine->getPipeline(command.targetPipeline)->pipeline;
				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

				if (*engine->app->get->width != viewport.width || *engine->app->get->height != viewport.height)
				{
					viewport = vk::Viewport(
						0.0f, 0.0f,
						static_cast<float>(*engine->app->get->width), static_cast<float>(*engine->app->get->height),
						0.0f, 1.0f
					);

					scissor = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(*engine->app->get->width, *engine->app->get->height));
				}

				commandBuffer.setViewport(0, 1, &viewport);
				commandBuffer.setScissor(0, 1, &scissor);

				drawBuffer(data.vertexBuffer, data.indexBuffer, commandBuffer, engine->getPipeline(command.targetPipeline), &camera);
			}
			if (command.commandType == CommandType::DrawModel)
			{
				ModelDrawData data = *((ModelDrawData*)command.data);
				vk::Pipeline pipeline = engine->getPipeline(command.targetPipeline)->pipeline;
				commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

				if (*engine->app->get->width != viewport.width || *engine->app->get->height != viewport.height)
				{
					viewport = vk::Viewport(
						0.0f, 0.0f,
						static_cast<float>(*engine->app->get->width), static_cast<float>(*engine->app->get->height),
						0.0f, 1.0f
					);

					scissor = vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(*engine->app->get->width, *engine->app->get->height));
				}

				commandBuffer.setViewport(0, 1, &viewport);
				commandBuffer.setScissor(0, 1, &scissor);

				drawModel(data.model, commandBuffer, engine->getPipeline(command.targetPipeline), &camera);
			}
		}

		commandBuffer.endRenderPass();
		try {
			commandBuffer.end();
		}
		catch (vk::SystemError err) {
			throw std::exception(err.what());
		}

		engine->commandQueue.clear();
	}

	void Renderer::drawBuffer(
		Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* _vertexBuffer, 
		Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* _indexBuffer, 
		vk::CommandBuffer& _commandBuffer, 
		PipelineBundle* pBundle,
		Camera* camera,
		Model* model
	)
	{
		vk::Buffer vertexBuffers[] = {
			_vertexBuffer->buffer.buffer
		};
		vk::DeviceSize offsets[] = {
			0
		};

		_commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		_commandBuffer.bindIndexBuffer(_indexBuffer->buffer.buffer, 0, vk::IndexType::eUint32);

		count++;

		ObjectData data = {};

		data.proj = camera->getMatrix();
		data.pre = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		data.pre = glm::rotate(data.pre, glm::radians(0.1f * count), glm::vec3(0.0f, 1.0f, 0.0f));
		data.pre = glm::rotate(data.pre, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		//remove later, only to make thing smaller
		data.pre = glm::scale(data.pre, glm::vec3(0.05f, 0.05f, 0.05f));

		if (model != NULL)
		{
			data.pre = data.pre * model->deafultTransform;
		}
		else
		{
			data.pre = data.pre;
		}

		vk::PipelineLayout layout = pBundle->layout;
		_commandBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ObjectData), &data);

		_commandBuffer.drawIndexed(static_cast<uint32_t>(_indexBuffer->Data.size()), 1, 0, 0, 0);
	}

	void Renderer::drawInstanced(
		Model* model, 
		vk::CommandBuffer& _commandBuffer, 
		Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer,
		PipelineBundle* pBundle,
		Camera* camera
	)
	{
		std::array<vk::Buffer, 2> vertexBuffers = {
			model->vBuffer->buffer.buffer,
			instanceBuffer->buffer.buffer
		};
		vk::DeviceSize offsets[] = {
			0,
			0
		};

		_commandBuffer.bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets);
		_commandBuffer.bindIndexBuffer(model->iBuffer->buffer.buffer, 0, vk::IndexType::eUint32);

		count++;

		ObjectData data = {};

		std::cout << swapchainBundle.extent.width << " " << swapchainBundle.extent.height << " " << (float)swapchainBundle.extent.width / (float)swapchainBundle.extent.height << std::endl;

		data.proj = camera->getMatrix();
		data.proj[1][1] *= -1.0f;
		data.view = glm::lookAtRH(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		data.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, -1.0f));
		data.model = glm::rotate(data.model, glm::radians(0.1f * count), glm::vec3(1.0f, 1.0f, 0.0f));
		data.model = glm::rotate(data.model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		//remove later, only to make things smaller
		data.model = glm::scale(data.model, glm::vec3(0.05f, 0.05f, 0.05f));

		data.pre = data.pre * model->deafultTransform;

		vk::PipelineLayout layout = pBundle->layout;
		_commandBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ObjectData), &data);

		std::cout << std::floor(instanceBuffer->Data.size() / 16) << " floored shit " << instanceBuffer->Data.size() << std::endl;

		_commandBuffer.drawIndexed(static_cast<uint32_t>(model->iBuffer->Data.size()), std::floor(instanceBuffer->Data.size() / 16), 0, 0, 0);
	}
	void Renderer::drawModel(
		Model* model,
		vk::CommandBuffer& _commandBuffer,
		PipelineBundle* pBundle,
		Camera* camera
	)
	{
		vk::Buffer vertexBuffers[] = {
			model->vBuffer->buffer.buffer
		};
		vk::DeviceSize offsets[] = {
			0
		};

		_commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		_commandBuffer.bindIndexBuffer(model->iBuffer->buffer.buffer, 0, vk::IndexType::eUint32);

		count++;

		ObjectData data = {};

		data.proj = camera->getMatrix();
		data.pre = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		data.pre = glm::rotate(data.pre, glm::radians(0.1f * count), glm::vec3(0.0f, 1.0f, 0.0f));
		data.pre = glm::rotate(data.pre, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		//remove later, only to make thing smaller
		data.pre = glm::scale(data.pre, glm::vec3(0.05f, 0.05f, 0.05f));

		if (model != NULL)
		{
			data.pre = data.pre * model->deafultTransform;
		}
		else
		{
			data.pre = data.pre;
		}

		vk::PipelineLayout layout = pBundle->layout;
		_commandBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(ObjectData), &data);

		_commandBuffer.drawIndexed(static_cast<uint32_t>(model->iBuffer->Data.size()), 1, 0, 0, 0);
	}
}