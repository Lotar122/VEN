#pragma once
#include <typeindex>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <any>

namespace nihil::nstd {
	using PtrManagerDeleteFunction = void (*)(void*);

	class PtrManagerClass 
	{
		struct DUO {
			std::type_index ind = typeid(int);
			void* ptr;

			DUO(std::type_index _ind, void* _ptr)
			{
				ind = _ind;
				ptr = _ptr;
			}
			DUO()
			{
				ind = typeid(int);
				ptr = NULL;
			}
		};
	public:
		std::unordered_map<std::type_index, PtrManagerDeleteFunction> deleteFunctions;
		std::vector<DUO> pointers;

		void registerType(std::type_index type_id, PtrManagerDeleteFunction function)
		{
			deleteFunctions.insert(std::make_pair(type_id, function));
		}

		void addPointer(std::type_index type_id, void* ptr)
		{
			pointers.push_back(DUO(type_id, ptr));
		}

		void deleteAll()
		{
			for (auto& x : pointers)
			{
				auto y = deleteFunctions.find(x.ind);

				y->second(x.ptr);
			}

			pointers.resize(0);
		}

		~PtrManagerClass()
		{
			for (auto& x : pointers)
			{
				auto y = deleteFunctions.find(x.ind);

				y->second(x.ptr);
			}
		}
	};
}