#pragma once

namespace nihil
{
	///The virtual abstract class for the onHandle event listener
	class onHandleListener
	{
	public:
		///The event handler
		virtual void onHandle() = 0;
	};
}