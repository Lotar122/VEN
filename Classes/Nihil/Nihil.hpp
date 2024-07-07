#pragma once

#include "Classes/Object/Object.hpp"
#include "Classes/WorkCommand/WorkCommand.hpp"
#include "nihil-render/nihil.hpp"

#include <unordered_map>
#include <set>
#include <algorithm>

void deleteQueueDrawCommandData(void* x)
{
	delete (nihil::engine::QueueDrawCommandData*)x;
}

namespace nihil {
	class Nihil 
	{
	public:
		std::vector<engine::Object*> objects;
		std::vector<engine::WorkCommand> workCommandPool;
		std::vector<engine::WorkCommand> drawWorkCommandPool;

		nstd::PtrManagerClass commandDataManager;

		graphics::Engine* engine;

		Nihil(graphics::Engine* _engine)
		{
			engine = _engine;
		}
		Nihil(bool debug = false)
		{
			engine = new graphics::Engine(debug);
		}

		void queueDrawObject(engine::Object* object);
		inline uint64_t encode64(uint32_t* duo) {
			uint64_t _duo = 0;
			_duo |= static_cast<uint64_t>(duo[0]);       // Set the lower 32 bits
			_duo |= static_cast<uint64_t>(duo[1]) << 32; // Set the upper 32 bits
			return _duo;
		}
		inline void decode64(uint64_t _duo, uint32_t* duo)
		{
			duo[0] = static_cast<uint32_t>(_duo & 0xFFFFFFFF);         // Extract lower 32 bits
			duo[1] = static_cast<uint32_t>((_duo >> 32) & 0xFFFFFFFF); // Extract upper 32 bits
		}
		void executeDraws();
	};
}