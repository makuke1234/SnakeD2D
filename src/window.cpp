#include "window.hpp"
#include "resource.h"

#include <tgmath.h>

#include <ctime>
#include <memory>

LRESULT CALLBACK snake::Application::sp_winProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept
{
	static snake::Application * This{};

	if (This == nullptr) [[unlikely]]
	{
		if (uMsg == WM_CREATE) [[likely]]
		{
			// Receive "This"
			if (auto cs = reinterpret_cast<CREATESTRUCTW *>(lp); cs != nullptr && cs->lpCreateParams != nullptr) [[likely]]
			{
				This = static_cast<snake::Application *>(cs->lpCreateParams);
			}
			else [[unlikely]]
			{
				snake::Application::s_error(errid::Window);
				::DestroyWindow(hwnd);
			}

			return 0;
		}
		else [[unlikely]]
			return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}

	switch (uMsg)
	{
	case WM_PAINT:
		This->onRender();
		break;
	case WM_KEYDOWN:
		return This->onKeyPress(wp, lp);
	case WM_KEYUP:
		return This->onKeyRelease(wp, lp);
	case WM_SIZING:
	{
		auto r = reinterpret_cast<RECT *>(lp);
		POINT tempMSz = This->m_minSize;
		// x-coordinate
		switch (wp)
		{
		case WMSZ_LEFT:
		case WMSZ_TOPLEFT:
		case WMSZ_BOTTOMLEFT:
			if ((r->right - r->left) < tempMSz.x)
				r->left = r->right - tempMSz.x;
			break;
		case WMSZ_RIGHT:
		case WMSZ_TOPRIGHT:
		case WMSZ_BOTTOMRIGHT:
			if ((r->right - r->left) < tempMSz.x)
				r->right = r->left + tempMSz.x;
			break;
		}

		// y-coordinate
		switch (wp)
		{
		case WMSZ_TOP:
		case WMSZ_TOPLEFT:
		case WMSZ_TOPRIGHT:
			if ((r->bottom - r->top) < tempMSz.y)
				r->top = r->bottom - tempMSz.y;
			break;
		case WMSZ_BOTTOM:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_BOTTOMRIGHT:
			if ((r->bottom - r->top) < tempMSz.y)
				r->bottom = r->top + tempMSz.y;
			break;
		}
		break;
	}
	case WM_SIZE:
	{
		RECT r;
		::GetClientRect(hwnd, &r);
		This->onResize(r.right - r.left, r.bottom - r.top);
		break;
	}
	case WM_DPICHANGED:
	{
		if (This->m_pRT)
			This->m_pRT->GetDpi(&This->m_dpiX, &This->m_dpiY);
		auto newR = reinterpret_cast<RECT *>(lp);
		::SetWindowPos(
			hwnd,
			nullptr,
			newR->left, newR->right,
			newR->right - newR->left, newR->bottom - newR->top,
			SWP_NOZORDER
		);
		// Calculate new border
		This->p_calcDpiSpecific();
		break;
	}
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 1;
	default:
		return ::DefWindowProcW(hwnd, uMsg, wp, lp);
	}

	return 0;
}

void snake::Application::TilesStruct::destroyAssets() noexcept
{
	for (auto & i : this->obstacleTiles)
		i.destroyAssets();
	for (auto & i : this->snakeBodyTiles)
		i.destroyAssets();
	this->snakeHeadTile.destroyAssets();
	this->snakeFoodTile.destroyAssets();
}

void snake::Application::BmpStruct::destroyAssets() noexcept
{
	snake::safeRelease(this->obstacleTile);
	snake::safeRelease(this->snakeBodyTile);
	snake::safeRelease(this->snakeHeadTile);
	for (auto & i : this->snakeFoodTiles)
	{
		snake::safeRelease(i);
	}
}

dx::BmBrush * snake::Application::BmpBrushesStruct::randomFoodTile(std::mt19937 & rng) const noexcept
{
	static std::uniform_int_distribution<std::size_t> distrib(0, Application::s_numFoodTiles - 1);
	return this->snakeFoodTiles[distrib(rng)];
}
bool snake::Application::BmpBrushesStruct::createAssets(
	dx::HwndRT * pRT,
	Application::BmpStruct const & bmps
) noexcept
{
	if ((this->obstacleTile = Tile::createBmpBrush(pRT, bmps.obstacleTile)) == nullptr) [[unlikely]]
		return false;
	if ((this->snakeBodyTile = Tile::createBmpBrush(pRT, bmps.snakeBodyTile)) == nullptr) [[unlikely]]
		return false;
	if ((this->snakeHeadTile = Tile::createBmpBrush(pRT, bmps.snakeHeadTile)) == nullptr) [[unlikely]]
		return false;
	for (std::size_t i = 0; i < this->snakeFoodTiles.size(); ++i)
	{
		if ((this->snakeFoodTiles[i] = Tile::createBmpBrush(pRT, bmps.snakeFoodTiles[i])) == nullptr) [[unlikely]]
			return false;
	}

	return true;
}
void snake::Application::BmpBrushesStruct::destroyAssets() noexcept
{
	snake::safeRelease(this->obstacleTile);
	snake::safeRelease(this->snakeBodyTile);
	snake::safeRelease(this->snakeHeadTile);
	for (auto & i : this->snakeFoodTiles)
	{
		snake::safeRelease(i);
	}
}

