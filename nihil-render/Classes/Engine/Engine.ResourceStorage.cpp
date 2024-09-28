#include "Engine.hpp"

namespace nihil::graphics {
    void Engine::registerObjectForDeletion(BufferBase* buffer)
    {
        bufferStorage.push_back(buffer);
    }
    void Engine::registerObjectForDeletion(vk::Image image)
    {
        imageStorage.push_back(image);
    }
    void Engine::registerObjectForDeletion(vk::ImageView imageView)
    {
        imageViewStorage.push_back(imageView);
    }
    void Engine::registerObjectForDeletion(vk::ShaderModule shaderModule)
    {
        shaderModuleStorage.push_back(shaderModule);
    }
    void Engine::registerObjectForDeletion(vk::PipelineLayout pipelineLayout)
    {
        pipelineLayoutStorage.push_back(pipelineLayout);
    }
    void Engine::registerObjectForDeletion(vk::RenderPass renderPass) 
    {
        renderPassStorage.push_back(renderPass);
    }
    void Engine::registerObjectForDeletion(vk::DeviceMemory memory)
    {
        memoryStorage.push_back(memory);
    }
}