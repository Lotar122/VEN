#pragma once

#include <unordered_map>
#include "Classes/AssetUsage/AssetUsage.hpp"
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
#include "vulkan/vulkan_handles.hpp"
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
		inline vk::DescriptorPool& dynamicPoolsView(size_t x, size_t y)
		{
			return dynamicPools[x * frameCount + y];
		}

		inline size_t& dynamicPoolSizesView(size_t x, size_t y, size_t z = 0)
		{
			return dynamicPoolSizes[x * (allTypes.size() + 1) + y];
		}

		inline size_t& staticPoolSizesView(size_t x, size_t y)
		{
			return staticPoolSizes[x * (allTypes.size() + 1) + y];
		}

		static constexpr std::array<vk::DescriptorType, 12> allTypes = {
			vk::DescriptorType::eSampler,
			vk::DescriptorType::eCombinedImageSampler,
			vk::DescriptorType::eSampledImage,
			vk::DescriptorType::eStorageImage,
			vk::DescriptorType::eUniformTexelBuffer,
			vk::DescriptorType::eStorageTexelBuffer,
			vk::DescriptorType::eUniformBuffer,
			vk::DescriptorType::eStorageBuffer,
			vk::DescriptorType::eUniformBufferDynamic,
			vk::DescriptorType::eStorageBufferDynamic,
			vk::DescriptorType::eInputAttachment,
		};

		void growStaticPools()
		{
			std::vector<vk::DescriptorPoolSize> poolSizes;
			poolSizes.reserve(allTypes.size());
			for (auto& type : allTypes) {
				vk::DescriptorPoolSize size{};
				size.type = type;
				size.descriptorCount = 128; // adjust depending on your expected usage
				poolSizes.push_back(size);
			}

			staticPoolSizes.resize(staticPoolSizes.size() + (allTypes.size() + 1));

			for(int i = staticPoolSizes.size() - (1 * (allTypes.size() + 1)); i < staticPoolSizes.size(); i++)
			{
				staticPoolSizes[i] = 0;
			}

			vk::DescriptorPoolCreateInfo poolInfo{};
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 128; // maximum number of descriptor sets this pool can allocate

			staticPools.emplace_back(engine->_device().createDescriptorPool(poolInfo));
		}

		void growDynamicPools()
		{
			std::vector<vk::DescriptorPoolSize> poolSizes;
			poolSizes.reserve(allTypes.size());
			for (auto& type : allTypes) {
				vk::DescriptorPoolSize size{};
				size.type = type;
				size.descriptorCount = 128; // adjust depending on your expected usage
				poolSizes.push_back(size);
			}

			dynamicPoolSizes.resize(dynamicPoolSizes.size() + (frameCount * (allTypes.size() + 1)));

			for(int i = dynamicPoolSizes.size() - (frameCount * (allTypes.size() + 1)); i < dynamicPoolSizes.size(); i++)
			{
				dynamicPoolSizes[i] = 0;
			}
		}
	public:
		std::vector<vk::DescriptorPool> staticPools;
		std::vector<vk::DescriptorPool> dynamicPools;

		std::vector<size_t> staticPoolSizes;
		std::vector<size_t> dynamicPoolSizes;

		std::vector<DescriptorSet<AssetUsage::Static>*> dynamicSets;

		//Deprecated. Pipelines will store the sets they use.
		DescriptorSet<AssetUsage::Static>* globalDescriptorSet;

		size_t prevFrameCount = 0;
		size_t frameCount = 0;

		size_t dynamicPoolSize;

		Engine* engine = nullptr;

		bool createdStaticDescriptorSet = false;

		DescriptorAllocator(Engine* _engine) 
		{
			assert(_engine != nullptr);

			engine = _engine;

			engine->_swapchain()->addEventListener(this, Listeners::onSwapchainRecreation);

			frameCount = engine->_swapchain()->imageCount;
			prevFrameCount = frameCount;

			growStaticPools();
		}

		inline void setGlobalDescriptorSet(DescriptorSet<AssetUsage::Static>* _globalDescriptorSet) { globalDescriptorSet = _globalDescriptorSet; };

		void onSwapchainRecreation() final override
		{
			frameCount = engine->_swapchain()->imageCount;

			Logger::Log("Swapchain Recreation Event in DescriptorAllocator");
		}

		vk::DescriptorSet allocateDynamicDescriptorSet(vk::DescriptorSetLayout* layout, std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings, size_t frameIndex)
		{
			//allocate in the pool corresponding to the frame.
			//Write a specialization for AssetUsage::Dynamic that uses this instead and it keeps frameCount vk::DescriptorSet objects so that they can be updated every frame. make it inherit from the DescriptorSet class.
			//You'll need to make an abstraact base class.
		}

		vk::DescriptorSet allocateStaticDescriptorSet(vk::DescriptorSetLayout* layout, std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings)
		{
			size_t h = (staticPoolSizes.size() / (allTypes.size() + 1)) - 1;
			if(staticPoolSizes[(staticPoolSizes.size() / (allTypes.size() + 1)) - 1] == 128) growStaticPools();
			std::array<size_t, allTypes.size()> sizes;

			for(int i = 0; i < allTypes.size(); i++)
			{
				sizes[i] = 0;
			}

			for(const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				sizes[(int)dsb.layoutBinding.descriptorType]++;
			}

			for(int i = 0; i < sizes.size(); i++)
			{
				size_t x = staticPoolSizesView((staticPoolSizes.size() / (allTypes.size() + 1)) - 1, i) + sizes[i];
				if(staticPoolSizesView((staticPoolSizes.size() / (allTypes.size() + 1)) - 1, i) + sizes[i] > 128) { growStaticPools(); break; }
			}

			for(const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				staticPoolSizesView((staticPoolSizes.size() / (allTypes.size() + 1)) - 1, (int)dsb.layoutBinding.descriptorType)++;
			}

			vk::DescriptorSetAllocateInfo allocInfo{
				staticPools.front(),
				1,
				layout
			};

			return engine->_device().allocateDescriptorSets(allocInfo)[0];
		}

		void writeStaticDescriptorSets(vk::DescriptorSet set, std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings)
		{
			std::vector<vk::WriteDescriptorSet> descriptorWrites;
    		descriptorWrites.reserve(_descriptorSetLayoutBindings.size());

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				switch (dsb.descriptorInfo.type)
				{
					case DescriptorInfo::Type::DescriptorImageInfo:
						descriptorWrites.emplace_back(set, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, &dsb.descriptorInfo.data.imageInfo);
						break;
					case DescriptorInfo::Type::DescriptorBufferInfo:
						descriptorWrites.emplace_back(set, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, nullptr, &dsb.descriptorInfo.data.bufferInfo);
						break;
					case DescriptorInfo::Type::BufferViewInfo:
						descriptorWrites.emplace_back(set, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, nullptr, nullptr, &dsb.descriptorInfo.data.bufferView);
						break;
				}
			}

			engine->_device().updateDescriptorSets(descriptorWrites, {});
		}

		~DescriptorAllocator()
		{
			for(auto& p : dynamicPools)
			{
				engine->_device().destroyDescriptorPool(p);
			}
			for(auto& p : staticPools)
			{
				engine->_device().destroyDescriptorPool(p);
			}
		}
	};
}