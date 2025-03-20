#include <iostream>

#include "Classes/App/App.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Swapchain/Swapchain.hpp"
#include "Classes/RenderPass/RenderPass.hpp"
#include "Classes/Pipeline/Pipeline.hpp"
#include "Classes/Shader/Shader.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Camera/Camera.hpp"
#include "Classes/Model/Model.hpp"
#include "Classes/Object/Object.hpp"
#include "Classes/Scene/Scene.hpp"
#include "Classes/Keyboard/Keyboard.hpp"

#include "Logger.hpp"

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//!
#include "OBJLoader.hpp"
#include "Semaphores.hpp"



int main()
{
    nihil::App app("nihil based app", 800, 800);

    nihil::graphics::EngineArgs eArgs = {};
    eArgs.appVersion.major = 0;
    eArgs.appVersion.minor = 0;
    eArgs.appVersion.patch = 0;

    eArgs.appName = "nihil based application";

    eArgs.validationLayers = true;

    eArgs.dPrefs.prefferDedicated = true;
    eArgs.dPrefs.prefferLargerVRAM = true;

    nihil::graphics::Engine engine(&app, eArgs);

    nihil::graphics::Swapchain swapchain(&app, vk::PresentModeKHR::eFifo, (uint8_t)3, &engine);

    std::vector<nihil::graphics::RenderPassAttachment> renderPassAttachments = {
        nihil::graphics::RenderPassAttachment(
            nihil::graphics::RenderPassAttachmentType::ColorAttachment,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR,
            0
        ),
        nihil::graphics::RenderPassAttachment(
            nihil::graphics::RenderPassAttachmentType::DepthAttachment,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal,
            1
        )
    };
    nihil::graphics::RenderPass renderPass(renderPassAttachments, &swapchain, engine._device());

    swapchain.create(std::make_pair(engine._renderQueueIndex(), engine._presentQueueIndex()), &renderPass);

    nihil::Logger::Log("Created the Swapchain.");

    engine.setSwapchain(&swapchain);

    engine.createRenderer();

    nihil::graphics::Shader basicVertexShader(&engine);
    nihil::graphics::Shader basicFragmentShader(&engine);

    basicVertexShader.LoadFromBinary("./Resources/Shaders/basic.vert.spv");
    basicFragmentShader.LoadFromBinary("./Resources/Shaders/basic.frag.spv");

    nihil::graphics::Shader instancedVertexShader(&engine);
    nihil::graphics::Shader instancedFragmentShader(&engine);

    instancedVertexShader.LoadFromBinary("./Resources/Shaders/instanced.vert.spv");
    instancedFragmentShader.LoadFromBinary("./Resources/Shaders/instanced.frag.spv");

    uint32_t vertexSize1 = sizeof(float) * 8;

    vk::VertexInputBindingDescription binding11 = {0, vertexSize1, vk::VertexInputRate::eVertex};

    vk::VertexInputAttributeDescription attribute11 = {0, 0, vk::Format::eR32G32B32Sfloat, 0};
    vk::VertexInputAttributeDescription attribute12 = {1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)};
    vk::VertexInputAttributeDescription attribute13 = {2, 0, vk::Format::eR32G32B32Sfloat, 5 * sizeof(float)};

    std::vector<vk::VertexInputBindingDescription> bindingDesc1 = {binding11};
    std::vector<vk::VertexInputAttributeDescription> attributeDesc1 = {attribute11, attribute12, attribute13};

    nihil::graphics::PipelineCreateInfo pipelineInfo1 = { bindingDesc1, attributeDesc1 };

    pipelineInfo1.vertexShader = basicVertexShader._ptr();
    pipelineInfo1.fragmentShader = basicFragmentShader._ptr();

    //to visualize the mesh
    pipelineInfo1.cullingMode = vk::CullModeFlagBits::eBack;
    pipelineInfo1.polygonMode = vk::PolygonMode::eFill;
    pipelineInfo1.frontFace = vk::FrontFace::eCounterClockwise;

    nihil::graphics::Pipeline basicPipeline(&engine);

    basicPipeline.create(pipelineInfo1, &renderPass);


    uint32_t vertexSize21 = sizeof(float) * 8;
    uint32_t vertexSize22 = sizeof(float) * 16;

    vk::VertexInputBindingDescription binding21 = {0, vertexSize21, vk::VertexInputRate::eVertex};
    vk::VertexInputBindingDescription binding22 = {1, vertexSize22, vk::VertexInputRate::eInstance};

    vk::VertexInputAttributeDescription attribute21 = {0, 0, vk::Format::eR32G32B32Sfloat, 0};
    vk::VertexInputAttributeDescription attribute22 = {1, 0, vk::Format::eR32G32Sfloat, 3 * sizeof(float)};
    vk::VertexInputAttributeDescription attribute23 = {2, 0, vk::Format::eR32G32B32Sfloat, 5 * sizeof(float)};

    vk::VertexInputAttributeDescription attribute24 = {4, 1, vk::Format::eR32G32B32A32Sfloat, 0};
    vk::VertexInputAttributeDescription attribute25 = {5, 1, vk::Format::eR32G32B32A32Sfloat, 4 * sizeof(float)};
    vk::VertexInputAttributeDescription attribute26 = {6, 1, vk::Format::eR32G32B32A32Sfloat, 8 * sizeof(float)};
    vk::VertexInputAttributeDescription attribute27 = {7, 1, vk::Format::eR32G32B32A32Sfloat, 12 * sizeof(float)};

    std::vector<vk::VertexInputBindingDescription> bindingDesc2 = {binding21, binding22};
    std::vector<vk::VertexInputAttributeDescription> attributeDesc2 = {
        attribute21, attribute22, attribute23, attribute24, attribute25, attribute26, attribute27
    };

    nihil::graphics::PipelineCreateInfo pipelineInfo2 = { bindingDesc2, attributeDesc2 };

    pipelineInfo2.vertexShader = instancedVertexShader._ptr();
    pipelineInfo2.fragmentShader = instancedFragmentShader._ptr();

    //to visualize the mesh
    pipelineInfo2.cullingMode = vk::CullModeFlagBits::eBack;
    pipelineInfo2.polygonMode = vk::PolygonMode::eFill;
    pipelineInfo2.frontFace = vk::FrontFace::eCounterClockwise;

    nihil::graphics::Pipeline instancedPipeline(&engine);

    instancedPipeline.create(pipelineInfo2, &renderPass);


    nihil::graphics::Model cubeModel("./Resources/Models/cube.obj", &engine, &basicPipeline, &instancedPipeline, &renderPass);
    nihil::graphics::Model pyramidModel("./Resources/Models/pyramid.obj", &engine, &basicPipeline, &instancedPipeline, &renderPass);
    
    std::vector<nihil::graphics::Object> objects =
    {
        
    };

    int z = 0;
    for (int i = 0; i < 100; i++)
    {
        for (int j = 0; j < 100; j++)
        {
            size_t index = objects.size();
            objects.push_back(nihil::graphics::Object(&cubeModel, &engine));
            objects[index].move(glm::vec3(2.0f * i, 0.0f, 2.0f * j));
            z++;
        }
    }

    nihil::graphics::Scene scene(&engine);

    //* Add objects here
    for (nihil::graphics::Object& o : objects) scene.addObject(&o);

    //moves all of the models onto the GPU
    scene.use();

    nihil::graphics::CameraCreateInfo cameraInfo = {};
    cameraInfo.app = &app;
    nihil::graphics::Camera camera(cameraInfo);

    camera.setLookAt(glm::vec3(0.0f, 0.0f, 100.0f));

    struct UserPointer 
    {
        nihil::graphics::PushConstants* pushConstants;
        nihil::graphics::Swapchain* swapchain;
        nihil::graphics::Camera* camera;
    };

    UserPointer userData{ nullptr, &swapchain, &camera };

    app.userPointer = reinterpret_cast<void*>(&userData);

    app.onResize = [](nihil::App* app, void* userPointer) {
        UserPointer userData = *reinterpret_cast<UserPointer*>(userPointer);

        //? Now event driven
        ////userData.camera->onResize();
        ////userData.swapchain->recreate();

        //this is user defined behavoiur
    };

    nihil::Keyboard keyboard(&app);

    while(!app.shouldExit)
    {
        app.handle();

        if (keyboard.getKey(nihil::Key::W)) camera.move(glm::vec3(0.0f, 0.0f, 1.0f));
        if (keyboard.getKey(nihil::Key::S)) camera.move(glm::vec3(0.0f, 0.0f, -1.0f));
        if (keyboard.getKey(nihil::Key::A)) camera.move(glm::vec3(-0.5f, 0.0f, 0.0f));
        if (keyboard.getKey(nihil::Key::D)) camera.move(glm::vec3(0.5f, 0.0f, 0.0f));
        if (keyboard.getKey(nihil::Key::LShift)) camera.move(glm::vec3(0.0f, 0.5f, 0.0f));
        if (keyboard.getKey(nihil::Key::Space)) camera.move(glm::vec3(0.0f, -0.5f, 0.0f));

        if (keyboard.getKey(nihil::Key::ArrowLeft)) camera.rotate(0.1f, 0.0f, 0.1f);
        if (keyboard.getKey(nihil::Key::ArrowRight)) camera.rotate(-0.1f, 0.0f, 0.1f);
        if (keyboard.getKey(nihil::Key::ArrowUp)) camera.rotate(0.0f, 0.1f, 0.1f);
        if (keyboard.getKey(nihil::Key::ArrowDown)) camera.rotate(0.0f, -0.1f, 0.1f);

        engine._renderer()->Render(&basicPipeline, &renderPass, &scene, &camera);
    }

    return 0;
}