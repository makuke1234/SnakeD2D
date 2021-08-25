#pragma once


#include "common.h"

#include <d2d1.h>
#include <dwrite.h>

#include <string_view>

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


	class Application
	{
	private:
		static constexpr std::wstring_view applicationName{ L"SnakeD2D" },
			className{ L"SnakeDirect2DClass" };

		static constexpr const wchar_t * s_errorIds[]
		{
			L"Unknown error occurred!",
			L"Error creating Direct2D factory!",
			L"Error initialisisng application class!",
			L"Error creating Window!",
			L"Error creating Direct2D assets (render target)!"
		};

		PSTR m_cmdArgs;
		HWND m_hwnd{ nullptr };
		ID2D1Factory * m_pD2DFactory{ nullptr };
		ID2D1HwndRenderTarget * m_pRT{ nullptr };

		ID2D1Layer * m_pLayer{ nullptr };

		ID2D1SolidColorBrush * m_testBrush{ nullptr };

		FLOAT m_dpiX{ 96.f }, m_dpiY{ 96.f };
		POINT m_minSize{ .x = 640, .y = 480 };

		static LRESULT CALLBACK wproc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept;

	public:
		// Disable default constructor
		Application() = delete;
		Application(PSTR cmdArgs) noexcept;
		// Disable copy constructors
		Application(Application const &) = delete;
		Application & operator=(Application const &) = delete;
		~Application() noexcept;

		template<class T>
		[[nodiscard]] T dipx(T input) const noexcept
		{
			return T((FLOAT(input) * this->m_dpiX) / 96.f);
		}
		template<class T>
		[[nodiscard]] T dipy(T input) const noexcept
		{
			return T((FLOAT(input) * this->m_dpiY) / 96.f);
		}

		bool Init(HINSTANCE hInst, int nCmdShow);
		int MsgLoop() noexcept;

		void Error(std::wstring_view str) const noexcept;
		static void sError(std::wstring_view str) noexcept;

		enum class errid : std::int_fast8_t
		{
			Unknown,
			D2DFactory,
			AppClass,
			Window,
			D2DAssets,

			errid_enum_size
		};
		void Error(errid id) const noexcept;
		static void sError(errid id) noexcept;

		bool CreateAssets() noexcept;
		void DestroyAssets() noexcept;

		void OnRender() noexcept;
		void OnResize(UINT width, UINT height) const noexcept;
	};

}
