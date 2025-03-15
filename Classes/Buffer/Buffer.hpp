#pragma once

#include "Classes/Engine/Engine.hpp"

namespace nihil::graphics
{
    template<typename T, vk::BufferUsageFlagBits usageT>
    class Buffer
    {
        Engine* engine = nullptr;

        size_t size = 0;
        std::vector<T> data;
        vk::BufferUsageFlagBits usage = usageT;

        vk::MemoryRequirements memRequirements;
        vk::PhysicalDeviceMemoryProperties memProperties;

        uint32_t memoryTypeIndex = uint32_t(-1);

        vk::Buffer buffer;
        vk::DeviceMemory memory;

        bool onGPU = false;

        bool destroyed = false;
    
    public:
        inline vk::Buffer _buffer() const { return buffer; };
        //return the size of the buffer in bytes
        inline size_t _size() const { return size; };
        //returns the size of the buffer in units of its type
        inline size_t _typedSize() const { return data.size(); };

        Buffer(const std::vector<T>& _data, Engine* _engine)
        {
            assert(_data.size() != 0);
            assert(_engine != nullptr);

            engine = _engine;

            data = _data;

            size = data.size() * sizeof(T);

            // Step 1: Create buffer
            vk::BufferCreateInfo bufferCreateInfo(
                {},                      // Flags (default: none)
                size,              // Size of the buffer
                usageT | vk::BufferUsageFlagBits::eTransferDst, // Usage flags
                vk::SharingMode::eExclusive // Sharing mode
            );

            buffer = engine->_device().createBuffer(bufferCreateInfo);

            // Step 2: Get memory requirements
            memRequirements = engine->_device().getBufferMemoryRequirements(buffer);

            // Step 3: Allocate memory
            memProperties = engine->_physicalDevice().getMemoryProperties();

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((memRequirements.memoryTypeBits & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) &&
                    (memProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent)) {
                    memoryTypeIndex = i;
                    break;
                }
            }

            if (memoryTypeIndex == uint32_t(-1)) {
                Logger::Exception("Failed to find suitable memory type to create buffer");
            }     
        }

        void moveToGPU()
        {
            if(onGPU) return;

            onGPU = true;

            Logger::Log("Allocating the buffer.");

            engine->_device().waitIdle();
            
            vk::MemoryAllocateInfo allocInfo(
                memRequirements.size,    // Allocation size
                memoryTypeIndex          // Memory type index
            );

            memory = engine->_device().allocateMemory(allocInfo);

            void* dataRaw = engine->_device().mapMemory(memory, 0, size);
            T* dataTyped = reinterpret_cast<T*>(dataRaw);
            //dum dum. we are copying bytes. not floats. although i wonder if copying via the correct typed pointer would give a speed benefit.
            std::memcpy(dataTyped, reinterpret_cast<T*>(data.data()), size);
            engine->_device().unmapMemory(memory);

            // Step 4: Bind buffer to memory
            engine->_device().bindBufferMemory(buffer, memory, 0);
        }

        inline void freeFromGPU()
        {
            if(!onGPU) return;

            onGPU = false;

            Logger::Log("Freeing the buffer.");

            engine->_device().waitIdle();

            engine->_device().freeMemory(memory);
        }

        void destroy()
        {
            assert(!destroyed);

            destroyed = true;

            engine->_device().waitIdle();

            freeFromGPU();

            engine->_device().destroyBuffer(buffer);
        }

        inline ~Buffer()
        {
            destroy();
        }
    };
}