#pragma once

#include <vector>

#include "Classes/Engine/Engine.hpp"
#include "Classes/Shader/Shader.hpp"
#include "Classes/Texture/Texture.hpp"
#include "Classes/Sampler/Sampler.hpp"
#include "Classes/Pipeline/Pipeline.hpp"

#include "Classes/Asset/Asset.hpp"

//this should include the texture and Shaders
namespace nihil::graphics
{
    class Material : public Asset
    {
        Engine* engine = nullptr;

        Texture* texture = nullptr;
        Sampler* sampler = nullptr;

        Shader* vertexShader = nullptr;
        Shader* fragmentShader = nullptr;
        Shader* instancedVertexShader = nullptr;
        Shader* instancedFragmentShader = nullptr;

        Pipeline* pipeline = nullptr;
        Pipeline* instancedPipeline = nullptr;

        RenderPass* renderPass = nullptr;
    public:
        Material(
            Texture* _texture, Sampler* _sampler, 
            Shader* _vertexShader, Shader* _fragmentShader, Shader* _instancedVertexShader, Shader* _instancedFragmentShader, 
            Pipeline* _pipeline, Pipeline* _instancedPipeline,
            RenderPass* _renderPass,
            Engine* _engine
        ) : Asset(AssetUsage::Undefined, _engine)
        {
            assert(_engine != nullptr);

            // assert(_texture != nullptr);
            // assert(_sampler != nullptr);

            // assert(_vertexShader != nullptr);
            // assert(_fragmentShader != nullptr);
            // assert(_instancedVertexShader != nullptr);
            // assert(_instancedFragmentShader != nullptr);

            // assert(_pipeline != nullptr);
            // assert(_instancedPipeline != nullptr);

            // assert(_renderPass != nullptr);

            engine = _engine;

            texture = _texture;
            sampler = _sampler;

            vertexShader = _vertexShader;
            fragmentShader = _fragmentShader;
            instancedVertexShader = _instancedVertexShader;
            instancedFragmentShader = _instancedFragmentShader;

            pipeline = _pipeline;
            instancedPipeline = _instancedPipeline;

            renderPass = _renderPass;
        }
    };
}