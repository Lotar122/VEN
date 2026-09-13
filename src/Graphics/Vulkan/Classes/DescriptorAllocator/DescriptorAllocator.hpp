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
The pools are all created with a size of 128. if a pool is depleted a new one is created.
At swapchain recreation the pools are recreated (in the sense that the per frame ones are). they are not merged since it would need to recreate all sets
*/

namespace nihil::graphics
{
	class DescriptorAllocator : public onSwapchainRecreationListener
	{
	private:
		inline vk::DescriptorPool& dynamicPoolsView(size_t poolIndex)
		{
			// poolIndex = pool "generation" index (see growDynamicPools()).
			// One physical pool is created per generation and serves every frame's
			// allocations; per-frame accounting lives in dynamicPoolSizes instead,
			// so this does NOT index by frame (the old x*frameCount+y formula
			// didn't match how growDynamicPools() actually grows dynamicPools).
			return dynamicPools[poolIndex];
		}

		inline size_t& dynamicPoolSizesView(size_t x, size_t y, size_t z = 0)
		{
			// x = Pool ("generation") Index
			// y = Frame Index
			// z = Descriptor Type Index, or allTypes.size() for the "sets allocated" slot
			// Each object has (frameCount * (allTypes.size() + 1)) slots
			return dynamicPoolSizes[x * (frameCount * (allTypes.size() + 1)) + y * (allTypes.size() + 1) + z];
		}

		inline size_t& staticPoolSizesView(size_t x, size_t y)
		{
			// x = Pool Index
			// y = Descriptor Type Index, or allTypes.size() for the "sets allocated" slot
			return staticPoolSizes[x * (allTypes.size() + 1) + y];
		}

		// NOTE: previously declared as size 12 with only 11 initializers, which silently
		// value-initialized a 12th, duplicate vk::DescriptorType::eSampler entry (enum value 0).
		// That desynced every array/loop below that assumes one slot per real descriptor type.
		static constexpr std::array<vk::DescriptorType, 11> allTypes = {
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

			// resize() value-initializes the new size_t elements to 0, so the new
			// (allTypes.size() + 1)-wide slice for this pool starts zeroed already.
			staticPoolSizes.resize(staticPoolSizes.size() + (allTypes.size() + 1));

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

			// See growStaticPools() - resize() already zero-initializes the new counters.
			dynamicPoolSizes.resize(dynamicPoolSizes.size() + (frameCount * (allTypes.size() + 1)));

			vk::DescriptorPoolCreateInfo poolInfo{};
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = 128; // maximum number of descriptor sets this pool can allocate

			dynamicPools.emplace_back(engine->_device().createDescriptorPool(poolInfo));
		}
	public:
		Engine* engine = nullptr;
		bool createdStaticDescriptorSet = false;

		size_t prevFrameCount = 0;
		size_t frameCount = 0;

		std::vector<vk::DescriptorPool> staticPools;
		std::vector<vk::DescriptorPool> dynamicPools;

		std::vector<size_t> staticPoolSizes;
		std::vector<size_t> dynamicPoolSizes;

		size_t dynamicPoolSize = 0; // currently unused within this class

		std::vector<DescriptorSet<AssetUsage::Static>*> dynamicSets;

		//Deprecated. Pipelines will store the sets they use.
		DescriptorSet<AssetUsage::Static>* globalDescriptorSet;

		DescriptorAllocator(Engine* _engine)
		{
			assert(_engine != nullptr);

			engine = _engine;

			engine->_swapchain()->addEventListener(this, Listeners::onSwapchainRecreation);

			frameCount = engine->_swapchain()->imageCount;
			prevFrameCount = frameCount;

			growStaticPools();
			// FIX: dynamicPools/dynamicPoolSizes used to start empty, which made the very
			// first allocateDynamicDescriptorSet() call underflow (0 / stride - 1) and index
			// far out of bounds. Seed one dynamic pool up front, same as the static pool.
			growDynamicPools();
		}

		inline void setGlobalDescriptorSet(DescriptorSet<AssetUsage::Static>* _globalDescriptorSet) { globalDescriptorSet = _globalDescriptorSet; };

