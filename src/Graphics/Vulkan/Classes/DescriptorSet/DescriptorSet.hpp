#pragma once

#include "vulkan/vulkan.hpp"
#include "Classes/Resources/Resources.hpp"
#include "Standard/Logger.hpp"
#include "Classes/Texture/Texture.hpp"
#include "Classes/Sampler/Sampler.hpp"
#include "Classes/AssetUsage/AssetUsage.hpp"
#include <unordered_map>

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
			else type = Type::DescriptorBufferInfo;

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

    class DescriptorSet
    {
    public:
        Engine* engine = nullptr;
		AssetUsage usage;
        Resource<vk::DescriptorSetLayout> descriptorSetLayout;
        Resource<vk::DescriptorPool> descriptorPool;
		Resource<vk::DescriptorSet> descriptorSet;

        bool created = false;

        std::unordered_map<uint32_t, DescriptorSetLayoutBinding> descriptors;

        DescriptorSet() {};

        inline DescriptorSet(const std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings, Engine* _engine, AssetUsage _usage = AssetUsage::Static) : usage(_usage)
        {
            create(_descriptorSetLayoutBindings, _engine, usage);
        }

        void create(const std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings, Engine* _engine, AssetUsage _usage = AssetUsage::Static)
        {
            if(created) Logger::Exception("The descriptor set has already been created.");
            created = true;
            assert(_engine != nullptr);
			assert(_usage != AssetUsage::Undefined);

			engine = _engine;
			usage = _usage;

			std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;

			std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
			descriptorSetLayoutBindings.reserve(_descriptorSetLayoutBindings.size());

			std::vector<vk::WriteDescriptorSet> descriptorWrites;
			descriptorWrites.reserve(descriptorSetLayoutBindings.size());

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				descriptorSetLayoutBindings.push_back(dsb.layoutBinding);
				descriptors.emplace(dsb.layoutBinding.binding, dsb);

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

			vk::DescriptorSetLayoutCreateInfo descriptorLayoutInfo({}, static_cast<uint32_t>(descriptorSetLayoutBindings.size()), descriptorSetLayoutBindings.data());
			descriptorSetLayout.assignRes(engine->_device().createDescriptorSetLayout(descriptorLayoutInfo), engine->_device());

			vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
			descriptorPoolInfo.maxSets = 1;   // we just need 1 static set
			descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
			descriptorPoolInfo.pPoolSizes = descriptorPoolSizes.data();

			descriptorPool.assignRes(engine->_device().createDescriptorPool(descriptorPoolInfo), engine->_device());

			vk::DescriptorSetAllocateInfo allocInfo{
				descriptorPool,
				1,
				descriptorSetLayout.getResP()
			};

			descriptorSet.assignRes(engine->_device().allocateDescriptorSets(allocInfo)[0], engine->_device());

			for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				switch (dsb.descriptorInfo.type)
				{
					case DescriptorInfo::Type::DescriptorImageInfo:
						descriptorWrites.emplace_back(descriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, &dsb.descriptorInfo.data.imageInfo);
						break;
					case DescriptorInfo::Type::DescriptorBufferInfo:
						descriptorWrites.emplace_back(descriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, nullptr, &dsb.descriptorInfo.data.bufferInfo);
						break;
					case DescriptorInfo::Type::BufferViewInfo:
						descriptorWrites.emplace_back(descriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, nullptr, nullptr, &dsb.descriptorInfo.data.bufferView);
						break;
				}
			}

			engine->_device().updateDescriptorSets(descriptorWrites, {});
        }

        //You can disable checks if you wish for that tiny bit of performace. Although it is not advised as it may result in driver errors of memory corruption
        template<bool checks = true>
        void update(const std::vector<DescriptorSetLayoutBinding>& _descriptorSetLayoutBindings)
        {
            if(_descriptorSetLayoutBindings.size() == 0) return;
            if constexpr (checks)
            {
                for(const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
                {
                    auto it = descriptors.find(dsb.layoutBinding.binding);
                    if(it != descriptors.end())
                    {
                        if(it->second.descriptorInfo.type == dsb.descriptorInfo.type) continue;
                        else Logger::Exception("Type mismatch between the descriptor to be written and the descriptor residing at binding: {}", dsb.layoutBinding.binding);
                    }
                    else Logger::Exception("The descriptors binding: {} does not point to a valid descriptor in this set.", dsb.layoutBinding.binding);
                }
            }

            std::vector<vk::WriteDescriptorSet> descriptorWrites;
			descriptorWrites.reserve(_descriptorSetLayoutBindings.size());

            for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
			{
				switch (dsb.descriptorInfo.type)
				{
					case DescriptorInfo::Type::DescriptorImageInfo:
						descriptorWrites.emplace_back(descriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, &dsb.descriptorInfo.data.imageInfo);
						break;
					case DescriptorInfo::Type::DescriptorBufferInfo:
						descriptorWrites.emplace_back(descriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, nullptr, &dsb.descriptorInfo.data.bufferInfo);
						break;
					case DescriptorInfo::Type::BufferViewInfo:
						descriptorWrites.emplace_back(descriptorSet, dsb.layoutBinding.binding, 0, 1, dsb.layoutBinding.descriptorType, nullptr, nullptr, &dsb.descriptorInfo.data.bufferView);
						break;
				}
			}

			engine->_device().updateDescriptorSets(descriptorWrites, {});
        }
    };
}