#pragma once

#include "Classes/Engine/Engine.hpp"
#include "Classes/Resources/Resources.hpp"
#include "StorageBuffer.hpp"
#include "FindMemoryTypeIndex.hpp"
#include "Classes/Asset/Asset.hpp"
#include "vulkan/vulkan.hpp"
#include <iterator>
#include <vector>

//!This violates the Vulkan spec. It rebins memory to buffers which is prohibited. A rewrite which instead recreates the buffer is pending.

namespace nihil::graphics
{
    class Model;

    template<typename T, auto usageT, auto propertiesT = static_cast<vk::MemoryPropertyFlags::MaskType>(vk::MemoryPropertyFlagBits::eDeviceLocal)>
    class Buffer : public Asset
    {
        friend class Model;
        Engine* engine = nullptr;
        vk::Fence transferFence = nullptr;

        size_t size = 0;
        std::vector<T> data;
        static constexpr vk::BufferUsageFlags usage = static_cast<vk::BufferUsageFlags>(usageT);
        static constexpr vk::MemoryPropertyFlags properties = static_cast<vk::MemoryPropertyFlags>(propertiesT);

        vk::MemoryRequirements memRequirements;
        vk::MemoryRequirements stagingMemRequirements;
        vk::PhysicalDeviceMemoryProperties memProperties;

        uint32_t memoryTypeIndex = uint32_t(-1), stagingMemoryTypeIndex = uint32_t(-1);

        Resource<vk::Buffer> buffer;
        Resource<vk::Buffer> stagingBuffer;
        vk::DeviceMemory memory;
        vk::DeviceMemory stagingMemory;

        bool onGPU = false, allocatedOnGPU = false;

        static constexpr bool CPUAccessible = 
            (properties & (vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)) == (vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

        bool destroyed = false;

        enum class UpdateMode
        {
            Immediate,
            Recording
        };

        struct UpdateCommand
        {
            uint64_t srcOffset;
            uint64_t dstOffset;
            uint64_t updateSize;
            uint64_t _pad = 0;
        };

        struct UpdateOptimizer
        {
            std::vector<UpdateCommand> commands;
        };

        UpdateOptimizer updateOptimizer = {};
        UpdateMode updateMode = UpdateMode::Immediate;
        UpdateMode preRecordingUpdateMode = updateMode;
    public:
        inline vk::Buffer _buffer() { return buffer.getRes(); };
        //return the size of the buffer in bytes
        inline size_t _size() const { return size; };
        //returns the size of the buffer in units of its type
        inline size_t _typedSize() const { return data.size(); };

        inline const Engine* _engine() const { return engine; };

        template<UpdateMode updateModeT>
        static void copyBufferImpl(vk::Buffer src, vk::Buffer dst, size_t size, vk::BufferCopy copyRegion, Engine* engine)
        {
            if constexpr (updateModeT == UpdateMode::Immediate) 
            {
                vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
                discardResult = engine->_device().resetFences(1, &engine->_transferFence());

                vk::CommandBufferBeginInfo beginInfo{};
                engine->_mainCommandBuffer().begin(beginInfo);
            }

            engine->_mainCommandBuffer().copyBuffer(src, dst, copyRegion);

            if constexpr (updateModeT == UpdateMode::Immediate) 
            {
                vk::BufferMemoryBarrier barrier{
                    vk::AccessFlagBits::eTransferWrite,
                    vk::AccessFlagBits::eShaderRead,
                    VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED,
                    dst,
                    0,
                    size
                };

                engine->_mainCommandBuffer().pipelineBarrier(
                    vk::PipelineStageFlagBits::eTransfer,
                    vk::PipelineStageFlagBits::eVertexShader,
                    {},
                    nullptr,
                    barrier,
                    nullptr
                );

                engine->_mainCommandBuffer().end();

                vk::SubmitInfo submitInfo = {};
                submitInfo.waitSemaphoreCount = 0;
                submitInfo.pWaitSemaphores = nullptr;

                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

                vk::Result discardResult = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());
            }
        }

        template<UpdateMode updateModeT = UpdateMode::Immediate>
        static inline void copyBuffer(vk::Buffer src, vk::Buffer dst, size_t size, Engine* engine)
        {
            copyBufferImpl<updateModeT>(src, dst, size, { 0, 0, size }, engine);
        }

        template<UpdateMode updateModeT = UpdateMode::Immediate>
        static inline void copyBuffer(vk::Buffer src, vk::Buffer dst, size_t size, vk::BufferCopy copyRegion, Engine* engine)
        {
            copyBufferImpl<updateModeT>(src, dst, size, copyRegion, engine);
        }

