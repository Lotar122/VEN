namespace nihil 
{
    template<typename T>
    BlockAllocator<T>::BlockAllocator()
    {
        slabs.emplace_back(
            static_cast<T*>(::operator new(
                sizeof(T) * slabSize,
                std::align_val_t{alignof(T)}
            )),
            0
        );
    }

    template<typename T>
    BlockAllocator<T>::~BlockAllocator()
    {
        for(const auto& s : slabs)
        {
            ::operator delete(s.first, std::align_val_t{alignof(T)});
        }
    }

    template<typename T>
    template<typename... Args>
    T* BlockAllocator<T>::allocate(Args&&... args)
    {
        if(!freeList.empty())
        {
            T* ptr = *freeList.begin();
            freeList.erase(ptr);

            ptr = new (ptr) T(std::forward<Args>(args)...);

            return std::launder(ptr);
        }
        else
        {
            if(slabs.back().second >= slabSize)
            {
                slabs.emplace_back(
                    static_cast<T*>(::operator new(
                        sizeof(T) * slabSize,
                        std::align_val_t{alignof(T)}
                    )),
                    0
                );
                T* ptr = slabs.back().first + slabs.back().second;
                slabs.back().second++;

                ptr = new (ptr) T(std::forward<Args>(args)...);

                return std::launder(ptr);
            }
            else [[likely]]
            {
                T* ptr = slabs.back().first + slabs.back().second;
                slabs.back().second++;

                ptr = new (ptr) T(std::forward<Args>(args)...);

                return std::launder(ptr);
            }
        }
    }

    template<typename T>
    void BlockAllocator<T>::free(T* block)
    {
        block->~T();
        freeList.insert(block);
    }
}