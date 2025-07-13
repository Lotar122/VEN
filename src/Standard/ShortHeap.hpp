#pragma once

#include <cstdlib>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <stdexcept>

/*
The Heap is a memory allocation mechanism for allocating lots of 
very small objects with a short lifetime. 

It works most efficiently when the objects are of the same
size, so add padding to your data so that block reuse is optimal.

! The heap cannot shrink, it can only reuse blocks.
*/

namespace nihil
{
    using byte = unsigned char;

    struct MemoryBlock
    {
        // char padd1[2];
        MemoryBlock* prev = nullptr;
        // char padd2[2];
        MemoryBlock* next = nullptr;
        // char padd3[2];

        size_t size = 0;

        bool free = true;

        byte* data = nullptr;

        size_t realtivePointer = 0;
    };

    class ShortHeap
    {
        size_t size = 0;
        size_t greed = 0;

        size_t currentPointer = 0;

        byte* memory = nullptr;

        std::vector<MemoryBlock> blocks;
        std::vector<MemoryBlock*> freeList;

        std::unordered_map<byte*, MemoryBlock*> pointers;

        MemoryBlock* lastBlock = nullptr;
        MemoryBlock* firstBlock = nullptr;

        void updateBlocks(byte* _memory, MemoryBlock* _block)
        {
            MemoryBlock* block = _block;

            pointers.clear();

            //* block->data = memory + block->realtivePointer;

            while(block != nullptr)
            {
                block->data = memory + block->realtivePointer;
                pointers.insert(std::make_pair(block->data, block));

                block = block->next;
            }
        }

    public:

        ShortHeap(size_t _startSize, size_t _greed)
        {
            size = _startSize;
            greed = _greed;

            memory = (byte*)malloc(size);

            blocks.reserve(size / 2);
            freeList.reserve(size / 2);
        }

        ~ShortHeap()
        {
            free(memory);
        }

        template<typename T>
        T* alloc()
        {
            return reinterpret_cast<T*>(this->salloc(sizeof(T))->data);
        }

        MemoryBlock* salloc(size_t _size)
        {
            if(!freeList.empty())
            {
                for(int i = 0; i < freeList.size(); i++)
                {
                    MemoryBlock* b = freeList[i];

                    if(b->size >= _size) 
                    {
                        b->free = false;

                        freeList.erase(freeList.begin() + i);

                        return b;
                    }
                }
            }

            if(currentPointer + _size > size)
            {
                size_t increase = greed;
                while(increase <= _size) increase += greed;

                memory = (byte*)realloc(memory, size + increase);
                size += increase;

                if(firstBlock)
                {
                    updateBlocks(memory, firstBlock);
                }
            }

            size_t index = blocks.size();

            blocks.push_back({});

            MemoryBlock* b = nullptr;

            b = blocks.data() + index;

            b->data = memory + currentPointer;
            b->size = _size;
            b->prev = lastBlock;
            b->realtivePointer = currentPointer;

            if(lastBlock)
            {
                lastBlock->next = b;
            }

            lastBlock = b;

            if(currentPointer == 0) firstBlock = b;

            currentPointer += _size;

            pointers.insert(std::make_pair(b->data, b));

            return b;
        }

        void free(MemoryBlock* _bp)
        {
            freeList.push_back(_bp);

		    _bp->free = true;
        }

        template<typename T>
        void free(T ptr)
        {
            auto it = pointers.find(reinterpret_cast<byte*>(ptr));
            if(it != pointers.end())
            {
                freeList.push_back(it->second);

                it->second->free = true;
            }
            else throw std::runtime_error("Invalid pointer to be freed");
        }
    };
}