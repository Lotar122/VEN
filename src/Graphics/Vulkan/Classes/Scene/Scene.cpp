#include "Scene.hpp"

#include "Classes/Camera/Camera.hpp"
#include "Classes/Model/Model.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Pipeline/Pipeline.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace nihil::graphics;

void Scene::addObjects(const Object** newObjects, size_t size)
{
    size_t originalObjectsSize = objects.size();
    objects.resize(objects.size() + size);
    std::memcpy(reinterpret_cast<char*>(objects.data() + originalObjectsSize), reinterpret_cast<char*>(newObjects), size * sizeof(Object**));
}

void Scene::addObjects(Object* newObjects, size_t size)
{
    size_t originalObjectsSize = objects.size();
    objects.resize(objects.size() + size);
    for (size_t i = 0; i < size; i++)
    {
        objects[originalObjectsSize + i] = &newObjects[i];
    }
}

void Scene::addObjects(const std::vector<Object*>& newObjects)
{
    size_t originalObjectsSize = objects.size();
    objects.resize(objects.size() + newObjects.size());
    std::memcpy(reinterpret_cast<char*>(objects.data() + originalObjectsSize), reinterpret_cast<const char*>(newObjects.data()), newObjects.size() * sizeof(Object**));
}

void Scene::addObjects(std::vector<Object>& newObjects)
{
    size_t originalObjectsSize = objects.size();
    objects.resize(objects.size() + newObjects.size());
    for (size_t i = 0; i < newObjects.size(); i++)
    {
        objects[originalObjectsSize + i] = &newObjects[i];
    }
}

//TODO
//change the key in the hashmap to the modelMaterialEncoded
//allocate descriptor sets if a texture is present (add color only capabilities in the material class)
//get the pipeline from material and not model
//remember to remove pipelines and the renderpass from the Model class

void Scene::recordCommands(vk::CommandBuffer& commandBuffer, Camera* camera, DescriptorAllocator* descriptorAllocator)
{
    instancedDraws.clear();
    normalDraws.clear();

    /*instancedDraws.reserve(objects.size());
    normalDraws.reserve(objects.size());*/
    
    for(Object* o : objects)
    {
        if(o->_model()->_instancedPipeline())
        {
            auto it = instancedDraws.find(o->_modelMaterialEncoded());
            if(it != instancedDraws.end())
            {
                it->second.push_back(o);
            }
            else 
            {
                //put the vectors in a pool and reuse across frames
                instancedDraws.insert(std::make_pair(o->_modelMaterialEncoded(), std::vector<Object*>()));
                instancedDraws[o->_modelMaterialEncoded()].reserve(64);
                instancedDraws[o->_modelMaterialEncoded()].push_back(o);
            }
        }
        else
        {
            normalDraws.push_back(o);
        }
    }
    for(auto& it : instancedDraws)
    {
        if(it.second.size() > 1)
        {
            if (descriptorAllocator)
            {
                commandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    it.second[0]->_model()->_instancedPipeline()->_layout(),
                    0,
                    descriptorAllocator->staticDescriptorSet,
                    {}
                );
            }

            //Push Constants
            //Model component of the push constants is unused
            commandBuffer.pushConstants(it.second[0]->_model()->_instancedPipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), it.second[0]->_pushConstants(camera));

            //Draw
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, it.second[0]->_model()->_instancedPipeline()->_pipeline());

            //Create or reuse the instance buffer
            constexpr size_t instanceDataChunkSize = 16 * sizeof(float);
            size_t instanceDataSize = it.second.size() * instanceDataChunkSize;

            Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* instanceBuffer = nullptr;

            auto bufferIT = instanceBuffers.find(it.first);
            if(bufferIT != instanceBuffers.end()) 
            {
                instanceBuffer = bufferIT->second; 
            }
            else
            {
                instanceBuffer = bufferHeap.alloc<Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>>();

                std::vector<float> instanceData;

                instanceData.resize(instanceDataSize / sizeof(float));

                for (int i = 0; i < it.second.size(); i++)
                {
                    const float* matrixData = glm::value_ptr(it.second[i]->_modelMatrix());
                    memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), matrixData, instanceDataChunkSize);
                }

                instanceBuffer = new (instanceBuffer) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);

                instanceBuffer->moveToGPU();

                instanceBuffers.insert(std::make_pair(it.first, instanceBuffer));
            }

            bool objectModified = false;

            for(Object* o : it.second)
            {
                if(o->modifiedThisFrame) objectModified = true;
            }

            if(bufferIT != instanceBuffers.end() && objectModified)
            {
                //In the future do this so that it only modifies the matrix of the modified object
                std::vector<float> instanceData;

                instanceData.resize(instanceDataSize / sizeof(float));

                for (int i = 0; i < it.second.size(); i++)
                {
                    const float* matrixData = glm::value_ptr(it.second[i]->_modelMatrix());
                    memcpy(reinterpret_cast<char*>(instanceData.data()) + (instanceDataChunkSize * i), matrixData, instanceDataChunkSize);
                }

                if(instanceDataSize == instanceBuffer->_size())
                {
                    //Just modify the data
                    instanceBuffer->update(std::move(instanceData));
                }
                else
                {
                    //the buffer is of the wrong size, delete it and make a new one. reuse the same memory location
                    instanceBuffer->~Buffer();

                    instanceBuffer = new (instanceBuffer) Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(instanceData, engine);
                }
            }

            assert(instanceBuffer != nullptr);

            std::array<vk::Buffer, 2> vertexBuffers = {
                it.second[0]->_model()->_vertexBuffer()._buffer(),
                instanceBuffer->_buffer()

            };
            std::array<vk::DeviceSize, 2> vertexBufferOffsets = {
                0,
                0
            };
            commandBuffer.bindVertexBuffers(0, vertexBuffers, vertexBufferOffsets);

            commandBuffer.bindIndexBuffer(it.second[0]->_model()->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

            //std::cout << it.first->_indexBuffer()._typedSize() << " " << it.second.size() << '\n';
            commandBuffer.drawIndexed(static_cast<uint32_t>(it.second[0]->_model()->_indexBuffer()._typedSize()), it.second.size(), 0, 0, 0);
        }
        else if(it.second.size() == 1)
        {
            normalDraws.push_back(it.second[0]);
        }
    }

    for(Object* o : normalDraws)
    {
        if (descriptorAllocator)
        {
            commandBuffer.bindDescriptorSets(
                vk::PipelineBindPoint::eGraphics,
                o->_model()->_pipeline()->_layout(),
                0,
                descriptorAllocator->staticDescriptorSet,
                {}
            );
        }

        //Push Constants
        commandBuffer.pushConstants(o->_model()->_pipeline()->_layout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), o->_pushConstants(camera));

        //Draw
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, o->model->_pipeline()->_pipeline());
        commandBuffer.bindVertexBuffers(0, o->_vertexBuffer()._buffer(), { 0 });

        commandBuffer.bindIndexBuffer(o->_indexBuffer()._buffer(), 0, vk::IndexType::eUint32);

        commandBuffer.drawIndexed(static_cast<uint32_t>(o->_indexBuffer()._typedSize()), 1, 0, 0, 0);
    }

    for(Object* o : objects) { o->afterRender(); };
}