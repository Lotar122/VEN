#pragma once

#include <vector>
#include <unordered_set>
#include <memory>

namespace nihil
{
    //make it free empty slabs later
    template<typename T>
    class BlockAllocator
    {
    public:
        std::vector<std::pair<std::byte*, size_t>> slabs;
        std::unordered_set<std::byte*> freeList;
        static constexpr size_t slabSize = 1024;

        BlockAllocator();

        ~BlockAllocator();

        T* allocate();

        void free(std::byte* block);
    };
}

#include "BlockAllocator.tpp"