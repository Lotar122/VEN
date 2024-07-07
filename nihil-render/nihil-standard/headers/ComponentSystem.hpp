#pragma once
#include <vector>
#include "Callable.hpp"
namespace nihil::nstd {
	union ComponentData {
		void* any;
	};
	enum class ComponentType {
		any = 0
	};

	struct CustomDataHandleInput {
		ComponentType type;
		ComponentData data;
		size_t dataSize;
		std::type_info* typeID;
	};

	struct CustomDataHandleOutput {
		ComponentType type;
		ComponentData data;
		size_t dataSize;
		std::type_info* typeID;
	};

	class Component {
		using functionTemplate = CustomDataHandleOutput (*)(CustomDataHandleInput);
	public:
		Callable <CallableTemplate<CustomDataHandleInput, CustomDataHandleOutput, functionTemplate>> customDataGetHandle;
		Callable <CallableTemplate<CustomDataHandleInput, CustomDataHandleOutput, functionTemplate>> customDataSetHandle;

		ComponentData data;
		ComponentType type;
		size_t dataSize;
		std::type_info* typeID;

		Component(ComponentData _data, ComponentType _type, size_t _dataSize, const std::type_info* _typeID)
		{
			data = _data;
			type = _type;
			dataSize = _dataSize;
			typeID = const_cast<std::type_info*>(_typeID);

			customDataGetHandle.function = [](nihil::nstd::CustomDataHandleInput in) {
				nihil::nstd::CustomDataHandleOutput out = {};
				out.data = in.data;
				out.type = in.type;
				out.dataSize = in.dataSize;
				out.typeID = in.typeID;
				return out;
			};

			customDataSetHandle.function = [](nihil::nstd::CustomDataHandleInput in) {
				nihil::nstd::CustomDataHandleOutput out = {};
				out.data = in.data;
				out.type = in.type;
				out.dataSize = in.dataSize;
				out.typeID = in.typeID;
				return out;
			};
		}

		CustomDataHandleOutput get()
		{
			CustomDataHandleInput input = {};
			input.data = data;
			input.type = type;
			input.dataSize = dataSize;
			input.typeID = typeID; 
			return customDataGetHandle.execute(input);
		}

		void set(void* _data)
		{
			data.any = _data;
			CustomDataHandleInput input = {};
			input.data = data;
			input.type = type;
			input.dataSize = dataSize;
			input.typeID = typeID;
			data.any = customDataSetHandle.execute(input).data.any;
		}
	};
}