#include "Engine.hpp"

namespace nihil::graphics {
    void Engine::registerObjectForDeletion(BufferBase* buffer)
    {
        bufferStorage.push_back(buffer);
        std::cout << "Passed a new buffer" << std::endl;
    }
    void Engine::registerObjectForDeletion(vk::Image image)
    {
        imageStorage.push_back(image);
        std::cout << "Passed a new image" << std::endl;
    }
    void Engine::registerObjectForDeletion(vk::ImageView imageView)
    {
        imageViewStorage.push_back(imageView);
        std::cout << "Passed a new imageView" << std::endl;
    }
    void Engine::registerObjectForDeletion(vk::ShaderModule shaderModule)
    {
        shaderModuleStorage.push_back(shaderModule);
        std::cout << "Passed a new shaderModule" << std::endl;
    }
    void Engine::registerObjectForDeletion(vk::PipelineLayout pipelineLayout)
    {
        pipelineLayoutStorage.push_back(pipelineLayout);
        std::cout << "Passed a new pipelineLayout" << std::endl;
    }
    void Engine::registerObjectForDeletion(vk::RenderPass renderPass) 
    {
        renderPassStorage.push_back(renderPass);
        std::cout << "Passed a new renderPass" << std::endl;
    }
    void Engine::registerObjectForDeletion(vk::DeviceMemory memory)
    {
        memoryStorage.push_back(memory);
        std::cout << "Passed a new memory" << std::endl;
    }
}