#include "BlockAllocator.hpp"

namespace nihil 
{
    template<typename T>
    BlockAllocator<T>::BlockAllocator()
    {
        slabs.emplace_back((std::byte*)std::malloc(sizeof(T) * slabSize), 0);
    }

    template<typename T>
    BlockAllocator<T>::~BlockAllocator()
    {
        for(const auto& s : slabs)
        {
            std::free(s.first);
        }
    }

    template<typename T>
    T* BlockAllocator<T>::allocate()
    {
        if(!freeList.empty())
        {
            std::byte* ptr = *freeList.begin();
            freeList.erase(ptr);
            return (T*)ptr;
        }
        else
        {
            if(slabs.back().second >= slabSize)
            {
                slabs.emplace_back((std::byte*)std::malloc(sizeof(T) * slabSize), 0);
                std::byte* ptr = slabs.back().first + (sizeof(T) * slabs.back().second);
                slabs.back().second++;
                return (T*)ptr;
            }
            else 
            {
                std::byte* ptr = slabs.back().first + (sizeof(T) * slabs.back().second);
                slabs.back().second++;
                return (T*)ptr;
            }
        }
    }

    template<typename T>
    void BlockAllocator<T>::free(std::byte* block)
    {
        static_cast<T*>(block)->~T();
        freeList.insert(block);
    }
}