        vk::DescriptorSetLayoutBinding  getDescriptorSetLayoutBinding(vk::ShaderStageFlagBits shaderStage, uint32_t binding)
        {
            if constexpr (
                usageT == vk::BufferUsageFlagBits::eStorageBuffer || 
                usageT == vk::BufferUsageFlagBits::eUniformBuffer
            )
            {
                vk::DescriptorSetLayoutBinding bufferBinding{};
                bufferBinding.binding = binding;
                /*samplerBinding.descriptorType = vk::DescriptorType::;*/
                if constexpr (usageT == vk::BufferUsageFlagBits::eStorageBuffer) bufferBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
                else if constexpr (usageT == vk::BufferUsageFlagBits::eUniformBuffer) bufferBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
                bufferBinding.descriptorCount = 1;
                bufferBinding.stageFlags = shaderStage;
                bufferBinding.pImmutableSamplers = nullptr;

                return bufferBinding;
            }
            else
            {
                Logger::Exception("Cannot bind buffer of type: {} via a descriptor set.", usageT);
            }
        }

        vk::DescriptorBufferInfo getDescriptorInfo()
        {
            if constexpr (
                usageT == vk::BufferUsageFlagBits::eStorageBuffer ||
                usageT == vk::BufferUsageFlagBits::eUniformBuffer
            )
            {
                vk::DescriptorBufferInfo bufferInfo{
                buffer,
                0,
                size
                };

                return bufferInfo;
            }
            else
            {
                Logger::Exception("Cannot bind buffer of type: {} via a descriptor set.", usageT);
            }
        }

        inline void BufferConsumeImpl(Buffer<T, usageT, propertiesT>* source, size_t newDataSize)
        {
            source->moveToGPU();
            
            allocateOnGPU();

            copyBuffer(source->buffer, buffer, size, {0, 0, source->size}, engine);

            updateGPUData({ source->size, source->size, newDataSize * sizeof(T) });
            
            source->freeFromGPU();
        }

        inline void BufferConstructorImpl(Engine* _engine, AssetUsage _assetUsage)
        {
            assert(_engine != nullptr);
            assert(_engine->_transferFence() != nullptr);

            engine = _engine;
            transferFence = engine->_transferFence();

            size = data.size() * sizeof(T);

            memProperties = engine->_physicalDevice().getMemoryProperties();

            vk::BufferCreateInfo bufferCreateInfo(
                {},                      // Flags (default: none)
                size,              // Size of the buffer
                usageT | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc, // Usage flags
                vk::SharingMode::eExclusive // Sharing mode
            );

            buffer.assignRes(engine->_device().createBuffer(bufferCreateInfo), engine->_device());

            memRequirements = engine->_device().getBufferMemoryRequirements(buffer.getRes());

            memoryTypeIndex = findMemoryTypeIndex(memProperties, memRequirements, properties);

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Creating a buffer which will use a staging buffer for copy operations.");

                vk::BufferCreateInfo stagingBufferCreateInfo{
                    {},
                    size,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    vk::SharingMode::eExclusive
                };

                stagingBuffer.assignRes(engine->_device().createBuffer(stagingBufferCreateInfo), engine->_device());

                stagingMemRequirements = engine->_device().getBufferMemoryRequirements(stagingBuffer.getRes());

                stagingMemoryTypeIndex = findMemoryTypeIndex(memProperties, stagingMemRequirements, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                if (stagingMemoryTypeIndex == uint32_t(-1))
                {
                    Logger::Exception("Failed to find the memory type to create a staging buffer");
                }
            }

            if (memoryTypeIndex == uint32_t(-1)) 
            {
                Logger::Exception("Failed to find suitable memory type to create buffer");
            }
        }

        Buffer(const std::vector<T>& _data, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            assert(_data.size() != 0);

            data = _data;

            BufferConstructorImpl(_engine, _assetUsage);
        }

        Buffer(std::vector<T>&& _data, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            assert(_data.size() != 0);

            data = std::move(_data);

            BufferConstructorImpl(_engine, _assetUsage);
        }

        Buffer(const T* _data, size_t _size, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            assert(_size != 0);
            assert(_data != nullptr);

            size = _size * sizeof(T);

            data.resize(size);

            std::memcpy(data.data(), _data, size);

            BufferConstructorImpl(_engine, _assetUsage);
        }

