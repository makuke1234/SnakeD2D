#pragma once


#include "common.h"

#include <d2d1.h>
#include <dwrite.h>
#include <mmsystem.h>

#include <string_view>
#include <vector>
#include <cstdint>

#ifdef _DEBUG
	#include <cstdio>
	#define dp(...) std::printf(__VA_ARGS__)
#else
	#define dp(...)
#endif

// Direct2D type aliases
namespace d2d
{
	using factory = ID2D1Factory;
	using hwndRT = ID2D1HwndRenderTarget;
	using solidBrush = ID2D1SolidColorBrush;
	using bm = ID2D1Bitmap;
	using bmBrush = ID2D1BitmapBrush;

	using f = FLOAT;
	using u32 = UINT32;

	using sF = D2D1_SIZE_F;
	using sU = D2D1_SIZE_U;

	using rF = D2D1_RECT_F;
	using rU = D2D1_RECT_U;


}

namespace dwrite
{
	using factory = IDWriteFactory;
	using txtFormat = IDWriteTextFormat;

}

namespace snake
{
	template<class Interface>
	void safeRelease(Interface *& ppInstance)
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
	template<class Int>
		requires std::is_integral_v<Int>
	[[nodiscard]] constexpr bool enumIsClose(Int in, Int limit) noexcept
	{
		return (in == 1 || in == (limit - 1));
	}

	void playSndRsc(std::uint16_t resource, HINSTANCE hInst) noexcept;
	void playSndRscAsync(std::uint16_t resource, HINSTANCE hInst) noexcept;
	void stopSndRscAsync() noexcept;
}
