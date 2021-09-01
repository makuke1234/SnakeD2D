#pragma once


#include "common.h"

#include <d2d1.h>
#include <dwrite.h>

#include <string_view>
#include <vector>

#ifdef _DEBUG
	#include <cstdio>
	#define dp(...) std::printf(__VA_ARGS__)
#else
	#define dp(...)
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
