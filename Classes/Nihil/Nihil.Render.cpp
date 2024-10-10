#include "Nihil.hpp"
#include <mutex>

void nihil::Nihil::queueDrawObject(engine::Object* object)
{
	engine::WorkCommand command = {};
	command.commandType = engine::CommandType::QueueDraw;
	engine::QueueDrawCommandData* drawData = commandDataManager.allocate<engine::QueueDrawCommandData>();
	drawData->object = object;
	command.data = (void*)drawData;

	drawWorkCommandPool.push_back(command);
}

namespace nihil::utils
{
	void drawWorkProcessor(
		int i,
		std::vector<std::string>* names,
		std::unordered_set<uint64_t>* pipelines,
		std::mutex* namesMutex,
		std::mutex* instancedDrawMutex,
		std::mutex* pipelinesMutex,
		nihil::Nihil* nihilO
	)
	{
		nihil::engine::QueueDrawCommandData* data;
		data = (nihil::engine::QueueDrawCommandData*)nihilO->drawWorkCommandPool[i].data;
		uint32_t index = names->size();
		data->object->model->index = index;
		auto it = std::find(names->begin(), names->end(), data->object->model->name);
		if (it == names->end())
		{
			namesMutex->lock();
			names->push_back(data->object->model->name);
			namesMutex->unlock();
		}
		else
		{
			index = std::distance(names->begin(), it);
		}

		uint32_t* duo = new uint32_t[2];
		duo[0] = data->object->model->instancedPipeline;
		duo[1] = index;

		pipelinesMutex->lock();
		pipelines->insert(nihilO->encode64(duo));
		pipelinesMutex->unlock();
		delete[] duo;
	}

	void drawCommandProcessor(
		nihil::engine::WorkCommand* wc,
		std::vector<std::string>* names,
		std::unordered_map<uint64_t, std::vector<nihil::engine::Object*>>* instancedDraw,
		std::mutex* namesMutex,
		std::mutex* instancedDrawMutex,
		std::mutex* pipelinesMutex,
		nihil::Nihil* nihilO
	)
	{
		nihil::engine::QueueDrawCommandData data = {};
		data = *(nihil::engine::QueueDrawCommandData*)wc->data;

		uint32_t index = 0;
		for (int i = 0; i < names->size(); i++)
		{
			if (data.object->model->name == (*names)[i]) { index = i; break; }
		}

		uint32_t* duo = new uint32_t[2];
		duo[0] = data.object->model->instancedPipeline;
		duo[1] = index;

		uint64_t key = nihilO->encode64(duo);

		nihilO->decode64(key, duo);

		delete[] duo;

		auto x = instancedDraw->find(key);

		instancedDrawMutex->lock();
		x->second.push_back(data.object);
		instancedDrawMutex->unlock();
	}
}

void nihil::Nihil::executeDraws(graphics::Camera& camera)
{
	//check for objects with the same instancePipelines
	std::unordered_map<uint64_t, std::vector<engine::Object*>> instancedDraw;
	std::unordered_set<uint64_t> pipelines;
	std::vector<std::string> names;

	std::mutex namesMutex;
	std::mutex instancedDrawMutex;
	std::mutex pipelinesMutex;
	std::vector<std::thread*> workers;

	for (int i = 0; i < drawWorkCommandPool.size(); i++)
	{
		//std::thread* w = new std::thread(
		//	utils::drawWorkProcessor, 
		//	i, 
		//	&names, 
		//	&pipelines, 
		//	&namesMutex, 
		//	&instancedDrawMutex, 
		//	&pipelinesMutex,
		//	this
		//);
		////w->join();
		//workers.push_back(w);
		utils::drawWorkProcessor(i, &names, &pipelines, &namesMutex, &instancedDrawMutex, &pipelinesMutex, this);
	}
	for (auto w : workers)
	{
		if(w->joinable()) w->join();
		delete w;
	}
	workers.clear();
	for (auto& p : pipelines)
	{
		std::vector<engine::Object*> empty;
		empty.reserve(50);
		if (instancedDraw.find(p) == instancedDraw.end())
		{
			instancedDraw.insert(std::make_pair(p, empty));
		}
	}

	for (engine::WorkCommand& wc : drawWorkCommandPool)
	{
		/*std::thread* w = new std::thread(
			utils::drawCommandProcessor, 
			&wc, 
			&names, 
			&instancedDraw, 
			&namesMutex, 
			&instancedDrawMutex, 
			&pipelinesMutex,
			this
		);
		workers.push_back(w);*/

		utils::drawCommandProcessor(&wc, &names, &instancedDraw, &namesMutex, &instancedDrawMutex, &pipelinesMutex, this);
	}
	/*for (auto w : workers)
	{
		if (w->joinable()) w->join();
		delete w;
	}
	workers.clear();*/
	std::vector<graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*> instanceBuffers;
	for (auto& x : instancedDraw)
	{
		//prepare instance buffers

		std::vector<float> instanceData;

		uint32_t* duo = new uint32_t[2];
		decode64(*const_cast<uint64_t*>(&x.first), duo);

		delete[] duo;

		for (auto& obj : x.second)
		{
			std::vector<float> y = nstd::flattenMatrix4x4(obj->renderingData);
			for (float& x : y)
			{
				instanceData.push_back(x);
			}
		}
		graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>* buffer =
			(graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>*)instanceBufferManager.allocate();

		buffer = new (buffer) graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer>(engine, instanceData);
		//graphics::Buffer<float, vk::BufferUsageFlagBits::eVertexBuffer> h = *buffer;

		//!!! uses more ram and vram every frame. make sure to make manager for these buffers to delete them after every frame. (done) still leaks memory elsewhere
		//engine->registerObjectForDeletion(buffer);
		//now allocated to the memory arena

		int y = 0;

		//add the possibillity for the elements to be sorted, as they were on the entry, so in the work command vector
		engine->queueInstancedDraw(x.second[0]->model, buffer, x.second[0]->model->instancedPipeline);

		int i = 0;

		//submit draw commands

		// !!! make sure to add checks that the models are the same before doing instanced draw
	}

	//check for objects with the same pipelines and print an optimization hint

	//the end list iterator happens here !!! happy happy happy

	//at the end delete all command data
	engine->Draw(camera);
	commandDataManager.reset();
	instanceBufferManager.reset();
	drawWorkCommandPool.clear();
}