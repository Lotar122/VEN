#pragma once

#include <vulkan/vulkan.hpp>

#include <fstream>
#include <vector>

#include "Logger.hpp"

#include "Classes/Engine/Engine.hpp"

#include "Classes/Asset/Asset.hpp"

namespace nihil::graphics
{
    class Shader : public Asset
    {
    public:
        Resource<vk::ShaderModule> shaderModule;

        std::string name;
    private:
        Engine* engine = nullptr;
    public:
        Shader(Engine* _engine);

        void LoadFromSource(const std::string& path);
        void LoadFromBinary(const std::string& path);

        ~Shader();

        inline vk::ShaderModule* _ptr() { return shaderModule.getResP(); };
    };
}