        //Consuming constructor, the old (smaller) buffer is copied into the new one so that we get the "growing" effect. After this constuctor the buffer will be on GPU. THE OLD BUFFER IS UNUSABLE.
        Buffer(Buffer<T, usageT, propertiesT>& source, const std::vector<T>& newData, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source.data);

            data.reserve(data.size() + newData.size());

            data.insert(data.end(), newData.begin(), newData.end());

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(&source, newData.size());
        }

        Buffer(Buffer<T, usageT, propertiesT>& source, std::vector<T>&& newData, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source.data);

            data.reserve(data.size() + newData.size());

            data.insert(data.end(), std::make_move_iterator(newData.begin()), std::make_move_iterator(newData.end()));

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(&source, newData.size());
        }

        Buffer(Buffer<T, usageT, propertiesT>& source, T* newData, size_t newDataSize, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source.data);

            data.resize(data.size() + newDataSize);

            memcpy(data.data() + source.size, newData, newDataSize * sizeof(T));

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(&source, newDataSize);
        }

        Buffer(Buffer<T, usageT, propertiesT>* source, const std::vector<T>& newData, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source->data);

            data.reserve(data.size() + newData.size());

            data.insert(data.end(), newData.begin(), newData.end());

            //this should work for now.
            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(source, newData.size());
        }

        Buffer(Buffer<T, usageT, propertiesT>* source, std::vector<T>&& newData, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source->data);

            data.reserve(data.size() + newData.size());

            data.insert(data.end(), std::make_move_iterator(newData.begin()), std::make_move_iterator(newData.end()));

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(source, newData.size());
        }

        Buffer(Buffer<T, usageT, propertiesT>* source, T* newData, size_t newDataSize, Engine* _engine, AssetUsage _assetUsage = AssetUsage::Undefined) : Asset(_assetUsage, _engine)
        {
            data = std::move(source->data);

            data.resize(data.size() + newDataSize);

            memcpy(data.data() + source->size, newData, newDataSize * sizeof(T));

            BufferConstructorImpl(_engine, _assetUsage);

            BufferConsumeImpl(source, newDataSize);
        }

        void allocateOnGPU()
        {
            if(allocatedOnGPU) return;

            Logger::Log("Allocating the buffer.");

            engine->_device().waitIdle();
            
            vk::MemoryAllocateInfo allocInfo(
                memRequirements.size,    // Allocation size
                memoryTypeIndex          // Memory type index
            );

            memory = engine->_device().allocateMemory(allocInfo);

            engine->_device().bindBufferMemory(buffer.getRes(), memory, 0);

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Allocating a staging buffer");

                vk::MemoryAllocateInfo stagingAllocInfo(
                    memRequirements.size,    // Allocation size
                    stagingMemoryTypeIndex          // Memory type index
                );

                stagingMemory = engine->_device().allocateMemory(stagingAllocInfo);

                engine->_device().bindBufferMemory(stagingBuffer.getRes(), stagingMemory, 0);
            }

            allocatedOnGPU = true;
        }

        template<UpdateMode updateModeT = UpdateMode::Immediate>
        void moveToGPU()
        {
            if(onGPU) return;

            onGPU = true;

            allocateOnGPU();

            Logger::Log("Moving the buffer.");

            engine->_device().waitIdle();

            if constexpr (!CPUAccessible)
            {
                void* dataRaw = engine->_device().mapMemory(stagingMemory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);

                engine->_device().unmapMemory(stagingMemory);

                copyBuffer<updateModeT>(stagingBuffer.getRes(), buffer.getRes(), size, engine);
            }
            else
            {
                Logger::Log("Using a direct copy");

                void* dataRaw = engine->_device().mapMemory(memory, 0, size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
                engine->_device().unmapMemory(memory);
            }
        }

        inline void freeFromGPU()
        {
            if(!allocatedOnGPU) return;

            onGPU = false;
            allocatedOnGPU = false;

            Logger::Log("Freeing the buffer.");

            engine->_device().waitIdle();

            engine->_device().freeMemory(memory);
            if constexpr (!CPUAccessible) engine->_device().freeMemory(stagingMemory);
        }

    private:
        template<UpdateMode updateModeT = UpdateMode::Immediate>
        inline void updateGPUData(vk::BufferCopy updateRegion)
        {
            bool wasOnGPU = onGPU;

            moveToGPU<updateModeT>();

            if constexpr (!CPUAccessible)
            {
                Logger::Log("Using a staging buffer");

                void* dataRaw = engine->_device().mapMemory(stagingMemory, updateRegion.dstOffset, updateRegion.size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(
                    dataTyped, 
                    reinterpret_cast<std::byte*>(data.data()) + updateRegion.srcOffset, 
                    updateRegion.size
                );
                engine->_device().unmapMemory(stagingMemory);

                copyBuffer<updateModeT>(stagingBuffer.getRes(), buffer.getRes(), size, updateRegion, engine);
            }
            else
            {
                Logger::Log("Using a direct copy");

                void* dataRaw = engine->_device().mapMemory(memory, updateRegion.dstOffset, updateRegion.size);
                T* dataTyped = reinterpret_cast<T*>(dataRaw);
                std::memcpy(
                    dataTyped, 
                    reinterpret_cast<std::byte*>(data.data()) + updateRegion.srcOffset, 
                    updateRegion.size
                );
                engine->_device().unmapMemory(memory);
            }

            const UpdateMode templateMode = updateModeT;

            if constexpr (updateModeT == UpdateMode::Immediate) if(!wasOnGPU) freeFromGPU();
        }

        inline void recordUpdate(const vk::BufferCopy& updateRegion)
        {
            if(updateOptimizer.commands.empty()) [[unlikely]]
            {
                updateOptimizer.commands.emplace_back(updateRegion.srcOffset, updateRegion.dstOffset, updateRegion.size);

                return;
            }
            auto& prev = updateOptimizer.commands.back();
            if(prev.dstOffset + prev.updateSize == updateRegion.dstOffset)
            {
                prev.updateSize += updateRegion.size;
            }
            else
            {
                updateOptimizer.commands.emplace_back(updateRegion.srcOffset, updateRegion.dstOffset, updateRegion.size);
            }
        }
    public:

        inline void setUpdateMode(UpdateMode _updateMode)
        {
            updateMode = _updateMode;
        }

        inline void beginUpdateRecording()
        {
            preRecordingUpdateMode = updateMode;
            updateMode = UpdateMode::Recording;

            updateOptimizer.commands.clear();
            updateOptimizer.commands.reserve(128);
        }

        void executeRecordedUpdates()
        {
            bool wasOnGPU = onGPU;

            updateMode = preRecordingUpdateMode;

            vk::Result discardResult = engine->_device().waitForFences(engine->_transferFence(), true, UINT64_MAX);
            discardResult = engine->_device().resetFences(1, &engine->_transferFence());

            vk::CommandBufferBeginInfo beginInfo{};
            engine->_mainCommandBuffer().begin(beginInfo);

            for(int i = 0; i < updateOptimizer.commands.size(); i++)
            {
                updateGPUData<UpdateMode::Recording>({ updateOptimizer.commands[i].srcOffset, updateOptimizer.commands[i].dstOffset, updateOptimizer.commands[i].updateSize });
            }

            vk::BufferMemoryBarrier barrier{
                vk::AccessFlagBits::eTransferWrite,
                vk::AccessFlagBits::eShaderRead,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                buffer,
                0,
                size
            };

            engine->_mainCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eVertexShader,
                {},
                nullptr,
                barrier,
                nullptr
            );

            engine->_mainCommandBuffer().end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.waitSemaphoreCount = 0;
            submitInfo.pWaitSemaphores = nullptr;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &engine->_mainCommandBuffer();

            discardResult = engine->_transferQueue().submit(1, &submitInfo, engine->_transferFence());

            if(!wasOnGPU) freeFromGPU();
        }

        void update(const std::vector<T>& _data)
        {
            assert(_data.size() == data.size());

            data = _data;

            updateGPUData({ 0, 0, size });
        }

        void update(const std::vector<T>&& _data)
        {
            assert(_data.size() == data.size());

            data = std::move(_data);

            updateGPUData({ 0, 0, size });
        }

        void update(const T* _data, size_t _size)
        {
            assert(data.size() == _size);

            std::memcpy(data.data(), _data, data.size());

            updateGPUData({ 0, 0, size });
        }

        void update(const T* _data, vk::BufferCopy updateRegion)
        {
            assert(_data != nullptr);

            std::memcpy(
                reinterpret_cast<std::byte*>(data.data()) + updateRegion.dstOffset, 
                _data + updateRegion.srcOffset, 
                updateRegion.size
            );

            if(updateMode == UpdateMode::Immediate) updateGPUData(updateRegion);
            else recordUpdate(updateRegion);
        }

        void destroy()
        {
            assert(!destroyed);

            destroyed = true;

            engine->_device().waitIdle();

            freeFromGPU();

            buffer.destroy();
            if constexpr (!CPUAccessible)
            {
                stagingBuffer.destroy();
            }
        }

        inline ~Buffer()
        {
            destroy();
        }
    };
}