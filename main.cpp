#include "nihil-render/nihil.hpp"
#include "nihil-render/nstd/nstd.hpp"

#include "Classes/Nihil/Nihil.hpp"

#include "windows.h"
#define _CRTDBG_MAP_ALLOC //to get more details
#include <stdlib.h>  
#include <crtdbg.h>   //for malloc and free

#include <windows.h>
#include <psapi.h>

void logMemoryUsage(const std::string& filename, uint64_t& elapsedTime, const bool* shouldClose) {
	// Open the file in truncation mode to empty it and write new data
	std::ofstream logFile(filename, std::ios::out | std::ios::trunc);
	if (!logFile.is_open()) {
		std::cerr << "Error opening file for logging." << std::endl;
		return;
	}

	// Write CSV header
	logFile << "Time (seconds),Memory Usage (MB)\n";

	while (!*(shouldClose)) {
		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
			SIZE_T memoryUsage = pmc.WorkingSetSize; // Get memory usage in bytes
			double memoryInMB = memoryUsage / (1024.0 * 1024.0); // Convert to MB

			// Log elapsed time and memory usage to CSV
			logFile << elapsedTime << "," << static_cast<int>(memoryInMB) << "\n";

			logFile.flush(); // Flush to ensure data is written

			elapsedTime++; // Increment elapsed time
			std::cout << elapsedTime << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::seconds(10)); // Log every 10 seconds
	}

	logFile.close();
}