bool snake::Application::TextStruct::createAssets(dx::HwndRT * pRT, dw::Factory * pWF) noexcept
{
	auto hr = pWF->CreateTextFormat(
		L"Consolas",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16.f,
		L"",
		&this->consolas16
	);
	if (FAILED(hr)) [[unlikely]]
		return false;

	hr = pWF->CreateTextFormat(
		L"Consolas",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16.f,
		L"",
		&this->consolas16Centered
	);
	if (FAILED(hr)) [[unlikely]]
		return false;

	// Set alignment
	hr = this->consolas16Centered->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	if (FAILED(hr)) [[unlikely]]
		return false;

	hr = pWF->CreateTextFormat(
		L"Consolas",
		nullptr,
		DWRITE_FONT_WEIGHT_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		24.f,
		L"",
		&this->consolas24CenteredBold
	);
	if (FAILED(hr)) [[unlikely]]
		return false;

	// Set alignment
	hr = this->consolas24CenteredBold->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	if (FAILED(hr)) [[unlikely]]
		return false;

	hr = pRT->CreateSolidColorBrush(
		D2D1::ColorF(255.f / 255.f, 242.f / 255.f, 0.f / 255.f),
		&this->pScoreBrush
	);
	if (FAILED(hr)) [[unlikely]]
		return false;
	hr = pRT->CreateSolidColorBrush(
		D2D1::ColorF(0.f / 255.f, 196.f / 255.f, 117.f / 255.f),
		&this->pWinBrush
	);
	if (FAILED(hr)) [[unlikely]]
		return false;
	hr = pRT->CreateSolidColorBrush(
		D2D1::ColorF(241.f / 255.f, 75.f / 255.f, 84.f / 255.f),
		&this->pLoseBrush
	);
	if (FAILED(hr)) [[unlikely]]
		return false;

	hr = pRT->CreateSolidColorBrush(
		D2D1::ColorF(153.f / 255.f, 217.f / 255.f, 234.f / 255.f),
		&this->pPauseBrush
	);
	if (FAILED(hr)) [[unlikely]]
		return false;


	return true;
}
void snake::Application::TextStruct::destroyAssets() noexcept
{
	snake::safeRelease(this->pScoreBrush);
	snake::safeRelease(this->pWinBrush);
	snake::safeRelease(this->pLoseBrush);
	snake::safeRelease(this->pPauseBrush);

	snake::safeRelease(this->consolas16);
	snake::safeRelease(this->consolas24CenteredBold);
}

