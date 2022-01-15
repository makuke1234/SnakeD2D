#pragma once


#include "common.h"

#include <d2d1.h>
#include <dwrite.h>
#include <mmsystem.h>

#include <string_view>
#include <vector>
#include <cstdint>
#include <cstdlib>

#ifdef _DEBUG
	#include <cstdio>
	#define dp(...) std::printf(__VA_ARGS__)
#else
	#define dp(...)
#endif

// Direct2D type aliases
namespace dx
{
	using Factory = ID2D1Factory;
	using HwndRT = ID2D1HwndRenderTarget;
	using SolidBrush = ID2D1SolidColorBrush;
	using Bmp = ID2D1Bitmap;
	using BmBrush = ID2D1BitmapBrush;

	using F = FLOAT;
	using U32 = UINT32;

	using SzF = D2D1_SIZE_F;
	using SzU = D2D1_SIZE_U;

	using RectF = D2D1_RECT_F;
	using RectU = D2D1_RECT_U;

	using Color = D2D1_COLOR_F;
}

namespace dw
{
	using Factory = IDWriteFactory;
	using TxtFormat = IDWriteTextFormat;

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

	template<class Combined, class Source>
	[[nodiscard]] constexpr Combined combine(Source s1, Source s2) noexcept
	{
		return Combined{ s1.width, s1.height, s2.width, s2.height };
	}

	void playSndRsc(std::uint16_t resource, HINSTANCE hInst) noexcept;
	void playSndRscAsync(std::uint16_t resource, HINSTANCE hInst) noexcept;
	void stopSndRscAsync() noexcept;

	bool getScreenSize(HWND window, SIZE & screen) noexcept;
}
