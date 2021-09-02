#pragma once

#include "common.hpp"
#include "tiles.hpp"
#include "snakelogic.hpp"

#include <deque>
#include <array>

namespace snake
{
	class Application
	{
	private:
		friend class logic;

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

		HINSTANCE m_hInst{ nullptr };
		PSTR m_cmdArgs;
		HWND m_hwnd{ nullptr };
		ID2D1Factory * m_pD2DFactory{ nullptr };
		ID2D1HwndRenderTarget * m_pRT{ nullptr };

		std::vector<tile> m_obstacleTiles;
		std::deque<tile> m_snakeBodyTiles;
		tile m_snakeHeadTile, m_snakeFoodTile;

		ID2D1Bitmap * m_pObstacleTileBm{ nullptr }, * m_pSnakeBodyTileBm{ nullptr },
			* m_pSnakeHeadTileBm{ nullptr };
		std::array<ID2D1Bitmap *, 9> m_pSnakeFoodTilesBm;

		FLOAT m_dpiX{ 96.f }, m_dpiY{ 96.f };
		D2D1_SIZE_U m_border{};
		POINT m_minSize{ .x = 640, .y = 480 };
		static constexpr FLOAT tileSz{ 18.f }, fieldWidth{ 63.f }, fieldHeight{ 36.f };
		D2D1_SIZE_F m_tileSzF{ tileSz, tileSz };

		logic m_snakeLogic{ *this };

		static LRESULT CALLBACK wproc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept;

		void p_calcDpiSpecific() noexcept;
		bool p_loadD2D1BitmapFromResource(
			LPCWSTR resourceId,
			D2D1_SIZE_U const & bmSize,
			ID2D1Bitmap *& bmRef,
			void * opBuf = nullptr,
			std::size_t * bufSize = nullptr
		) noexcept;

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

		template<class T>
		[[nodiscard]] T revdipx(T input) const noexcept
		{
			return T(FLOAT(input) * 96.f / this->m_dpiX);
		}
		template<class T>
		[[nodiscard]] T revdipy(T input ) const noexcept
		{
			return T(FLOAT(input) * 96.f / this->m_dpiY);
		}

		template<class retT, class T>
		[[nodiscard]] retT dipxi(T input, long mult) const noexcept
		{
			return retT(long(dipx(input)) * mult);
		}
		template<class retT, class T>
		[[nodiscard]] retT dipyi(T input, long mult) const noexcept
		{
			return retT(long(dipy(input)) * mult);
		}

		template<class X, class Y>
		[[nodiscard]] X dipxy(Y x, Y y) const noexcept
		{
			auto [a1, a2] = X();
			using retT = decltype(a1);
			return { retT(dipx(x)), retT(dipy(y)) };
		}
		template<class X, class Y>
		[[nodiscard]] X dipxyi(Y x, Y y, long xmulti = 1, long ymulti = 1) const noexcept
		{
			auto [a1, a2] = X();
			using retT = decltype(a1);
			return { retT(long(dipx(x)) * xmulti), retT(long(dipy(y)) * ymulti) };
		}

		template<class X, class Y>
		[[nodiscard]] X revdipxy(Y x, Y y) const noexcept
		{
			auto [a1, a2] = X();
			using retT = decltype(a1);
			return { retT(revdipx(x)), retT(revdipy(y)) };
		}

		bool Init(HINSTANCE hInst, int nCmdShow);
		int MsgLoop() noexcept;

		void Error(const wchar_t * str) const noexcept;
		static void sError(const wchar_t * str) noexcept;

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

		LRESULT OnKeyPress(WPARAM wp, LPARAM lp) noexcept;
		LRESULT OnKeyRelease(WPARAM wp, LPARAM lp) noexcept;

		tile makeSnakeTile(long cx, long cy) const noexcept;
		void moveTile(tile & t, long cx, long cy) const noexcept;

		void initSnakeData();
	};

}