void snake::Application::TextStruct::onRender(dx::SzF const & tileSz, dx::HwndRT * pRT, snake::Logic::SnakeInfo::Scoring const & scoring) const noexcept
{
	constexpr long posx0{ 20 }, posy0{ 8 }, posx1{ long(s_fieldWidth) - posx0 - 1 }, posy1{ posy0 + 1 };

	switch (scoring.mode)
	{
	case Logic::SnakeInfo::modes::normal:
	{
		// Draw score
		std::wstring txt{ L"Score: " };
		txt += std::to_wstring(scoring.score);

		auto ltop{ Application::s_calcToTile(tileSz, 50, 1) };
		auto rbottom{ Application::s_calcToTile(tileSz, 62, 2) };

		pRT->DrawTextW(
			txt.c_str(),
			dx::U32(txt.size()),
			this->consolas16,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pScoreBrush
		);
		
		// Draw time
		txt = L"Time: ";
		{
			int64_t lseconds = int64_t(scoring.totalTime);
			int seconds = int(lseconds % 60ll);
			int milliseconds = int(int64_t(1000.f * scoring.totalTime) - 1000ll * lseconds) / 10;
			int hours = int(lseconds / 3600ll);
			int minutes = int((lseconds - int64_t(hours) * 3600ll - int64_t(seconds)) / 60ll);

			wchar_t digit[3] = { 0 };
			digit[0] = L'0' + wchar_t(hours / 10);
			digit[1] = L'0' + wchar_t(hours % 10);
			txt += digit;
			txt += L':';

			digit[0] = L'0' + wchar_t(minutes / 10);
			digit[1] = L'0' + wchar_t(minutes % 10);
			txt += digit;
			txt += L':';

			digit[0] = L'0' + wchar_t(seconds / 10);
			digit[1] = L'0' + wchar_t(seconds % 10);
			txt += digit;
			txt += L'.';

			digit[0] = L'0' + wchar_t(milliseconds / 10);
			digit[1] = L'0' + wchar_t(milliseconds % 10);
			txt += digit;
		}
		ltop    = Application::s_calcToTile(tileSz, 0, Application::s_fieldHeight - 1);
		rbottom = Application::s_calcToTile(tileSz, Application::s_fieldWidth, Application::s_fieldHeight);
		pRT->DrawTextW(
			txt.c_str(),
			dx::U32(txt.size()),
			this->consolas16Centered,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pScoreBrush
		);

		// If game has been paused
		if (scoring.paused)
		{
			constexpr std::wstring_view pauseStr1{ L"The game has been paused." },
				pauseStr2{ L"Press 'ESCAPE' to resume or 'ENTER' to restart." };
			auto ltop{ Application::s_calcToTile(tileSz, posx0, posy0) };
			auto rbottom{ Application::s_calcToTile(tileSz, posx1, posy1) };

			pRT->DrawTextW(
				pauseStr1.data(),
				dx::U32(pauseStr1.size()),
				this->consolas24CenteredBold,
				snake::combine<dx::RectF>(ltop, rbottom),
				this->pPauseBrush
			);

			ltop = Application::s_calcToTile(tileSz, posx0, posy0 + 16);
			rbottom = Application::s_calcToTile(tileSz, posx1, posy1 + 16);

			pRT->DrawTextW(
				pauseStr2.data(),
				dx::U32(pauseStr2.size()),
				this->consolas24CenteredBold,
				snake::combine<dx::RectF>(ltop, rbottom),
				this->pPauseBrush
			);
		}
		break;
	}
	case Logic::SnakeInfo::modes::win:
	{
		constexpr std::wstring_view win{ L"You won!" }, scoreLabel{ L"Your score was:" };

		auto ltop{ Application::s_calcToTile(tileSz, posx0, posy0) };
		auto rbottom{ Application::s_calcToTile(tileSz, posx1, posy1) };

		pRT->DrawTextW(
			win.data(),
			dx::U32(win.size()),
			this->consolas24CenteredBold,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pWinBrush
		);

		ltop = Application::s_calcToTile(tileSz, posx0, posy0 + 2);
		rbottom = Application::s_calcToTile(tileSz, posx1, posy1 + 2);

		pRT->DrawTextW(
			scoreLabel.data(),
			dx::U32(scoreLabel.size()),
			this->consolas24CenteredBold,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pWinBrush
		);

		ltop = Application::s_calcToTile(tileSz, posx0, posy0 + 4);
		rbottom = Application::s_calcToTile(tileSz, posx1, posy1 + 4);


		// Show score
		std::wstring scoreStr{ std::to_wstring(scoring.score) };
		pRT->DrawTextW(
			scoreStr.c_str(),
			dx::U32(scoreStr.size()),
			this->consolas24CenteredBold,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pWinBrush
		);

		// Show instructions
		constexpr std::wstring_view retStr{ L"Press 'ENTER' to start again." };
		
		ltop = Application::s_calcToTile(tileSz, posx0, posy0 + 16);
		rbottom = Application::s_calcToTile(tileSz, posx1, posy1 + 16);

		pRT->DrawTextW(
			retStr.data(),
			dx::U32(retStr.size()),
			this->consolas24CenteredBold,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pWinBrush
		);
		break;
	}
	case Logic::SnakeInfo::modes::game_over:
	{
		constexpr std::wstring_view gameOver{ L"Game over!" }, scoreLabel{ L"Your score was:" };

		auto ltop{ Application::s_calcToTile(tileSz, posx0, posy0) };
		auto rbottom{ Application::s_calcToTile(tileSz, posx1, posy1) };

		pRT->DrawTextW(
			gameOver.data(),
			dx::U32(gameOver.size()),
			this->consolas24CenteredBold,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pLoseBrush
		);

		ltop = Application::s_calcToTile(tileSz, posx0, posy0 + 2);
		rbottom = Application::s_calcToTile(tileSz, posx1, posy1 + 2);

		pRT->DrawTextW(
			scoreLabel.data(),
			dx::U32(scoreLabel.size()),
			this->consolas24CenteredBold,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pLoseBrush
		);

		ltop = Application::s_calcToTile(tileSz, posx0, posy0 + 4);
		rbottom = Application::s_calcToTile(tileSz, posx1, posy1 + 4);


		// Show score
		std::wstring scoreStr{ std::to_wstring(scoring.score) };
		pRT->DrawTextW(
			scoreStr.c_str(),
			dx::U32(scoreStr.size()),
			this->consolas24CenteredBold,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pLoseBrush
		);

		// Show instructions
		constexpr std::wstring_view retStr{ L"Press 'ENTER' to start again." };
		
		ltop = Application::s_calcToTile(tileSz, posx0, posy0 + 16);
		rbottom = Application::s_calcToTile(tileSz, posx1, posy1 + 16);

		pRT->DrawTextW(
			retStr.data(),
			dx::U32(retStr.size()),
			this->consolas24CenteredBold,
			snake::combine<dx::RectF>(ltop, rbottom),
			this->pLoseBrush
		);
		break;
	}
	}
}

