#pragma once

#include "common.hpp"
#include "tiles.hpp"

namespace snake
{
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
			L"Error creating Direct2D render target!",
			L"Error creating Direct2D assets!"
		};

		PSTR m_cmdArgs;
		HWND m_hwnd{ nullptr };
		ID2D1Factory * m_pD2DFactory{ nullptr };
		ID2D1HwndRenderTarget * m_pRT{ nullptr };

		std::vector<tile> m_obstacleTiles;
		std::vector<tile> m_snakeBodyTiles;
		tile m_snakeHeadTile, m_snakeFoodTile;

		ID2D1Bitmap * m_pObstacleTileBm{ nullptr }, * m_pSnakeBodyTileBm{ nullptr },
			* m_pSnakeHeadTileBm{ nullptr }, * m_pSnakeFoodTileBm{ nullptr };

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

		template<class X, class Y>
		[[nodiscard]] X dipxy(Y x, Y y)
		{
			return { dipx(x), dipy(y) };
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
			D2DRT,
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
