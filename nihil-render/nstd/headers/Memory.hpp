#pragma once

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <typeindex>

namespace nihil::nstd
{
	class MemoryArena
	{
		uint64_t size = 1024;

		void* mem = NULL;
		uint64_t offset = 0;

	public:

		MemoryArena(uint64_t _size = 1024)
		{
			size = _size;
			mem = std::malloc(size);
		}

		template<typename T>
		T* allocate()
		{
			uint64_t objectSize = sizeof(T);
			if (!(offset + objectSize < size)) { throw std::exception("Arena out of memory"); return NULL; }
			uint64_t oldOffset = offset;
			offset += objectSize;
			return reinterpret_cast<T*>((char*)mem + oldOffset);
		}

		void* allocate(uint64_t objectSize)
		{
			if (!(offset + objectSize < size)) { throw std::exception("Arena out of memory"); return NULL; }
			uint64_t oldOffset = offset;
			offset += objectSize;
			return reinterpret_cast<void*>((char*)mem + oldOffset);
		}

		void reset()
		{
			offset = 0;
		}

		void free()
		{
			std::free(mem);
			mem = NULL;
		}

		~MemoryArena()
		{
			if (mem != NULL) free();
		}
	};

	class MemoryPool
	{
		uint64_t numBlocks = 16;
		uint64_t blockSize = 4;

		uint64_t offset = 0;

		void* mem = NULL;

	public:
		MemoryPool(uint64_t _blockSize = 4, uint64_t _numBlocks = 16)
		{
			numBlocks = _numBlocks;
			blockSize = _blockSize;

			mem = std::malloc(blockSize * numBlocks);
		}

		void* allocate()
		{
			if (offset + blockSize > numBlocks * blockSize) { throw std::exception("Memory pool out of memory"); return NULL; }
			uint64_t oldOffset = offset;
			offset += blockSize;
			return reinterpret_cast<void*>((char*)mem + oldOffset);
		}

		void reset()
		{
			offset = 0;
		}

		void free()
		{
			std::free(mem);
			mem = NULL;
		}

		~MemoryPool()
		{
			if (mem != NULL) free();
		}
	};

	using DestructorFunction = void(*)(void*);

	template<typename T>
	class ObjectPool
	{
		DestructorFunction des;

		void* mem = NULL;

		uint64_t blockSize = 0;
		uint64_t numBlocks = 0;

		uint64_t offset = 0;

		std::vector<T*> objects;

	public:
		ObjectPool(DestructorFunction _des, uint64_t _blockSize, uint64_t _numBlocks)
		{
			blockSize = _blockSize;
			numBlocks = _numBlocks;

			des = _des;

			mem = std::malloc(blockSize * numBlocks);

			objects.reserve(numBlocks);
		}

		T* allocate()
		{
			if (offset + blockSize > blockSize * numBlocks) { throw std::exception("Object pool out of memory"); return NULL; }
			uint64_t oldOffset = offset;
			offset += blockSize;
			void* res = reinterpret_cast<void*>((char*)mem + oldOffset);

			objects.push_back((T*)res);

			return (T*)res;
		}

		void reset()
		{
			if (offset == 0) return;
			for (T*& ptr : objects)
			{
				des(reinterpret_cast<void*>(ptr));
			}

			objects.clear();
			objects.reserve(numBlocks);
			offset = 0;
		}

		void free()
		{
			reset();
			std::free(mem);
			mem = NULL;
		}

		~ObjectPool()
		{
			if (mem != NULL) free();
		}
	};

	//combined size
	template<typename... Types>
	class CombinedSize;

	template<typename First, typename... Rest>
	class CombinedSize<First, Rest...>
	{
	public:
		static constexpr std::size_t value = sizeof(First) + CombinedSize<Rest...>::value;
	};

	template<>
	class CombinedSize<>
	{
	public:
		static constexpr std::size_t value = 0;
	};
}