void snake::Application::p_calcDpiSpecific() noexcept
{
	RECT clientR{}, windowR{};
	::GetClientRect(this->m_hwnd, &clientR);
	::GetWindowRect(this->m_hwnd, &windowR);
	this->m_border = dx::SzU{
		.width  = dx::U32(windowR.right  - windowR.left) - (clientR.right  - clientR.left),
		.height = dx::U32(windowR.bottom - windowR.top)  - (clientR.bottom - clientR.top)
	};

	this->m_minSize.x = LONG(std::ceil(this->fromDipxi<float>(s_tileSz, this->s_fieldWidth)  + float(this->m_border.width)));
	this->m_minSize.y = LONG(std::ceil(this->fromDipyi<float>(s_tileSz, this->s_fieldHeight) + float(this->m_border.height)));

	auto [rX, rY] = this->fromDipxy<dx::SzF>(s_tileSz, s_tileSz);
	float dX = rX - float(dx::U32(rX)), dY = rY - float(dx::U32(rY));
	this->m_tileSzF = dx::SzF{
		.width  = s_tileSz - this->toDipx(dX),
		.height = s_tileSz - this->toDipy(dY)
	};
}
void snake::Application::p_calcPositions() noexcept
{
	RECT r{};
	::GetClientRect(this->m_hwnd, &r);
	auto realx = float(r.right  - r.left);
	auto realy = float(r.bottom - r.top);

	auto x = this->fromDipxi<float>(s_tileSz, this->s_fieldWidth);
	auto y = this->fromDipyi<float>(s_tileSz, this->s_fieldHeight);

	auto factX = realx / x;
	auto factY = realy / y;

	this->m_factor = factX < factY ? factX : factY;
	this->m_offsetX = this->toDipx((realx - x * this->m_factor) / 2.0f);
	this->m_offsetY = this->toDipy((realy - y * this->m_factor) / 2.0f);
}
void snake::Application::p_toggleFullScreen() noexcept
{
	this->m_isFullscreen ^= 1;

	if (this->m_isFullscreen)
	{
		RECT r{};
		::GetClientRect(this->m_hwnd, &r);
		this->m_oldSize = {
			.cx = r.right  - r.left,
			.cy = r.bottom - r.top
		};
		::GetWindowRect(this->m_hwnd, &r);
		this->m_oldPos = {
			.cx = r.left,
			.cy = r.top
		};
		this->m_oldStyle = static_cast<DWORD>(::SetWindowLongPtrW(
			this->m_hwnd,
			GWL_STYLE,
			WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE
		));

		// Get screen size
		SIZE screen;
		if (snake::getScreenSize(this->m_hwnd, screen)) [[likely]]
		{
			::MoveWindow(this->m_hwnd, 0, 0, screen.cx, screen.cy, TRUE);
		}
		else [[unlikely]]
		{
			this->error(L"Error going to fullscreen!");
		}
	}
	else
	{
		RECT r = {
			.left   = 0,
			.top    = 0,
			.right  = this->m_oldSize.cx,
			.bottom = this->m_oldSize.cy
		};
		::SetWindowLongPtrW(
			this->m_hwnd,
			GWL_STYLE,
			this->m_oldStyle
		);
		::AdjustWindowRect(&r, this->m_oldStyle, FALSE);
		::MoveWindow(
			this->m_hwnd,
			this->m_oldPos.cx,
			this->m_oldPos.cy,
			r.right  - r.left,
			r.bottom - r.top,
			TRUE
		);
		this->p_calcDpiSpecific();
	}
}
bool snake::Application::p_loadD2D1BitmapFromResource(
	std::uint16_t resourceId,
	dx::SzU const & bmSize,
	dx::Bmp *& bmRef,
	void * opBuf,
	std::size_t * bufSize
) noexcept
{
	std::unique_ptr<COLORREF> ourBuf;
	COLORREF * accessBuf{ nullptr };

	auto hBmp = static_cast<HBITMAP>(::LoadImageW(
		this->m_hInst, MAKEINTRESOURCEW(resourceId), IMAGE_BITMAP, 0, 0, 0
	));
	if (hBmp == nullptr) [[unlikely]]
		return false;

	BITMAP bmp{};
	::GetObject(hBmp, sizeof(BITMAP), &bmp);

	BITMAPINFOHEADER bmpH{};
	bmpH.biSize        = sizeof bmpH;
	bmpH.biWidth       = bmp.bmWidth;
	bmpH.biHeight      = -bmp.bmHeight;
	bmpH.biPlanes      = 1;
	bmpH.biBitCount    = 8 * sizeof(COLORREF);
	bmpH.biCompression = BI_RGB;

	bmpH.biSizeImage = DWORD(sizeof(COLORREF) * bmp.bmWidth * bmp.bmHeight);

	if (bufSize != nullptr)
		*bufSize = bmpH.biSizeImage;


	if (opBuf != nullptr)
		accessBuf = static_cast<COLORREF *>(opBuf);
	else
	{
		ourBuf.reset(new COLORREF[bmp.bmWidth * bmp.bmHeight]);
		accessBuf = ourBuf.get();
	}

	auto res = ::GetDIBits(
		CreateCompatibleDC(nullptr), hBmp, 0, bmp.bmHeight, accessBuf,
		reinterpret_cast<BITMAPINFO *>(&bmpH), DIB_RGB_COLORS
	);
	::DeleteObject(hBmp);

	if (!res) [[unlikely]]
		return false;


	auto hr = this->m_pRT->CreateBitmap(
		dx::SzU{ .width = dx::U32(bmp.bmWidth), .height = dx::U32(bmp.bmHeight) },
		accessBuf,
		dx::U32(sizeof(COLORREF) * bmp.bmWidth),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX * dx::F(bmp.bmWidth)  / dx::F(bmSize.width),
			this->m_dpiY * dx::F(bmp.bmHeight) / dx::F(bmSize.height)
		),
		&bmRef
	);

	if (FAILED(hr)) [[unlikely]]
		return false;


	return true;
}

