#pragma once

#include "common.hpp"
#include "tiles.hpp"
#include "snakelogic.hpp"

#include <deque>
#include <array>
#include <random>

namespace snake
{
	class Application
	{
	private:
		friend class Logic;

		static constexpr std::wstring_view s_applicationName{ L"SnakeD2D" },
			s_className{ L"SnakeDirect2DClass" };

		static constexpr const wchar_t * sc_errorIds[]
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

		std::mt19937 m_rng;
		HINSTANCE m_hInst{ nullptr };
		LPCWSTR m_lpCmdArgs;
		HWND m_hwnd{ nullptr };
		dx::Factory * m_pD2DFactory{ nullptr };
		dw::Factory * m_pDWriteFactory{ nullptr };
		dx::HwndRT * m_pRT{ nullptr };

		struct TilesStruct
		{
			std::vector<Tile> obstacleTiles;
			std::deque<Tile> snakeBodyTiles;
			Tile snakeHeadTile, snakeFoodTile;

			void destroyAssets() noexcept;
		} m_tiles;
		friend struct TilesStruct;
		
		static constexpr std::size_t s_numFoodTiles{ 9 };

		snake::Logic m_snakeLogic{ *this };
		
		struct BmpStruct
		{		
			dx::Bmp * obstacleTile{ nullptr }, * snakeBodyTile{ nullptr },
				* snakeHeadTile{ nullptr };
			std::array<dx::Bmp *, s_numFoodTiles> snakeFoodTiles;

			void destroyAssets() noexcept;
		} m_bmps;
		struct BmpBrushesStruct
		{
			dx::BmBrush * obstacleTile{ nullptr }, * snakeBodyTile{ nullptr },
				* snakeHeadTile{ nullptr };
			std::array<dx::BmBrush *, s_numFoodTiles> snakeFoodTiles;
			
			dx::BmBrush * randomFoodTile(std::mt19937 & rng) const noexcept;
			bool createAssets(dx::HwndRT * pRT, BmpStruct const & bmps) noexcept;
			void destroyAssets() noexcept;
		} m_bmpBrushes;

		struct TextStruct
		{
			dw::TxtFormat * consolas16{ nullptr }, * consolas16Centered{ nullptr },
				* consolas24CenteredBold{ nullptr };
			dx::SolidBrush * pScoreBrush{ nullptr }, * pWinBrush{ nullptr },
				* pLoseBrush{ nullptr }, * pPauseBrush{ nullptr };

			bool createAssets(dx::HwndRT * pRT, dw::Factory * pWF) noexcept;
			void destroyAssets() noexcept;

			void onRender(dx::SzF const & tileSz, dx::HwndRT * pRT, snake::Logic::SnakeInfo::Scoring const & scoring) const noexcept;
		} m_text;

		dx::F m_dpiX{ 96.f }, m_dpiY{ 96.f };
		dx::SzU m_border{};
		POINT m_minSize{ .x = 640, .y = 480 };
		static constexpr dx::F s_tileSz{ 18.f }, s_fieldWidth{ 63.f }, s_fieldHeight{ 36.f };
		dx::SzF m_tileSzF{ s_tileSz, s_tileSz };


		static LRESULT CALLBACK sp_winProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept;

		void p_calcDpiSpecific() noexcept;
		bool p_loadD2D1BitmapFromResource(
			std::uint16_t resourceId,
			dx::SzU const & bmSize,
			dx::Bmp *& bmRef,
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
			return T((dx::F(input) * this->m_dpiX) / 96.f);
		}
		template<class T>
		[[nodiscard]] T fromDipy(T input) const noexcept
		{
			return T((dx::F(input) * this->m_dpiY) / 96.f);
		}

		template<class T>
		[[nodiscard]] T toDipx(T input) const noexcept
		{
			return T(dx::F(input) * 96.f / this->m_dpiX);
		}
		template<class T>
		[[nodiscard]] T toDipy(T input ) const noexcept
		{
			return T(dx::F(input) * 96.f / this->m_dpiY);
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


		enum class errid : std::uint8_t
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

			enum_size
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

		[[nodiscard]] static constexpr dx::SzF s_calcToTile(dx::SzF const & tileSz, long cx, long cy) noexcept
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
		[[nodiscard]] constexpr dx::SzF calcToTile(long cx, long cy) const noexcept
		{
			return this->s_calcToTile(this->m_tileSzF, cx, cy);
		}


		[[nodiscard]] static constexpr dx::SzU s_calcFromTile(dx::SzF const & tileSz, float x, float y) noexcept
		{
			return { .width = dx::U32(x / tileSz.width + .5f), .height = dx::U32(y / tileSz.height + .5f) };
		}
		[[nodiscard]] constexpr long calcFromTilex(float x) const noexcept
		{
			return long(x / this->m_tileSzF.width);
		}
		[[nodiscard]] constexpr long calcFromTiley(float y) const noexcept
		{
			return long(y / this->m_tileSzF.height);
		}
		[[nodiscard]] constexpr dx::SzU calcFromTile(float x, float y) const noexcept
		{
			return this->s_calcFromTile(this->m_tileSzF, x, y);
		}

		Tile makeRandomSnakeTile() noexcept;
		Tile makeSnakeTile(long cx, long cy) const noexcept;
		void makeSnakeTile(Tile & t, long cx, long cy) const noexcept;
		void moveTile(Tile & t, long cx, long cy) const noexcept;

		void initSnakeData();

		void genFood(snake::Tile & output) noexcept;

		void restartGame();

		void playSnd(std::uint16_t rsc) const noexcept;
		void playSndAsync(std::uint16_t rsc) const noexcept;
	};

}
