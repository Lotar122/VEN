#include "Nihil.hpp"

void deleteQueueDrawCommandData(void* x)
{
	delete (nihil::engine::QueueDrawCommandData*)x;
}