snake::Application::Application(LPCWSTR lpCmdArgs) noexcept
	: m_rng(std::random_device()()), m_lpCmdArgs(lpCmdArgs)
{}
snake::Application::~Application() noexcept
{
	this->destroyAssets();
	snake::safeRelease(this->m_pDWriteFactory);
	snake::safeRelease(this->m_pD2DFactory);
}

bool snake::Application::initApp(HINSTANCE hInst, int nCmdShow)
{
	this->m_hInst = hInst;
	// Init factory
	auto hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		IID_ID2D1Factory,
		static_cast<void **>(static_cast<void *>(&this->m_pD2DFactory))
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->error(errid::D2DFactory);
		return false;
	}

	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(this->m_pDWriteFactory),
		static_cast<IUnknown **>(static_cast<void *>(&this->m_pDWriteFactory))
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->error(errid::DWriteFactory);
		return false;
	}

	// Initialise application class
	WNDCLASSEXW w{};

	w.cbSize        = sizeof w;
	w.style         = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc   = &this->sp_winProc;
	w.hInstance     = hInst;
	w.hCursor       = ::LoadCursorW(nullptr, IDC_ARROW);
	w.hIconSm       = w.hIcon = ::LoadIconW(hInst, IDI_APPLICATION);
	w.lpszClassName = this->s_className.data();
	
	if (!::RegisterClassExW(&w)) [[unlikely]]
	{
		this->error(errid::AppClass);
		return false;
	}

	// Get DPI
	m_pD2DFactory->GetDesktopDpi(&this->m_dpiX, &this->m_dpiY);

	// Create window
	this->m_hwnd = ::CreateWindowExW(
		0,
		this->s_className.data(),
		this->s_applicationName.data(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInst,
		this
	);
	if (this->m_hwnd == nullptr) [[unlikely]]
	{
		this->error(errid::Window);
		return false;
	}

	this->p_calcDpiSpecific();
	::SetWindowPos(
		this->m_hwnd,
		nullptr,
		0,
		0,
		this->m_minSize.x,
		this->m_minSize.y,
		SWP_NOZORDER | SWP_NOMOVE
	);

	// Upper left
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{ 0.f, 0.f },
		dx::SzU{ 6, 1 }
	);
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{ 0.f, this->m_tileSzF.height },
		dx::SzU{ 1, 5 }
	);

	// Upper right
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{ this->m_tileSzF.width * (this->s_fieldWidth - 6.f), 0.f },
		dx::SzU{ 6, 1 }
	);
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{ this->m_tileSzF.width * (this->s_fieldWidth - 1.f), this->m_tileSzF.height },
		dx::SzU{ 1, 5 }
	);

	// Bottom left
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{ 0.f, this->m_tileSzF.height * (this->s_fieldHeight - 1.f) },
		dx::SzU{ 6, 1 }
	);
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{ 0.f, this->m_tileSzF.height * (this->s_fieldHeight - 6.f) },
		dx::SzU{ 1, 5 }
	);

	// Bottom right
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{
			this->m_tileSzF.width  * (this->s_fieldWidth  - 6.f),
			this->m_tileSzF.height * (this->s_fieldHeight - 1.f)
		},
		dx::SzU{ 6, 1 }
	);
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{
			this->m_tileSzF.width  * (this->s_fieldWidth  - 1.f),
			this->m_tileSzF.height * (this->s_fieldHeight - 6.f)
		},
		dx::SzU{ 1, 5 }
	);

	// Center piece
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		dx::SzF{
			this->m_tileSzF.width  * float(long(this->s_fieldWidth  - 8.f) / 2),
			this->m_tileSzF.height * float(long(this->s_fieldHeight - 6.f) / 2)
		},
		dx::SzU{ 8, 6 }
	);

	// Create render target & initialise assets
	if (!this->createAssets()) [[unlikely]]
	{
		return false;
	}

	// Initialize snake data structure
	this->initSnakeData();

	::ShowWindow(this->m_hwnd, nCmdShow);
	::UpdateWindow(this->m_hwnd);

	if (!this->m_snakeLogic.startSnakeLoop()) [[unlikely]]
	{
		return false;
	}

	return true;
}


