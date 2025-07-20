#pragma once

#ifdef DISABLEMATERIAL

#include <vector>

#include "Classes/Engine/Engine.hpp"
#include "Classes/Shader/Shader.hpp"

#include "Classes/Asset/Asset.hpp"

//this should include the texture and Shaders
namespace nihil::graphics
{
    class Material : public Asset
    {
        Engine* engine = nullptr;
    public:
        Material(Engine* _engine) : Asset(_engine)
        {
            assert(_engine != nullptr);

            engine = _engine;

            sampleCount = engine->_swapchain()->_sampleCount();

            //FOR NOW DEAFULT THE PIPELINE CREATION;
            //!Make this customizable later on


        }

        //Texture texture

        Shader* vertexShader;
        Shader* fragmentShader;

        Shader* instancedVertexShader;

        Pipeline* pipeline;
        Pipeline* instancedPipeline;

        Shader shaderMemory[3];
        Pipeline pipelineMemory[2];

        std::vector<vk::VertexInputAttributeDescription> attributeDescriptors;
        std::vector<vk::VertexInputBindingDescription> bindingDescriptors;

        vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
        vk::CullModeFlags cullingMode = vk::CullModeFlagBits::eBack;
        vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;

        vk::SampleCountFlagBits sampleCount;
    };
}

#endif