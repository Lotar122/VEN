#pragma once

#include <unordered_map>
#include "Classes/Engine/Engine.hpp"
#include "Classes/Listeners/Listeners.hpp"
#include "Classes/Listeners/onSwapchainRecreation.hpp"
#include "Classes/Resource/Resource.hpp"
#include "Classes/Sampler/Sampler.hpp"
#include "Classes/Texture/Texture.hpp"
#include "Classes/Asset/Asset.hpp"
#include "Classes/Resources/Resources.hpp"
#include "Classes/DescriptorSet/DescriptorSet.hpp"
#include "Classes/Swapchain/Swapchain.hpp"
#include <vulkan/vulkan.hpp>
#include <ranges>

/*
The pools are all created with a size of 128. if a pool is depleeted a new one is created.
At swapchain recreation the pools are recreated (in the sense that the per frame ones are). they are not merged since it would need to recreate all sets
*/

namespace nihil::graphics
{
	class DescriptorAllocator : public onSwapchainRecreationListener
	{
	private:
		inline Resource<vk::DescriptorPool>& dynamicPoolsView(size_t x, size_t y)
		{
			return dynamicPools[x * frameCount + y];
		}
	public:
		Engine* engine = nullptr;
		DescriptorSet staticDescriptorSet;
		bool createdStaticDescriptorSet = false;

		size_t prevFrameCount = 0;
		size_t frameCount = 0;

		std::vector<Resource<vk::DescriptorPool>> staticPools;
		std::vector<Resource<vk::DescriptorPool>> dynamicPools;

		std::vector<size_t> staticPoolSizes;
		std::vector<size_t> dynamicPoolSizes;

		DescriptorAllocator(Engine* _engine) 
		{
			assert(_engine != nullptr);

			engine = _engine;

			engine->_swapchain()->addEventListener(this, Listeners::onSwapchainRecreation);

			frameCount = engine->_swapchain()->imageCount;
			prevFrameCount = frameCount;
		}

		void onSwapchainRecreation() final override
		{
			frameCount = engine->_swapchain()->imageCount;

			Logger::Log("Swapchain Recreation Event in DescriptorAllocator");
		}

		void createStaticDescriptorSet(const std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings)
		{
			if (createdStaticDescriptorSet) Logger::Exception("Cannot recreate the static descriptor set.");

			staticDescriptorSet.create(_descriptorSetLayoutBindings, engine);
		}

		void createDynamicDescriptorSet(std::vector<vk::DescriptorSetLayoutBinding>& dynamicDescriptors)
		{
			vk::DescriptorSetLayoutCreateInfo dynamicDescriptorSetLayoutInfo{};
			dynamicDescriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(dynamicDescriptors.size());
			dynamicDescriptorSetLayoutInfo.pBindings = dynamicDescriptors.data();

			//return engine->_device().createDescriptorSetLayout(dynamicDescriptorSetLayoutInfo);
		}

		void requestDynamicDescriptorSet()
		{

		}
	};
}