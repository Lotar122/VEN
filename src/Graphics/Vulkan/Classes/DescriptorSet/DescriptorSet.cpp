#include "DescriptorSet.hpp"
#include "Classes/AssetUsage/AssetUsage.hpp"
#include "Classes/DescriptorAllocator/DescriptorAllocator.hpp"

namespace nihil::graphics
{
    void DescriptorSet<nihil::AssetUsage::Static>::create(std::vector<DescriptorSetLayoutBinding>&& _descriptorSetLayoutBindings, DescriptorAllocator* _descriptorAllocator, Engine* _engine)
    {
        assert(_engine != nullptr);
        assert(_descriptorAllocator != nullptr);
        assert(usage != AssetUsage::Undefined);

        engine = _engine;
        descriptorAllocator = _descriptorAllocator;

        std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
        descriptorSetLayoutBindings.reserve(_descriptorSetLayoutBindings.size());

        for (const DescriptorSetLayoutBinding& dsb : _descriptorSetLayoutBindings)
        {
            descriptorSetLayoutBindings.push_back(dsb.layoutBinding);
            descriptors.emplace(dsb.layoutBinding.binding, dsb);
        }

        vk::DescriptorSetLayoutCreateInfo descriptorLayoutInfo({}, static_cast<uint32_t>(descriptorSetLayoutBindings.size()), descriptorSetLayoutBindings.data());
        layout.assignRes(engine->_device().createDescriptorSetLayout(descriptorLayoutInfo), engine->_device());

        set.assignRes(descriptorAllocator->allocateStaticDescriptorSet(layout.getResP(), _descriptorSetLayoutBindings), engine->_device());

        descriptorAllocator->writeStaticDescriptorSets(set, _descriptorSetLayoutBindings);
    }
}