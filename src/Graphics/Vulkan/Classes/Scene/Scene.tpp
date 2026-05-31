#include <chrono>
namespace nihil::graphics
{
    template<bool _freeList, bool _homeless>
    void Scene::constructSlots(
        size_t instanceDataSize,
        size_t instanceDataChunkSize,
        Scene::ObjectInstance* toRender,
        size_t toRenderSize,
        std::vector<Scene::InstanceDataSlot>& slots, 
        size_t thisFrame, 
        Buffer<std::vector<std::byte>, vk::BufferUsageFlagBits::eVertexBuffer>& instanceBuffer, 
        std::vector<size_t>* __freeList, std::vector<size_t>* __homeless
    )
    {
        if(toRenderSize == 0) [[unlikely]] return;
        using FreeListType = std::conditional_t<_freeList, std::vector<size_t>&, std::vector<size_t>>;
        using HomelessType = std::conditional_t<_homeless, std::vector<size_t>&, std::vector<size_t>>;

        struct InitVec
        {
            static inline FreeListType initFreeList(std::vector<size_t>* p)
            {
                if constexpr (_freeList) {
                    p->clear();
                    return *p;
                }
                else
                {
                    return FreeListType();
                }
            }
            static inline HomelessType initHomeless(std::vector<size_t>* p)
            {
                if constexpr (_homeless) {
                    p->clear();
                    return *p;
                }
                else
                {
                    return HomelessType();
                }
            }
        };

        HomelessType homeless = InitVec::initHomeless(__homeless);
        FreeListType freeList = InitVec::initFreeList(__freeList);

        homeless.reserve(toRenderSize);
        freeList.reserve(slots.size());

        // if(toRender.size() < slots.size())
        // {
        //     slots.erase(slots.end() - (slots.size() - toRender.size()), slots.end());
        // }

        for(size_t i = 0; i < toRenderSize; i++)
        {
            Object* o = toRender[i].object;
            size_t prevDataSlot = o->prevDataSlot;
            if(prevDataSlot != std::numeric_limits<size_t>::max() && prevDataSlot < slots.size()) [[likely]]
            {
                InstanceDataSlot& slot = slots[prevDataSlot];
                if(slot.prevAssignedFrame == thisFrame) [[unlikely]]
                {
                    homeless.push_back(i);
                    continue;
                }

                slot.prevAssignedFrame = thisFrame;
                slot.currentResident = o->_getAssetId();
                slot.currentResidentRenderIndex = i;
                slot.prevWriteFrame = std::numeric_limits<size_t>::max();
            }
            else
            {
                homeless.push_back(i);
            }
        }

        for(size_t i = 0; i < slots.size(); i++)
        {
            if(slots[i].prevAssignedFrame != thisFrame) freeList.push_back(i);
        }

        slots.reserve(slots.size() + homeless.size());

        while(!homeless.empty())
        {
            size_t homelessBack = homeless.back();
            size_t objectId = toRender[homelessBack].object->_getAssetId();
            if(!freeList.empty()) 
            {
                InstanceDataSlot& slot = slots[freeList.back()];
                slot.prevAssignedFrame = thisFrame;
                slot.currentResident = objectId;
                slot.currentResidentRenderIndex = homelessBack;
                slot.prevWriteFrame = std::numeric_limits<size_t>::max();

                toRender[homelessBack].object->prevDataSlot = freeList.back();

                freeList.pop_back();
            }
            else
            {
                slots.emplace_back( objectId, homelessBack, thisFrame );
                toRender[homelessBack].object->prevDataSlot = slots.size() - 1;
            }

            homeless.pop_back();
        }

        //Slot compacting

        size_t toErase = 0;
        for(size_t i = slots.size() - 1; i >= 0; i--)
        {
            if(slots[i].prevAssignedFrame == thisFrame) break;
            toErase++;
        }

        slots.erase(slots.end() - toErase, slots.end());

        while(!freeList.empty())
        {
            if(freeList.back() >= slots.size())
            {
                freeList.pop_back();
                continue;
            }
            slots[freeList.back()] = std::move(slots.back());
            slots.pop_back();
            freeList.pop_back();
        }

        if(!freeList.empty()) Carbo::Logger::Exception("Non-integral slots");

        auto start = std::chrono::high_resolution_clock::now();

        //instanceBuffer.beginUpdateRecording();
        instanceBuffer.moveToGPU();
        instanceBuffer.beginDirectWrite();

        for(size_t i = 0; i < slots.size(); i++)
        {
            InstanceDataSlot& slot = slots[i];
            Object* o = toRender[slot.currentResidentRenderIndex].object;

            bool dirty = o->lastModifiedFrame > slot.prevWriteFrame || slot.prevWriteFrame == std::numeric_limits<size_t>::max() || slot.prevOffset != i || slot.prevOffset == std::numeric_limits<size_t>::max() || slot.lastResident != slot.currentResident;
            //std::cout<<std::format("Dirty: {}, prevOffset: {}, i: {}, lastResident: {}, currentResident: {}, currentResidentRenderIndex: {}, thisFrame: {}\n", dirty, slot.prevOffset, i, slot.lastResident, slot.currentResident, slot.currentResidentRenderIndex, thisFrame);
            if(dirty)
            {
                instanceBuffer.update(reinterpret_cast<const std::byte*>(o->_instanceData().first), vk::BufferCopy{ 0, i * o->_instanceData().second, o->_instanceData().second });
                slot.prevWriteFrame = thisFrame;
            }

            slot.prevOffset = i;
            slot.lastResident = slot.currentResident;
        }

        //instanceBuffer.executeRecordedUpdates();
        instanceBuffer.executeDirectWrites();

        auto end = std::chrono::high_resolution_clock::now();

        std::cout<<std::format("Memory copies took: {}\n", std::chrono::duration_cast<std::chrono::microseconds>(end - start));
    }
}