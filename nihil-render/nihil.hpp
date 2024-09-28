#pragma once

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Classes/App/App.hpp"
#include "Classes/Buffer/Buffer.hpp"
#include "Classes/Commands/Command.hpp"
#include "Classes/Engine/Engine.hpp"
#include "Classes/Engine/Engine.Structs.InData.hpp"
#include "Classes/Engine/RenderStructs.hpp"
#include "Classes/InstanceData/InstanceData.hpp"
#include "Classes/Memory/Memory.hpp"
#include "Classes/Model/Model.hpp"
#include "Classes/Pipeline/Pipeline.hpp"
#include "Classes/Renderer/Renderer.hpp"
#include "Classes/Scene/Scene.hpp"
#include "Classes/SPIRV/SPIRV.hpp"

#include "glm/glm.hpp"
#include "nstd/nstd.hpp"

#include "implicit_cast.hpp"
#include "TerminalColors.hpp"