int main()
{
	_CrtMemState sOld;
	_CrtMemState sNew;
	_CrtMemState sDiff;
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtMemCheckpoint(&sOld); //take a snapshot

	nihil::graphics::Version vulkanVersion = {};
	vulkanVersion.make_version(0, 1, 0, 0);

	nihil::graphics::Version appVersion = {};
	appVersion.make_version(0, 1, 0, 0);

	nihil::graphics::Engine* engine = new nihil::graphics::Engine(true);

	nihil::AppCreationArgs* appArgs = new nihil::AppCreationArgs();
	appArgs->appVersion = appVersion;
	appArgs->vulkanVersion = vulkanVersion;
	appArgs->engine = engine;

	appArgs->name = "vis ex nihil";
	appArgs->width = 1280;
	appArgs->height = 720;

	nihil::App* app = new nihil::App(appArgs);

	nihil::graphics::Model* modelCarP = NULL;
	nihil::graphics::Model* modelCubeP = NULL;

	std::thread render = std::thread([app, engine, &modelCarP, &modelCubeP](nihil::App* app, nihil::graphics::Engine* engine) {
		engine->Setup(true);

		std::vector<nihil::graphics::VertexAttribute> vAttrib(8);
		vAttrib[0].binding = 0;
		vAttrib[0].location = 0;
		vAttrib[0].format = vk::Format::eR32G32B32Sfloat;
		vAttrib[0].offset = 0;

		vAttrib[1].binding = 0;
		vAttrib[1].location = 1;
		vAttrib[1].format = vk::Format::eR32G32B32Sfloat;
		vAttrib[1].offset = 3 * sizeof(float);

		vAttrib[2].binding = 0;
		vAttrib[2].location = 2;
		vAttrib[2].format = vk::Format::eR32G32Sfloat;
		vAttrib[2].offset = 6 * sizeof(float);

		vAttrib[3].binding = 0;
		vAttrib[3].location = 3;
		vAttrib[3].format = vk::Format::eR32G32B32Sfloat;
		vAttrib[3].offset = 8 * sizeof(float);

		//matrix 4x4 (instancedata)
		vAttrib[4].binding = 1;
		vAttrib[4].location = 5;
		vAttrib[4].format = vk::Format::eR32G32B32A32Sfloat;
		vAttrib[4].offset = 0;

		vAttrib[5].binding = 1;
		vAttrib[5].location = 6;
		vAttrib[5].format = vk::Format::eR32G32B32A32Sfloat;
		vAttrib[5].offset = 4 * sizeof(float);

		vAttrib[6].binding = 1;
		vAttrib[6].location = 7;
		vAttrib[6].format = vk::Format::eR32G32B32A32Sfloat;
		vAttrib[6].offset = 8 * sizeof(float);

		vAttrib[7].binding = 1;
		vAttrib[7].location = 8;
		vAttrib[7].format = vk::Format::eR32G32B32A32Sfloat;
		vAttrib[7].offset = 12 * sizeof(float);

		std::vector<nihil::graphics::VertexBindingInformation> bindingInfo = {};
		nihil::graphics::VertexBindingInformation binding1 = {};
		nihil::graphics::VertexBindingInformation binding2 = {};

		binding1.inputRate = vk::VertexInputRate::eVertex;
		binding1.stride = 0;
		binding2.inputRate = vk::VertexInputRate::eInstance;
		binding2.stride = 0;

		bindingInfo.push_back(binding1);
		bindingInfo.push_back(binding2);

		vk::ShaderModule* vertexShader = NULL;
		vk::ShaderModule* fragmentShader = NULL;
		engine->renderer->CreateShaderModule("./resources/Shaders/shaderV.spv", *engine->get->logicalDevice, &vertexShader);
		engine->renderer->CreateShaderModule("./resources/Shaders/shaderF.spv", *engine->get->logicalDevice, &fragmentShader);

		nihil::graphics::PipelineInfo pipelineInfo = engine->CreatePipelineConfiguration(vAttrib, bindingInfo, vertexShader, fragmentShader);
		
		nihil::graphics::RenderPassInfo basicPassInfo = {};
		nihil::graphics::Attachment basicColorAttachment = {};
		basicColorAttachment.type = nihil::graphics::AttachmentType::Color;
		basicColorAttachment.sampleCount = vk::SampleCountFlagBits::e1;
		basicColorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
		basicColorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		basicColorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		basicColorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		basicColorAttachment.initialLayout = vk::ImageLayout::ePresentSrcKHR;
		basicColorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		basicPassInfo.attachments.push_back(basicColorAttachment);

		nihil::graphics::Attachment basicDepthAttachment = {};
		basicDepthAttachment.type = nihil::graphics::AttachmentType::Depth;
		basicDepthAttachment.sampleCount = vk::SampleCountFlagBits::e1;
		basicDepthAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
		basicDepthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		basicDepthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		basicDepthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		basicDepthAttachment.initialLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		basicDepthAttachment.finalLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		basicPassInfo.attachments.push_back(basicDepthAttachment);

		vk::RenderPass basicPass = engine->CreateRenderPass(basicPassInfo, engine->swapchain);
		
		nihil::graphics::PipelineBundle pipelineBundle = engine->CreatePipeline(pipelineInfo, basicPass);

		uint32_t basicPipeline = engine->registerPipeline(pipelineBundle);

		glm::mat4 trans = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		std::vector<float> instanceData(0);
		std::vector<float> instanceData2(0);

		for (int i = 0; i < 4; i++)
		{
			glm::mat4 y = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f * i, 0.0f, 0.0f));
			std::vector<float> x = nihil::nstd::flattenMatrix4x4(y);

			for (float& f : x)
			{
				instanceData.push_back(f);
			}
		}
		for (int i = 0; i < 2; i++)
		{
			glm::mat4 y = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f * i * 2, 10.0f, 0.0f));
			y = glm::rotate(y, glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			std::vector<float> x = nihil::nstd::flattenMatrix4x4(y);

			for (float& f : x)
			{
				instanceData2.push_back(f);
			}
		}

		nihil::graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer> instanceBuffer(engine, instanceData);
		nihil::graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer> instanceBuffer2(engine, instanceData2);

		nihil::graphics::Model* modelCar = new nihil::graphics::Model(engine, "./resources/models/sphere.obj");
		nihil::graphics::Model* modelCube = new nihil::graphics::Model(engine, "./resources/models/cube.obj");

		modelCarP = modelCar;
		modelCubeP = modelCube;

		//modelCar->deafultTransform = glm::scale(modelCar->deafultTransform, glm::vec3(0.5f, 0.5f, 0.5f));

		modelCar->setDeafultPipeline(basicPipeline);
		modelCar->setInstancedPipeline(basicPipeline);

		modelCube->setDeafultPipeline(basicPipeline);
		modelCube->setInstancedPipeline(basicPipeline);

		std::vector<nihil::nstd::Component> compArr;

		//pipeline assignment
		//engine->renderer->pipeline = pipeline;

		nihil::engine::Object obj1;
		obj1.setPosition(glm::vec3(0.0f, 0.0f, 20.0f));
		obj1.model = modelCarP;

		nihil::engine::Object obj2;
		obj2.setPosition(glm::vec3(-11.0f, 0.0f, -20.0f));
		obj2.model = modelCubeP;

		nihil::engine::Object obj3;
		obj3.setPosition(glm::vec3(11.0f, 0.0f, -20.0f));
		//it is considered bad practice to manually alter these values
		obj3.renderingData = glm::scale(obj3.renderingData, glm::vec3(0.5f, 0.5f, 0.5f));
		obj3.model = modelCarP;

		nihil::graphics::Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		camera.setPerspectiveProjection(45.0f, 1.0f, 0.1f, 100.0f);

		nihil::Nihil nihilO(engine);

		nihilO.keyboard->registerKey(nihil::Key::ArrowLeft);
		nihilO.keyboard->registerKey(nihil::Key::ArrowRight);
		nihilO.keyboard->registerKey(nihil::Key::ArrowDown);
		nihilO.keyboard->registerKey(nihil::Key::ArrowUp);
		nihilO.keyboard->registerKey(nihil::Key::w);
		nihilO.keyboard->registerKey(nihil::Key::a);
		nihilO.keyboard->registerKey(nihil::Key::s);
		nihilO.keyboard->registerKey(nihil::Key::d);

		while (!*(app->get->shouldClose))
		{
			//camera.setOrthographicProjection(-1, 1, -1, 1, engine);

			//obj1.model->deafultTransform = glm::rotate(glm::mat4(1.0f), glm::radians(0.1f * loopCounter), glm::vec3(0.0f, 1.0f, 0.0f));
			//obj3.rotate(glm::vec3(0.0f, 0.05f, 0.0f));

			//nihilO.queueDrawObject(&obj2);

			if (nihilO.keyboard->checkKey(nihil::Key::ArrowLeft))
			{
				camera.rotate(0.0f, 0.5f);
			}
			else if (nihilO.keyboard->checkKey(nihil::Key::ArrowRight))
			{
				camera.rotate(0.0f, -0.5f);
			}
			else if (nihilO.keyboard->checkKey(nihil::Key::w))
			{
				camera.move(glm::vec3(0.0f, 0.0f, -0.05f));
			}
			else if (nihilO.keyboard->checkKey(nihil::Key::s))
			{
				camera.move(glm::vec3(0.0f, 0.0f, 0.05f));
			}
			else if (nihilO.keyboard->checkKey(nihil::Key::a))
			{
				camera.move(glm::vec3(0.05f, 0.0f, 0.0f));
			}
			else if (nihilO.keyboard->checkKey(nihil::Key::d))
			{
				camera.move(glm::vec3(-0.05f, 0.0f, 0.0f));
			}

			std::cout << "Queue Draw1" << std::endl;
			nihilO.queueDrawObject(&obj2);
			std::cout << "Queue Draw2" << std::endl;
			nihilO.queueDrawObject(&obj3);

			std::cout << "Execute Draws" << std::endl;
			nihilO.executeDraws(camera);
		}
	}, app, engine);

	std::thread logThread([app]() {
		uint64_t timer = 0;
		logMemoryUsage("./nihil-memory.csv", timer, app->get->shouldClose);
		std::cout << "timer: " << timer << std::endl;
	});

	while (!*(app->get->shouldClose))
	{
		app->handle();

		//std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	render.join();
	logThread.join();

	delete modelCarP;
	delete modelCubeP;
	delete engine;
	delete app;

	//i will work on memory leaks in the future. for now i will leave as is, same goes for vulkan resource managment.
	_CrtMemCheckpoint(&sNew); //take a snapshot 
	if (_CrtMemDifference(&sDiff, &sOld, &sNew)) // if there is a difference
	{
		OutputDebugString("-----------_CrtMemDumpStatistics ---------");
		_CrtMemDumpStatistics(&sDiff);
		OutputDebugString("-----------_CrtMemDumpAllObjectsSince ---------");
		_CrtMemDumpAllObjectsSince(&sOld);
		OutputDebugString("-----------_CrtDumpMemoryLeaks ---------");
		_CrtDumpMemoryLeaks();
	}

	return 0;
}