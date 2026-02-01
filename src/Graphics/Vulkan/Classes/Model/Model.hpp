#pragma once

#include <new>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include "Legacy/OBJLoader.hpp"

#include "Classes/Buffer/Buffer.hpp"

#include "Classes/Asset/Asset.hpp"

//TODO: remove all the pipeline and renderpass stuff

namespace nihil::graphics
{
    class Engine;

    using byte = unsigned char;

    class Model : public Asset
    {
        Engine* engine = nullptr;
        Pipeline* pipeline = nullptr;
        Pipeline* instancedPipeline = nullptr;
        RenderPass* renderPass = nullptr;

        glm::mat4 model;
        std::string path;
        
        Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* vertexBuffer = nullptr;
        Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* indexBuffer = nullptr;

        alignas(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>) byte vertexBufferMemory[sizeof(Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>)];
        alignas(Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>) byte indexBufferMemory[sizeof(Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>)];
    public:
        Model(
            const std::string& _path, Engine* _engine, Pipeline* _pipeline = nullptr, 
            Pipeline* _instancedPipeline = nullptr, RenderPass* _renderPass = nullptr, 
            glm::mat4 _model = glm::mat4(1.0f)
        ) : Asset(AssetUsage::Undefined, _engine)
        {
            path = _path;
            model = _model;
            pipeline = _pipeline;
            renderPass = _renderPass;
            instancedPipeline = _instancedPipeline;

            assert(_engine != nullptr);

            engine = _engine;

            auto loadingResult = readOBJFile(path);

            vertexBuffer = new (vertexBufferMemory) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(loadingResult.first, engine);
            indexBuffer = new (indexBufferMemory) Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>(loadingResult.second, engine);
        }

        template<auto usageT1, auto propertiesT1, auto usageT2, auto propertiesT2>
        Model(
            Buffer<float, usageT1, propertiesT1>&& _vertexBuffer, Buffer<uint32_t, usageT2, propertiesT2>&& _indexBuffer,
            Engine* _engine, Pipeline* _pipeline = nullptr, 
            Pipeline* _instancedPipeline = nullptr, RenderPass* _renderPass = nullptr, 
            glm::mat4 _model = glm::mat4(1.0f)
        ) : Asset(AssetUsage::Undefined, _engine)
        {
            path = "";
            model = _model;
            pipeline = _pipeline;
            renderPass = _renderPass;
            instancedPipeline = _instancedPipeline;

            assert(_engine != nullptr);

            engine = _engine;

            vertexBuffer = new (vertexBufferMemory) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(_vertexBuffer);
            indexBuffer = new (indexBufferMemory) Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>(_indexBuffer);
        }

        template<auto usageT1, auto propertiesT1, auto usageT2, auto propertiesT2>
        Model(
            Buffer<float, usageT1, propertiesT1>& _vertexBuffer, Buffer<uint32_t, usageT2, propertiesT2>& _indexBuffer,
            Engine* _engine, Pipeline* _pipeline = nullptr, 
            Pipeline* _instancedPipeline = nullptr, RenderPass* _renderPass = nullptr, 
            glm::mat4 _model = glm::mat4(1.0f)
        ) : Asset(AssetUsage::Undefined, _engine)
        {
            path = "";
            model = _model;
            pipeline = _pipeline;
            renderPass = _renderPass;
            instancedPipeline = _instancedPipeline;

            assert(_engine != nullptr);

            engine = _engine;

            vertexBuffer = new (vertexBufferMemory) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(_vertexBuffer.data, &engine);
            indexBuffer = new (indexBufferMemory) Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>(_indexBuffer.data, &engine);
        }

        inline Pipeline* _pipeline() { return pipeline; };
        inline Pipeline* _instancedPipeline() { return instancedPipeline; };
        inline RenderPass* _renderPass() { return renderPass; };
        inline Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>& _vertexBuffer() { return *vertexBuffer; };
        inline Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>& _indexBuffer() { return *indexBuffer; };

        inline Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* _vertexBufferPtr() { return vertexBuffer; };
        inline Buffer<uint32_t, vk::BufferUsageFlagBits::eIndexBuffer>* _indexBufferPtr() { return indexBuffer; };

        inline void setPipeline(Pipeline* _pipeline)
        {
            pipeline = _pipeline;
        }

        inline void setInstancePipeline(Pipeline* _pipeline)
        {
            instancedPipeline = _pipeline;
        }

        inline void setRenderPass(RenderPass* _renderPass)
        {
            renderPass = _renderPass;
        }

        inline void moveToGPU() 
        {
            vertexBuffer->moveToGPU();
            indexBuffer->moveToGPU();
        }

        inline void freeFromGPU()
        {
            vertexBuffer->freeFromGPU();
            indexBuffer->freeFromGPU();
        }

        inline glm::mat4& _defaultTransform() { return model; };

        ~Model()
        {
            vertexBuffer->~Buffer();
            indexBuffer->~Buffer();
        }
    };
}