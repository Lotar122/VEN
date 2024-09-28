#include "nihil-render/nihil.hpp"
#include "nihil-render/nstd/nstd.hpp"

#include "Classes/Nihil/Nihil.hpp"

#include "windows.h"
#define _CRTDBG_MAP_ALLOC //to get more details
#include <stdlib.h>  
#include <crtdbg.h>   //for malloc and free

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

	//std::thread render = std::thread([app, engine, &modelCarP, &modelCubeP](nihil::App* app, nihil::graphics::Engine* engine) {
		engine->Setup();

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
		nihil::graphics::PipelineBundle pipelineBundle = engine->CreatePipeline(pipelineInfo);

		engine->registerPipeline(pipelineBundle);

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
		nihil::graphics::Model* modelCube = new nihil::graphics::Model(engine, "./resources/models/lambo.obj");

		modelCarP = modelCar;
		modelCubeP = modelCube;

		modelCar->setDeafultPipeline(0);
		modelCar->setInstancedPipeline(0);

		modelCube->setDeafultPipeline(0);
		modelCube->setInstancedPipeline(0);

		std::vector<nihil::nstd::Component> compArr;

		//pipeline assignment
		//engine->renderer->pipeline = pipeline;

		nihil::engine::Object obj1;
		obj1.setPosition(glm::vec3(2.0f, 0.0f, 1.0f));
		obj1.model = modelCarP;

		nihil::engine::Object obj2;
		obj2.setPosition(glm::vec3(0.0f, 0.0f, 2.0f));
		obj2.model = modelCarP;

		nihil::graphics::Camera camera = {};


		nihil::Nihil nihilO(engine);

		uint64_t deleteDebug = 0;
		while (!*(app->get->shouldClose))
		{
			//camera.setOrthographicProjection(-1, 1, -1, 1, engine);
			camera.setPerspectiveProjection(glm::radians(90.0f), (float)engine->swapchain->extent.width / (float)engine->swapchain->extent.height, 0.1f, 100.0f);

			if (camera.getMatrix() == glm::perspectiveRH(glm::radians(90.0f), (float)engine->swapchain->extent.width / (float)engine->swapchain->extent.height, 0.1f, 100.0f))
			{
				std::cout << "were lucky" << std::endl;
			}

			nihilO.queueDrawObject(&obj2);

			nihilO.queueDrawObject(&obj1);

			nihilO.executeDraws(camera);
		}
	//}, app, engine);

	while (!*(app->get->shouldClose))
	{
		app->handle();
	}

	//render.join();

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