#include "Nihil.hpp"

void nihil::Nihil::queueDrawObject(engine::Object* object)
{
	nstd::PtrManagerDeleteFunction fn = deleteQueueDrawCommandData;
	commandDataManager.registerType(typeid(engine::QueueDrawCommandData*), fn);

	engine::WorkCommand command = {};
	command.commandType = engine::CommandType::QueueDraw;
	engine::QueueDrawCommandData* drawData = new engine::QueueDrawCommandData();
	drawData->object = object;
	command.data = (void*)drawData;

	commandDataManager.addPointer(typeid(engine::QueueDrawCommandData*), (void*)drawData);

	std::cout << "one" << std::endl;
	std::cout << ((engine::QueueDrawCommandData*)(command.data))->object->model->name.size() << "Lenght of pointer name passed" << std::endl;

	drawWorkCommandPool.push_back(command);

	std::cout << "two" << std::endl;
	std::cout << ((engine::QueueDrawCommandData*)(drawWorkCommandPool[0].data))->object->model->name.size() << std::endl;
}

void nihil::Nihil::executeDraws()
{
	//check for objects with the same instancePipelines
	std::unordered_map<uint64_t, std::vector<engine::Object*>> instancedDraw;
	std::unordered_set<uint64_t> pipelines;
	std::vector<std::string> names;
	std::cout << "start1" << std::endl;
	std::cout << ((engine::QueueDrawCommandData*)(drawWorkCommandPool[0].data))->object->model->name.size() << std::endl;
	for (int i = 0; i < drawWorkCommandPool.size(); i++)
	{
		engine::QueueDrawCommandData* data;
		data = (engine::QueueDrawCommandData*)drawWorkCommandPool[i].data;

		uint32_t index = names.size();
		std::cout << data->object->model->name.size() << "hemlo" << std::endl;
		data->object->model->index = index;
		auto it = std::find(names.begin(), names.end(), data->object->model->name);
		if (it == names.end())
		{
			names.push_back(data->object->model->name);
		}
		else
		{
			index = std::distance(names.begin(), it);
		}

		std::cout << data->object->model->name.size() << std::endl;

		uint32_t* duo = new uint32_t[2];
		duo[0] = data->object->model->instancedPipeline;
		duo[1] = index;

		std::cout << "Init array of: " << duo[0] << " " << duo[1] << ", so for a model with the name of: " << names[duo[1]] << std::endl;
		pipelines.insert(encode64(duo));
		delete[] duo;
	}
	std::cout << "start2" << std::endl;
	for (auto& p : pipelines)
	{
		std::vector<engine::Object*> empty;
		empty.reserve(50);
		if (instancedDraw.find(p) == instancedDraw.end())
		{
			instancedDraw.insert(std::make_pair(p, empty));
		}
	}
	std::cout << "start3" << std::endl;
	for (engine::WorkCommand& wc : drawWorkCommandPool)
	{
		engine::QueueDrawCommandData data = {};
		data = *(engine::QueueDrawCommandData*)wc.data;

		uint32_t index = 0;
		for (int i = 0; i < names.size(); i++)
		{
			if (data.object->model->name == names[i]) { index = i; break; }
		}

		uint32_t* duo = new uint32_t[2];
		duo[0] = data.object->model->instancedPipeline;
		duo[1] = index;

		uint64_t key = encode64(duo);

		std::cout << "Init array of: " << duo[0] << " " << duo[1] << ", so for a model with the name of: " << names[index] << std::endl;

		decode64(key, duo);
		std::cout << "Init array of (decode): " << duo[0] << " " << duo[1] << ", so for a model with the name of: " << names[index] << std::endl;

		delete[] duo;

		auto x = instancedDraw.find(key);

		x->second.push_back(data.object);

		std::cout << "size of smth: " << x->second.size() << std::endl;
	}
	for (auto& x : instancedDraw)
	{
		std::cout << x.second.size() << std::endl;
	}
	std::cout << "start4" << std::endl;
	std::vector<graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*> instanceBuffers;
	for (auto& x : instancedDraw)
	{
		//prepare instance buffers

		std::vector<float> instanceData;

		uint32_t* duo = new uint32_t[2];
		decode64(*const_cast<uint64_t*>(&x.first), duo);

		std::cout << "Init array of (decode): " << duo[0] << " " << duo[1] << ", so for a model with the name of: " << names[duo[1]] << std::endl;

		delete[] duo;
		for (auto& obj : x.second)
		{
			std::cout << "Doing the flattening to: " << obj->model->name << std::endl;
			std::vector<float> y = nstd::flattenMatrix4x4(obj->renderingData);
			for (float& x : y)
			{
				instanceData.push_back(x);
			}
		}
		std::cout << instanceData.size() << " asdyfg" << std::endl;
		graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* buffer =
			new graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(
				engine, instanceData
			);
		engine->registerObjectForDeletion(buffer);

		//add the possibillity for the elements to be sorted, as they were on the entry, so in the work command vector
		engine->queueInstancedDraw(x.second[0]->model, buffer, x.second[0]->model->instancedPipeline);

		//submit draw commands

		// !!! make sure to add checks that the models are the same before doing instanced draw
	}

	std::cout << instancedDraw.size() << " Lenght of instancedDraws" << std::endl;

	//check for objects with the same pipelines and print an optimization hint

	//at the end delete all command data
	std::vector<nihil::nstd::Component> x;
	engine->Draw(x);
	commandDataManager.deleteAll();
	drawWorkCommandPool.clear();
}