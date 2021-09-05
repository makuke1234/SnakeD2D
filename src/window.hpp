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
		LPCWSTR m_lpCmdArgs;
		HWND m_hwnd{ nullptr };
		ID2D1Factory * m_pD2DFactory{ nullptr };
		IDWriteFactory * m_pDWriteFactory{ nullptr };
		ID2D1HwndRenderTarget * m_pRT{ nullptr };

		struct tilesStruct
		{
			std::vector<tile> obstacleTiles;
			std::deque<tile> snakeBodyTiles;
			tile snakeHeadTile, snakeFoodTile;

			void destroyAssets() noexcept;
		} m_tiles;
		
		static constexpr std::size_t s_numFoodTiles{ 9 };

		struct bmpsStruct
		{		
			ID2D1Bitmap * obstacleTile{ nullptr }, * snakeBodyTile{ nullptr },
				* snakeHeadTile{ nullptr };
			std::array<ID2D1Bitmap *, s_numFoodTiles> snakeFoodTiles;

			void destroyAssets() noexcept;
		} m_bmps;
		struct bmpBrushesStruct
		{
			ID2D1BitmapBrush * obstacleTile{ nullptr }, * snakeBodyTile{ nullptr },
				* snakeHeadTile{ nullptr };
			std::array<ID2D1BitmapBrush *, s_numFoodTiles> snakeFoodTiles;
			
			bool createAssets(ID2D1HwndRenderTarget * pRT, bmpsStruct const & bmps) noexcept;
			void destroyAssets() noexcept;
		} m_bmpBrushes;

		struct textStruct
		{
			IDWriteTextFormat * consolas16{ nullptr };
			ID2D1SolidColorBrush * pTextBrush{ nullptr };

			bool createAssets(ID2D1HwndRenderTarget * pRT, IDWriteFactory * pWF) noexcept;
			void destroyAssets() noexcept;

			void onRender(D2D1_SIZE_F const & tileSz, ID2D1HwndRenderTarget * pRT) const noexcept;
		} m_text;

		FLOAT m_dpiX{ 96.f }, m_dpiY{ 96.f };
		D2D1_SIZE_U m_border{};
		POINT m_minSize{ .x = 640, .y = 480 };
		static constexpr FLOAT tileSz{ 18.f }, fieldWidth{ 63.f }, fieldHeight{ 36.f };
		D2D1_SIZE_F m_tileSzF{ tileSz, tileSz };

		snake::logic m_snakeLogic{ *this };

		static LRESULT CALLBACK wproc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept;

		void p_calcDpiSpecific() noexcept;
		bool p_loadD2D1BitmapFromResource(
			std::uint16_t resourceId,
			D2D1_SIZE_U const & bmSize,
			ID2D1Bitmap *& bmRef,
			void * opBuf = nullptr,
			std::size_t * bufSize = nullptr
		) noexcept;

	public:
		// Disable default constructor
		Application() = delete;
		Application(LPCWSTR lpCmdArgs) noexcept;
		// Disable copy constructors
		Application(Application const &) = delete;
		Application & operator=(Application const &) = delete;
		~Application() noexcept;

		template<class T>
		[[nodiscard]] T fromDipx(T input) const noexcept
		{
			return T((FLOAT(input) * this->m_dpiX) / 96.f);
		}
		template<class T>
		[[nodiscard]] T fromDipy(T input) const noexcept
		{
			return T((FLOAT(input) * this->m_dpiY) / 96.f);
		}

		template<class T>
		[[nodiscard]] T toDipx(T input) const noexcept
		{
			return T(FLOAT(input) * 96.f / this->m_dpiX);
		}
		template<class T>
		[[nodiscard]] T toDipy(T input ) const noexcept
		{
			return T(FLOAT(input) * 96.f / this->m_dpiY);
		}

		template<class retT, class T>
		[[nodiscard]] retT fromDipxi(T input, long mult) const noexcept
		{
			return retT(long(fromDipx(input)) * mult);
		}
		template<class retT, class T>
		[[nodiscard]] retT fromDipyi(T input, long mult) const noexcept
		{
			return retT(long(fromDipy(input)) * mult);
		}

		template<class X, class Y>
		[[nodiscard]] X fromDipxy(Y x, Y y) const noexcept
		{
			auto [a1, a2] = X();
			using retT = decltype(a1);
			return { retT(fromDipx(x)), retT(fromDipy(y)) };
		}
		template<class X, class Y>
		[[nodiscard]] X fromDipxyi(Y x, Y y, long xmulti = 1, long ymulti = 1) const noexcept
		{
			auto [a1, a2] = X();
			using retT = decltype(a1);
			return { retT(long(fromDipx(x)) * xmulti), retT(long(fromDipy(y)) * ymulti) };
		}

		template<class X, class Y>
		[[nodiscard]] X toDipxy(Y x, Y y) const noexcept
		{
			auto [a1, a2] = X();
			using retT = decltype(a1);
			return { retT(toDipx(x)), retT(toDipy(y)) };
		}

		bool initApp(HINSTANCE hInst, int nCmdShow);
		int msgLoop() noexcept;


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
		static void s_error(errid id) noexcept;
		static void s_error(const wchar_t * str) noexcept;
		
		void error(errid id) const noexcept;
		void error(const wchar_t * str) const noexcept;

		bool createAssets() noexcept;
		void destroyAssets() noexcept;

		void onRender() noexcept;
		void onResize(UINT width, UINT height) const noexcept;

		LRESULT onKeyPress(WPARAM wp, LPARAM lp) noexcept;
		LRESULT onKeyRelease(WPARAM wp, LPARAM lp) noexcept;

		[[nodiscard]] static constexpr D2D1_SIZE_F s_calcToTile(D2D1_SIZE_F const & tileSz, long cx, long cy) noexcept
		{
			return { .width = tileSz.width * float(cx), .height = tileSz.height * float(cy) };
		}
		[[nodiscard]] constexpr float calcToTilex(long cx) const noexcept
		{
			return this->m_tileSzF.width * float(cx);
		}
		[[nodiscard]] constexpr float calcToTiley(long cy) const noexcept
		{
			return this->m_tileSzF.height * float(cy);
		}
		[[nodiscard]] constexpr D2D1_SIZE_F calcToTile(long cx, long cy) const noexcept
		{
			return this->s_calcToTile(this->m_tileSzF, cx, cy);
		}


		[[nodiscard]] static constexpr D2D1_SIZE_U s_calcFromTile(D2D1_SIZE_F const & tileSz, float x, float y) noexcept
		{
			return { .width = UINT32(x / tileSz.width + .5f), .height = UINT32(y / tileSz.height + .5f) };
		}
		[[nodiscard]] constexpr long calcFromTilex(float x) const noexcept
		{
			return long(x / this->m_tileSzF.width);
		}
		[[nodiscard]] constexpr long calcFromTiley(float y) const noexcept
		{
			return long(y / this->m_tileSzF.height);
		}
		[[nodiscard]] constexpr D2D1_SIZE_U calcFromTile(float x, float y) const noexcept
		{
			return this->s_calcFromTile(this->m_tileSzF, x, y);
		}

		tile makeSnakeTile(long cx, long cy) const noexcept;
		void moveTile(tile & t, long cx, long cy) const noexcept;

		void initSnakeData();

		void restartGame();

		void playSnd(std::uint16_t rsc) const noexcept;
		void playSndAsync(std::uint16_t rsc) const noexcept;
	};

}