int snake::Application::msgLoop() noexcept
{
	MSG msg;
	BOOL br;
	while ((br = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (br == -1) [[unlikely]]
		{
			this->s_error(errid::Unknown);
			return -1;
		}
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}
	return int(msg.wParam);
}

void snake::Application::s_error(errid id) noexcept
{
	using eT = std::underlying_type_t<errid>;
	
	auto oid = eT(id);
	if (oid >= eT(errid::enum_size)) [[unlikely]]
		oid = eT(errid::Unknown);

	::MessageBoxW(
		nullptr,
		snake::Application::sc_errorIds[oid], 
		Application::s_applicationName.data(),
		MB_ICONERROR | MB_OK
	);
}
void snake::Application::s_error(const wchar_t * str) noexcept
{
	::MessageBoxW(nullptr, str, Application::s_applicationName.data(), MB_ICONERROR | MB_OK);
}

void snake::Application::error(errid id) const noexcept
{
	using eT = std::underlying_type_t<errid>;

	auto oid = eT(id);
	if (oid >= eT(errid::enum_size)) [[unlikely]]
		oid = eT(errid::Unknown);
	
	::MessageBoxW(
		this->m_hwnd,
		this->sc_errorIds[oid],
		this->s_applicationName.data(),
		MB_ICONERROR | MB_OK
	);
}
void snake::Application::error(const wchar_t * str) const noexcept
{
	::MessageBoxW(
		this->m_hwnd,
		str,
		this->s_applicationName.data(),
		MB_ICONERROR | MB_OK
	);
}

bool snake::Application::createAssets() noexcept
{
	// Create Render target
	if (this->m_pRT)
		return true;

	RECT r;
	::GetClientRect(this->m_hwnd, &r);
	dx::SzU size = dx::SzU{ .width = dx::U32(r.right - r.left), .height = dx::U32(r.bottom - r.top) };
	auto hr = this->m_pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(this->m_hwnd, size),
		&this->m_pRT
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->error(errid::D2DRT);
		return false;
	}

	// Create assets
	
	// Create bitmaps

	auto [itWidth, itHeight] = this->fromDipxyi<dx::SzU>(s_tileSz, s_tileSz);
	auto borderW = (itWidth * 2) / 15;
	auto borderH = (itHeight * 2) / 15;

	// Create raw bitmap buffer
	std::unique_ptr<COLORREF> rawArray{ new COLORREF[itWidth * itHeight] };
	for (UINT y = 0; y < itHeight; ++y)
	{
		for (UINT x = 0; x < itWidth; ++x)
		{
			// Draw dark grey
			if (x < borderW || x >= (itWidth - borderW) || y < borderH || y >= (itHeight - borderH))
			{
				rawArray.get()[y * itWidth + x] = RGB(70, 63, 47);
			}
			// Draw light gray
			else
			{
				rawArray.get()[y * itWidth + x] = RGB(193, 184, 162);
			}
		}
	}
	hr = this->m_pRT->CreateBitmap(
		dx::SzU{ itWidth, itHeight },
		rawArray.get(),
		itWidth * sizeof(COLORREF),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX,
			this->m_dpiY
		),
		&this->m_bmps.obstacleTile
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->error(errid::D2DAssets);
		return false;
	}

	for (UINT y = 0; y < itHeight; ++y)
	{
		for (UINT x = 0; x < itWidth; ++x)
		{
			// Draw black
			if (x < borderW || x >= (itWidth - borderW) || y < borderH || y >= (itHeight - borderH))
			{
				rawArray.get()[y * itWidth + x] = RGB(29, 44, 63);
			}
			// Draw red
			else
			{
				rawArray.get()[y * itWidth + x] = RGB(255, 50, 0);
			}
		}
	}
	hr = this->m_pRT->CreateBitmap(
		dx::SzU{ itWidth, itHeight },
		rawArray.get(),
		itWidth * sizeof(COLORREF),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX,
			this->m_dpiY
		),
		&this->m_bmps.snakeBodyTile
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->error(errid::D2DAssets);
		return false;
	}

	for (UINT y = 0; y < itHeight; ++y)
	{
		for (UINT x = 0; x < itWidth; ++x)
		{
			if (x < borderW || x >= (itWidth - borderW) || y < borderH || y >= (itHeight - borderH))
			{
				continue;
			}
			// Draw yellow
			else
			{
				rawArray.get()[y * itWidth + x] = RGB(255, 187, 30);
			}
		}
	}
	hr = this->m_pRT->CreateBitmap(
		dx::SzU{ itWidth, itHeight },
		rawArray.get(),
		itWidth * sizeof(COLORREF),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX,
			this->m_dpiY
		),
		&this->m_bmps.snakeHeadTile
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->error(errid::D2DAssets);
		return false;
	}

	// Load food tile bitmaps
	std::size_t tMemSize{};
	if (!this->p_loadD2D1BitmapFromResource(
		IDB_SNAKE_FOOD_TILE1,
		dx::SzU{ itWidth, itHeight },
		this->m_bmps.snakeFoodTiles[0],
		nullptr,
		&tMemSize
	)) [[unlikely]]
	{
		this->error(errid::D2DAssets);
		return false;
	}

	std::unique_ptr<std::uint8_t> picMem{ new std::uint8_t[tMemSize] };

	
	for (std::size_t i = 1; i < this->m_bmps.snakeFoodTiles.size(); ++i)
	{
		if (!this->p_loadD2D1BitmapFromResource(
			std::uint16_t(IDB_SNAKE_FOOD_TILE1 + i),
			dx::SzU{ itWidth, itHeight },
			this->m_bmps.snakeFoodTiles[i],
			picMem.get()
		))
		{
			this->error(errid::D2DAssets);
			return false;
		}
	}

	// Create bitmap brushes
	if (!this->m_bmpBrushes.createAssets(this->m_pRT, this->m_bmps)) [[unlikely]]
	{
		this->error(errid::D2DAssetsBmBrushes);
		return false;
	}

	for (auto & i : this->m_tiles.obstacleTiles)
	{
		i.createAssets(this->m_bmpBrushes.obstacleTile);
	}
	for (auto & i : this->m_tiles.snakeBodyTiles)
	{
		i.createAssets(this->m_bmpBrushes.snakeBodyTile);
	}
	this->m_tiles.snakeHeadTile.createAssets(this->m_bmpBrushes.snakeHeadTile);
	this->m_tiles.snakeFoodTile.createAssets(this->m_bmpBrushes.randomFoodTile(this->m_rng));
	
	// Create fonts
	if (!this->m_text.createAssets(this->m_pRT, this->m_pDWriteFactory)) [[unlikely]]
	{
		this->error(errid::DWAssetsFonts);
		return false;
	}

	return true;
}
void snake::Application::destroyAssets() noexcept
{
	// Destroy font resources
	this->m_text.destroyAssets();
	
	// Reset tiles' resources
	this->m_tiles.destroyAssets();

	// Destroy bitmap brushes
	this->m_bmpBrushes.destroyAssets();
	// Destroy bitmaps
	this->m_bmps.destroyAssets();

	snake::safeRelease(this->m_pRT);
}

