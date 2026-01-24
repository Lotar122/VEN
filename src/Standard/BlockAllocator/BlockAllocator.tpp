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
    template<typename... Args>
    T* BlockAllocator<T>::allocate(Args&&... args)
    {
        if(!freeList.empty())
        {
            T* ptr = reinterpret_cast<T*>(*freeList.begin());
            freeList.erase(reinterpret_cast<std::byte*>(ptr));

            ptr = new (ptr) T(std::forward<Args>(args)...);

            return std::launder(ptr);
        }
        else
        {
            if(slabs.back().second >= slabSize)
            {
                slabs.emplace_back((std::byte*)std::malloc(sizeof(T) * slabSize), 0);
                T* ptr = reinterpret_cast<T*>(slabs.back().first + (sizeof(T) * slabs.back().second));
                slabs.back().second++;

                ptr = new (ptr) T(std::forward<Args>(args)...);

                return std::launder(ptr);
            }
            else 
            {
                T* ptr = reinterpret_cast<T*>(slabs.back().first + (sizeof(T) * slabs.back().second));
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
        freeList.insert(reinterpret_cast<std::byte*>(block));
    }
}