		void onSwapchainRecreation() final override
		{
			size_t newFrameCount = engine->_swapchain()->imageCount;

			if (newFrameCount != frameCount)
			{
				// FIX: dynamicPoolSizesView()'s offsets are computed using frameCount as a
				// stride, so every existing dynamic pool's bookkeeping is silently wrong the
				// moment frameCount changes. Tear the dynamic pools down and rebuild, matching
				// what the class comment above already promised.
				for (auto& p : dynamicPools)
				{
					engine->_device().destroyDescriptorPool(p);
				}
				dynamicPools.clear();
				dynamicPoolSizes.clear();

				prevFrameCount = frameCount;
				frameCount = newFrameCount;

				growDynamicPools();
			}

			Carbo::Logger::Log("Swapchain Recreation Event in DescriptorAllocator");
		}

		vk::DescriptorSet allocateDynamicDescriptorSet(vk::DescriptorSetLayout* layout, std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings, size_t frameIndex)
		{
			//allocate in the pool corresponding to the frame.
			//Write a specialization for AssetUsage::Dynamic that uses this instead and it keeps frameCount vk::DescriptorSet objects so that they can be updated every frame. make it inherit from the DescriptorSet class.
			//You'll need to make an abstraact base class.

			size_t stride = frameCount * (allTypes.size() + 1);
			size_t poolIndex = (dynamicPoolSizes.size() / stride) - 1;

			// The reserved slot at z == allTypes.size() tracks how many *sets* (not
			// descriptors) this (pool, frame) has handed out, so maxSets (128) gets
			// respected even when no single descriptor type is near its own 128 cap.
			if (dynamicPoolSizesView(poolIndex, frameIndex, allTypes.size()) == 128)
			{
				growDynamicPools();
				poolIndex = (dynamicPoolSizes.size() / stride) - 1;
			}

			std::array<size_t, allTypes.size()> sizes{};

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				sizes[(int)dsb.layoutBinding.descriptorType]++;
			}

			for (int i = 0; i < sizes.size(); i++)
			{
				if (dynamicPoolSizesView(poolIndex, frameIndex, i) + sizes[i] > 128) { growDynamicPools(); poolIndex = (dynamicPoolSizes.size() / stride) - 1; break; }
			}

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				dynamicPoolSizesView(poolIndex, frameIndex, (int)dsb.layoutBinding.descriptorType)++;
			}
			dynamicPoolSizesView(poolIndex, frameIndex, allTypes.size())++;

			vk::DescriptorSetAllocateInfo allocInfo{
				dynamicPools.back(),
				1,
				layout
			};

			return engine->_device().allocateDescriptorSets(allocInfo)[0];
		}

		vk::DescriptorSet allocateStaticDescriptorSet(vk::DescriptorSetLayout* layout, std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings)
		{
			size_t poolIndex = (staticPoolSizes.size() / (allTypes.size() + 1)) - 1;

			if (staticPoolSizesView(poolIndex, allTypes.size()) == 128)
			{
				growStaticPools();
				poolIndex = (staticPoolSizes.size() / (allTypes.size() + 1)) - 1;
			}

			std::array<size_t, allTypes.size()> sizes{};

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				sizes[(int)dsb.layoutBinding.descriptorType]++;
			}

			for (int i = 0; i < sizes.size(); i++)
			{
				if (staticPoolSizesView(poolIndex, i) + sizes[i] > 128) { growStaticPools(); poolIndex = (staticPoolSizes.size() / (allTypes.size() + 1)) - 1; break; }
			}

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				staticPoolSizesView(poolIndex, (int)dsb.layoutBinding.descriptorType)++;
			}
			staticPoolSizesView(poolIndex, allTypes.size())++;

			vk::DescriptorSetAllocateInfo allocInfo{
				staticPools.back(),
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
			for (auto& p : dynamicPools)
			{
				engine->_device().destroyDescriptorPool(p);
			}
			for (auto& p : staticPools)
			{
				engine->_device().destroyDescriptorPool(p);
			}
		}
	};
}