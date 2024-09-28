#pragma once

namespace nihil::nstd {
	template<typename T>
	inline void* _voidp(T* x)
	{
		return (void*)x;
	}
}