void snake::Application::onRender() noexcept
{
	this->createAssets();

	PAINTSTRUCT ps;
	[[maybe_unused]] auto hdc = ::BeginPaint(this->m_hwnd, &ps);

	// Begin also render target painting
	this->m_pRT->BeginDraw();

	auto scale     = D2D1::Matrix3x2F::Scale(D2D1::SizeF(this->m_factor, this->m_factor), D2D1::Point2F());
	auto translate = D2D1::Matrix3x2F::Translation(D2D1::SizeF(this->m_offsetX, this->m_offsetY));
	this->m_pRT->SetTransform(scale * translate);

	this->m_pRT->Clear(D2D1::ColorF(60.f / 255.f, 45.f / 255.f, 159.f / 255.f));

	// Get width and height
	/*auto [fwidth, fheight] = this->m_pRT->GetSize();
	auto [width, height] = POINT{ LONG(fwidth), LONG(fheight) };
	*/

	// Render scoring first
	this->m_text.onRender(this->m_tileSzF, this->m_pRT, this->m_snakeLogic.m_sInfo.scoring);
	
	
	Tile::onRender(this->m_tiles.obstacleTiles , this->m_pRT);
	this->m_tiles.snakeFoodTile.onRender(this->m_pRT);

	Tile::onRender(this->m_tiles.snakeBodyTiles, this->m_pRT);
	this->m_tiles.snakeHeadTile.onRender(this->m_pRT);

	if (this->m_pRT->EndDraw() == HRESULT(D2DERR_RECREATE_TARGET)) [[unlikely]]
		this->destroyAssets();

	::EndPaint(this->m_hwnd, &ps);
}
void snake::Application::onResize(UINT width, UINT height) noexcept
{
	if (this->m_pRT == nullptr) [[unlikely]]
	{
		return;
	}

	this->m_pRT->Resize(dx::SzU{ width, height });
	this->p_calcPositions();
}

