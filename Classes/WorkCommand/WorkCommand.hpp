#pragma once

#include <iostream>
#include "nihil-render/nihil.hpp"
#include "Classes/Object/Object.hpp"

namespace nihil::engine {
	enum class CommandType {
		QueueDraw
	};
	struct WorkCommand {
		CommandType commandType;
		void* data;
	};

	struct QueueDrawCommandData {
		graphics::DrawCommand drawCommand;
		Object* object;
	};
};