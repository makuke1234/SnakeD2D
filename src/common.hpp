#pragma once


#include "common.h"

#include <d2d1.h>
#include <dwrite.h>

#include <string_view>
#include <vector>
#include <cstdint>

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

	template<class Enum>
	[[nodiscard]] constexpr std::underlying_type_t<Enum> enumDiff(Enum e1, Enum e2) noexcept
	{
		using T = std::underlying_type_t<Enum>;
		return T(e2) > T(e1) ? (T(e2) - T(e1)) : (T(e1) - T(e2));
	}
	template<class T>
		requires std::is_integral_v<T>
	[[nodiscard]] constexpr bool enumIsClose(T in, T limit) noexcept
	{
		return (in == 1 || in == (limit - 1));
	}
}