LRESULT snake::Application::onKeyPress(WPARAM wp, LPARAM lp) noexcept
{
	Logic::direction dir{};
	switch (wp)
	{
	// Toggle game pause state
	case VK_ESCAPE:
		if (this->m_snakeLogic.m_sInfo.scoring.mode == Logic::SnakeInfo::modes::normal)
		{
			this->m_snakeLogic.m_sInfo.scoring.paused ^= 1;
			// Update screen
			::InvalidateRect(this->m_hwnd, nullptr, FALSE);
			return 0;
		}
		break;
	case VK_RETURN:
		if (this->m_snakeLogic.m_sInfo.scoring.mode != Logic::SnakeInfo::modes::normal || this->m_snakeLogic.m_sInfo.scoring.paused)
		{
			this->restartGame();
			return 0;
		}
		break;
	case VK_F11:
		// Act only if key was just pressed down, prevents spamming
		if ((lp & 0x40000000) == 0)
		{
			this->p_toggleFullScreen();
		}
		break;
	}

	if (!this->m_snakeLogic.m_sInfo.scoring.paused)
	{
		switch (wp)
		{
		case VK_LEFT:
			dir = Logic::direction::left;
			break;
		case VK_RIGHT:
			dir = Logic::direction::right;
			break;
		case VK_UP:
			dir = Logic::direction::up;
			break;
		case VK_DOWN:
			dir = Logic::direction::down;
			break;
		default:
			return 0;
		}
		// Check if the user wants to make a valid move
		if (dir != this->m_snakeLogic.m_sInfo.scoring.snakeDir &&
			enumIsClose(enumDiff(dir, this->m_snakeLogic.m_sInfo.scoring.snakeDir), Logic::s_direction_enum_size))
		{
			this->m_snakeLogic.changeDirection(dir);
			this->m_snakeLogic.stepNow();
		}
	}

	return 0;
}
LRESULT snake::Application::onKeyRelease(WPARAM wp, LPARAM lp) noexcept
{
	return ::DefWindowProcW(this->m_hwnd, WM_KEYUP, wp, lp);
}

snake::Tile snake::Application::makeRandomSnakeTile() noexcept
{
	static std::uniform_int_distribution<long> wDist(0, long(this->s_fieldWidth) - 1), hDist(0, long(this->s_fieldHeight) - 1);
	return Tile(
		this->m_tileSzF,
		this->calcToTile(wDist(this->m_rng), hDist(this->m_rng)),
		dx::SzU{ 1, 1 }
	);
}
snake::Tile snake::Application::makeSnakeTile(long cx, long cy) const noexcept
{
	return Tile(
		this->m_tileSzF,
		this->calcToTile(cx, cy),
		dx::SzU{ 1, 1 }
	);
}
void snake::Application::makeSnakeTile(Tile & t, long cx, long cy) const noexcept
{
	t = static_cast<Tile const &>(Tile(
		this->m_tileSzF,
		this->calcToTile(cx, cy),
		dx::SzU{ 1, 1 }
	));
}
void snake::Application::moveTile(Tile & t, long cx, long cy) const noexcept
{
	t.move(this->calcToTile(cx, cy));
}

void snake::Application::initSnakeData()
{
	this->m_tiles.snakeBodyTiles.clear();

	this->makeSnakeTile(this->m_tiles.snakeHeadTile, 46, 25);
	this->m_tiles.snakeBodyTiles.emplace_back(this->makeSnakeTile(47, 25));
	this->m_tiles.snakeBodyTiles.emplace_back(this->makeSnakeTile(48, 25));
	this->m_tiles.snakeBodyTiles.emplace_back(this->makeSnakeTile(49, 25));
	this->m_tiles.snakeBodyTiles.emplace_back(this->makeSnakeTile(50, 25));

	for (auto & i : this->m_tiles.snakeBodyTiles)
	{
		i.createAssets(this->m_bmpBrushes.snakeBodyTile);
	}

	this->genFood(this->m_tiles.snakeFoodTile);
}

void snake::Application::genFood(snake::Tile & output) noexcept
{
	auto collides = [this](snake::Tile const& output) noexcept -> bool
	{
		auto outBounds = output.getBounds();
		if (this->m_tiles.snakeHeadTile.collides(outBounds))
			return true;
		
		for (auto const & i : this->m_tiles.obstacleTiles)
		{
			if (i.collides(outBounds))
				return true;
		}
		for (auto const & i : this->m_tiles.snakeBodyTiles)
		{
			if (i.collides(outBounds))
				return true;
		}

		return false;
	};

	do
	{
		output = this->makeRandomSnakeTile();
	} while (collides(output));
	output.destroyAssets();
	output.createAssets(this->m_bmpBrushes.randomFoodTile(this->m_rng));
}

void snake::Application::restartGame()
{
	this->initSnakeData();
	this->m_snakeLogic.resetScoring();
	::InvalidateRect(this->m_hwnd, nullptr, FALSE);
}

void snake::Application::playSnd(std::uint16_t rsc) const noexcept
{
	snake::playSndRsc(rsc, this->m_hInst);
}
void snake::Application::playSndAsync(std::uint16_t rsc) const noexcept
{
	snake::playSndRscAsync(rsc, this->m_hInst);
}
