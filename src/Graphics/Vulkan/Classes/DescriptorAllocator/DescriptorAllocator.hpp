#pragma once

#include <unordered_map>
#include "Classes/Engine/Engine.hpp"
#include "Classes/Sampler/Sampler.hpp"
#include "Classes/Texture/Texture.hpp"
#include "Classes/Asset/Asset.hpp"
#include "Classes/Resources/Resources.hpp"
#include <vulkan/vulkan.hpp>
#include <ranges>

namespace nihil::graphics
{
	struct DescriptorInfo
	{
		enum class Type
		{
			DescriptorBufferInfo,
			DescriptorImageInfo,
			BufferViewInfo
		};

		union Data
		{
			vk::DescriptorBufferInfo bufferInfo;
			vk::DescriptorImageInfo imageInfo;
			vk::BufferView bufferView;

			Data(vk::DescriptorBufferInfo _bufferInfo) : bufferInfo(_bufferInfo) {}
			Data(vk::DescriptorImageInfo _imageInfo) : imageInfo(_imageInfo) {}
			Data(vk::BufferView _bufferView) : bufferView(_bufferView) {}
			Data() {};
		};

		Data data;
		Type type;

		template<typename T>
		requires requires(T t) { t.getDescriptorInfo(); }
		DescriptorInfo(T* resourcePointer)
		{
			if constexpr (std::is_same_v<T, Texture>) type = Type::DescriptorImageInfo;
			else if constexpr (std::is_same_v<T, Sampler>) type = Type::DescriptorImageInfo;

			data = resourcePointer->getDescriptorInfo();
		}

		inline vk::DescriptorBufferInfo& _getBufferInfo() { return data.bufferInfo; };
		inline vk::DescriptorImageInfo& _getImageInfo() { return data.imageInfo; };
		inline vk::BufferView& _getBufferView() { return data.bufferView; };
	};

	struct DescriptorSetLayoutBinding
	{
		vk::DescriptorSetLayoutBinding layoutBinding;
		AssetUsage usage;
		DescriptorInfo descriptorInfo;

		template<typename T>
		requires requires(T t) { t.getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits::eAll, 0); }
		DescriptorSetLayoutBinding(T* resourcePointer, vk::ShaderStageFlagBits shaderStage, uint32_t binding) :
			layoutBinding(resourcePointer->getDescriptorSetLayoutBinding(shaderStage, binding)), usage(resourcePointer->_getAssetUsage()), descriptorInfo(resourcePointer) {};
	};

	class DescriptorAllocator
	{
	public:
		Engine* engine = nullptr;
		std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> staticDescriptors;
		Resource<vk::DescriptorSetLayout> staticDescriptorSetLayout;
		vk::DescriptorPool staticDescriptorPool;
		vk::DescriptorSet staticDescriptorSet;
		bool createdStaticDescriptorSet = false;

		DescriptorAllocator() {};

		void createStaticDescriptorSet(const std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings, Engine* _engine)
		{
			if (createdStaticDescriptorSet) Logger::Exception("Cannot recreate the static descriptor set.");

			assert(_engine != nullptr);

			engine = _engine;

			std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;

			std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
			descriptorSetLayoutBindings.reserve(_descriptorSetLayoutBindings.size());

			std::vector<vk::WriteDescriptorSet> descriptorWrites;
			descriptorWrites.reserve(descriptorSetLayoutBindings.size());

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				descriptorSetLayoutBindings.push_back(dsb.layoutBinding);
				staticDescriptors.emplace(dsb.layoutBinding.binding, dsb.layoutBinding);

				auto it = std::find_if(descriptorPoolSizes.begin(), descriptorPoolSizes.end(), [&dsb](const vk::DescriptorPoolSize& a) {
					return a.type == dsb.layoutBinding.descriptorType; 
				});

				if(it != descriptorPoolSizes.end())
				{
					it->descriptorCount++;
				}
				else
				{
					descriptorPoolSizes.emplace_back(dsb.layoutBinding.descriptorType, 1);
				}
			}

			vk::DescriptorSetLayoutCreateInfo descriptorLayoutInfo({}, descriptorSetLayoutBindings.size(), descriptorSetLayoutBindings.data());
			staticDescriptorSetLayout.assignRes(engine->_device().createDescriptorSetLayout(descriptorLayoutInfo), engine->_device());

			vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
			descriptorPoolInfo.maxSets = 1;   // we just need 1 static set
			descriptorPoolInfo.poolSizeCount = descriptorPoolSizes.size();
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes.data();

			staticDescriptorPool = engine->_device().createDescriptorPool(descriptorPoolInfo);

			vk::DescriptorSetAllocateInfo allocInfo{
				staticDescriptorPool,
				1,
				staticDescriptorSetLayout.getResP()
			};

			staticDescriptorSet = engine->_device().allocateDescriptorSets(allocInfo)[0];

			//TODO: make Resource for all new types (vk::DescriptorSet, vk::DescriptorPool)

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				switch (dsb.descriptorInfo.type)
				{
					case DescriptorInfo::Type::DescriptorImageInfo:
						descriptorWrites.emplace_back(staticDescriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, &dsb.descriptorInfo.data.imageInfo);
						break;
					case DescriptorInfo::Type::DescriptorBufferInfo:
						descriptorWrites.emplace_back(staticDescriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, nullptr, &dsb.descriptorInfo.data.bufferInfo);
						break;
					case DescriptorInfo::Type::BufferViewInfo:
						descriptorWrites.emplace_back(staticDescriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, nullptr, nullptr, &dsb.descriptorInfo.data.bufferView);
						break;
				}
			}

			engine->_device().updateDescriptorSets(descriptorWrites, {});

			//update descriptor sets. use the descriptor set info for the info. 
			//iterate through the std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings; to construct the writes vector;
		}

		void createDynamicDescriptorSet(std::vector<vk::DescriptorSetLayoutBinding>& dynamicDescriptors)
		{
			vk::DescriptorSetLayoutCreateInfo dynamicDescriptorSetLayoutInfo{};
			dynamicDescriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(dynamicDescriptors.size());
			dynamicDescriptorSetLayoutInfo.pBindings = dynamicDescriptors.data();


		}

		void requestDynamicDescriptorSet()
		{

		}
	};
}