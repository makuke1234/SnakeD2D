#ifndef COMMON_H

#define COMMON_H


#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define UNICODE
#define _UNICODE

#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <Windows.h>

#endif

namespace snake
{
	template<class Interface>
	void SafeRelease(Interface *& ppInstance)
	{
		if (ppInstance != nullptr) [[likely]]
		{
			ppInstance->Release();
			ppInstance = nullptr;
		}
	}
}
