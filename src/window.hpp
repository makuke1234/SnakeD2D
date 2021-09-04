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
			L"Error creating DirectWrite factory!",
			L"Error initialisisng application class!",
			L"Error creating Window!",
			L"Error creating Direct2D render target!",
			L"Error creating Direct2D assets!",
			L"Error creating Direct2D assets (bitmap brushes)!",
			L"Error creating DirectWrite assets (fonts)!"
		};

		HINSTANCE m_hInst{ nullptr };
		PSTR m_cmdArgs;
		HWND m_hwnd{ nullptr };
		ID2D1Factory * m_pD2DFactory{ nullptr };
		IDWriteFactory * m_pDWriteFactory{ nullptr };
		ID2D1HwndRenderTarget * m_pRT{ nullptr };

		struct tilesStruct
		{
			std::vector<tile> obstacleTiles;
			std::deque<tile> snakeBodyTiles;
			tile snakeHeadTile, snakeFoodTile;

			void DestroyAssets() noexcept;
		} m_tiles;
		
		static constexpr std::size_t s_numFoodTiles{ 9 };

		struct bitmapsStruct
		{		
			ID2D1Bitmap * obstacleTile{ nullptr }, * snakeBodyTile{ nullptr },
				* snakeHeadTile{ nullptr };
			std::array<ID2D1Bitmap *, s_numFoodTiles> snakeFoodTiles;

			void DestroyAssets() noexcept;
		} m_bmps;
		struct bitmapBrushesStruct
		{
			ID2D1BitmapBrush * obstacleTile{ nullptr }, * snakeBodyTile{ nullptr },
				* snakeHeadTile{ nullptr };
			std::array<ID2D1BitmapBrush *, s_numFoodTiles> snakeFoodTiles;
			
			bool CreateAssets(ID2D1HwndRenderTarget * pRT, bitmapsStruct const & bmps) noexcept;
			void DestroyAssets() noexcept;
		} m_bmpBrushes;

		struct textStruct
		{
			IDWriteTextFormat * consolas16{ nullptr };
			ID2D1SolidColorBrush * pTextBrush{ nullptr };

			bool CreateAssets(ID2D1HwndRenderTarget * pRT, IDWriteFactory * pWF) noexcept;
			void DestroyAssets() noexcept;

			void OnRender(D2D1_SIZE_F const & tileSz, ID2D1HwndRenderTarget * pRT) const noexcept;
		} m_text;

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
			DWriteFactory,
			AppClass,
			Window,
			D2DRT,
			D2DAssets,
			D2DAssetsBmBrushes,
			DWAssetsFonts,

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

		[[nodiscard]] static constexpr D2D1_SIZE_F s_calcTile(D2D1_SIZE_F const & tileSz, long cx, long cy) noexcept
		{
			return { .width = tileSz.width * float(cx), .height = tileSz.height * float(cy) };
		}
		[[nodiscard]] constexpr float calcTilex(long cx) const noexcept
		{
			return this->m_tileSzF.width * float(cx);
		}
		[[nodiscard]] constexpr float calcTiley(long cy) const noexcept
		{
			return this->m_tileSzF.height * float(cy);
		}
		[[nodiscard]] constexpr D2D1_SIZE_F calcTile(long cx, long cy) const noexcept
		{
			return this->s_calcTile(this->m_tileSzF, cx, cy);
		}


		[[nodiscard]] static constexpr D2D1_SIZE_U s_revcalcTile(D2D1_SIZE_F const & tileSz, float x, float y) noexcept
		{
			return { .width = UINT32(x / tileSz.width + .5f), .height = UINT32(y / tileSz.height + .5f) };
		}
		[[nodiscard]] constexpr long revcalcTilex(float x) const noexcept
		{
			return long(x / this->m_tileSzF.width);
		}
		[[nodiscard]] constexpr long revcalcTiley(float y) const noexcept
		{
			return long(y / this->m_tileSzF.height);
		}
		[[nodiscard]] constexpr D2D1_SIZE_U revcalcTile(float x, float y) const noexcept
		{
			return this->s_revcalcTile(this->m_tileSzF, x, y);
		}

		tile makeSnakeTile(long cx, long cy) const noexcept;
		void moveTile(tile & t, long cx, long cy) const noexcept;

		void initSnakeData();
	};

}
