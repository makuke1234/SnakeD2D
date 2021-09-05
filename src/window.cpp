#include "window.hpp"
#include "resource.h"

#include <tgmath.h>
#include <memory>

LRESULT CALLBACK snake::Application::wproc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) noexcept
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
				snake::Application::sError(errid::Window);
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
		This->OnRender();
		break;
	case WM_KEYDOWN:
		return This->OnKeyPress(wp, lp);
	case WM_KEYUP:
		return This->OnKeyRelease(wp, lp);
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
		This->OnResize(LOWORD(lp), HIWORD(lp));
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

void snake::Application::tilesStruct::DestroyAssets() noexcept
{
	for (auto & i : this->obstacleTiles)
		i.DestroyAssets();
	for (auto & i : this->snakeBodyTiles)
		i.DestroyAssets();
	this->snakeHeadTile.DestroyAssets();
	this->snakeFoodTile.DestroyAssets();
}

void snake::Application::bitmapsStruct::DestroyAssets() noexcept
{
	snake::safeRelease(this->obstacleTile);
	snake::safeRelease(this->snakeBodyTile);
	snake::safeRelease(this->snakeHeadTile);
	for (auto & i : this->snakeFoodTiles)
	{
		snake::safeRelease(i);
	}
}

bool snake::Application::bitmapBrushesStruct::CreateAssets(
	ID2D1HwndRenderTarget * pRT,
	Application::bitmapsStruct const & bmps
) noexcept
{
	if ((this->obstacleTile = tile::CreateBmBrush(pRT, bmps.obstacleTile)) == nullptr) [[unlikely]]
		return false;
	if ((this->snakeBodyTile = tile::CreateBmBrush(pRT, bmps.snakeBodyTile)) == nullptr) [[unlikely]]
		return false;
	if ((this->snakeHeadTile = tile::CreateBmBrush(pRT, bmps.snakeHeadTile)) == nullptr) [[unlikely]]
		return false;
	for (std::size_t i = 0; i < this->snakeFoodTiles.size(); ++i)
	{
		if ((this->snakeFoodTiles[i] = tile::CreateBmBrush(pRT, bmps.snakeFoodTiles[i])) == nullptr) [[unlikely]]
			return false;
	}

	return true;
}
void snake::Application::bitmapBrushesStruct::DestroyAssets() noexcept
{
	snake::safeRelease(this->obstacleTile);
	snake::safeRelease(this->snakeBodyTile);
	snake::safeRelease(this->snakeHeadTile);
	for (auto & i : this->snakeFoodTiles)
	{
		snake::safeRelease(i);
	}
}

bool snake::Application::textStruct::CreateAssets(ID2D1HwndRenderTarget * pRT, IDWriteFactory * pWF) noexcept
{
	auto hr = pWF->CreateTextFormat(
		L"Consolas",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16,
		L"",
		&this->consolas16
	);
	if (FAILED(hr)) [[unlikely]]
		return false;

	hr = pRT->CreateSolidColorBrush(
		D2D1::ColorF(63.f / 255.f, 127.f / 255.f, 127.f / 255.f),
		&this->pTextBrush
	);
	if (FAILED(hr)) [[unlikely]]
		return false;

	return true;
}
void snake::Application::textStruct::DestroyAssets() noexcept
{
	snake::safeRelease(this->pTextBrush);
	snake::safeRelease(this->consolas16);
}

void snake::Application::textStruct::OnRender(D2D1_SIZE_F const & tileSz, ID2D1HwndRenderTarget * pRT) const noexcept
{
	// Draw example text
	constexpr std::wstring_view testT{ L"test text" };

	auto ltop{ Application::s_calcTile(tileSz, 10, 10) };
	auto rbottom{ Application::s_calcTile(tileSz, 30, 13) };

	pRT->DrawTextW(
		testT.data(),
		testT.size(),
		this->consolas16,
		D2D1::RectF(ltop.width, ltop.height, rbottom.width, rbottom.height),
		this->pTextBrush
	);
}

void snake::Application::p_calcDpiSpecific() noexcept
{
	RECT clientR{}, windowR{};
	::GetClientRect(this->m_hwnd, &clientR);
	::GetWindowRect(this->m_hwnd, &windowR);
	this->m_border = D2D1::SizeU(
		(windowR.right  - windowR.left) - (clientR.right  - clientR.left),
		(windowR.bottom - windowR.top)  - (clientR.bottom - clientR.top)
	);

	this->m_minSize.x = ceil(this->dipxi<float>(tileSz, this->fieldWidth)  + this->m_border.width);
	this->m_minSize.y = ceil(this->dipyi<float>(tileSz, this->fieldHeight) + this->m_border.height);

	auto [rX, rY] = this->dipxy<D2D1_SIZE_F>(tileSz, tileSz);
	float dX = rX - float(long(rX)), dY = rY - float(long(rY));
	this->m_tileSzF = D2D1::SizeF(tileSz - this->revdipx(dX), tileSz - this->revdipy(dY));
}
bool snake::Application::p_loadD2D1BitmapFromResource(
	LPCWSTR resourceId,
	D2D1_SIZE_U const & bmSize,
	ID2D1Bitmap *& bmRef,
	void * opBuf,
	std::size_t * bufSize
) noexcept
{
	std::unique_ptr<COLORREF> ourBuf;
	COLORREF * accessBuf{ nullptr };

	HBITMAP hBmp = static_cast<HBITMAP>(::LoadImageW(
		this->m_hInst, resourceId, IMAGE_BITMAP,
		0, 0, 0
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

	bmpH.biSizeImage = sizeof(COLORREF) * bmp.bmWidth * bmp.bmHeight;

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
		D2D1::SizeU(bmp.bmWidth, bmp.bmHeight),
		accessBuf,
		sizeof(COLORREF) * bmp.bmWidth,
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX * bmp.bmWidth  / bmSize.width,
			this->m_dpiY * bmp.bmHeight / bmSize.height
		),
		&bmRef
	);

	if (FAILED(hr)) [[unlikely]]
		return false;


	return true;
}

snake::Application::Application(PSTR cmdArgs) noexcept
	: m_cmdArgs(cmdArgs)
{}
snake::Application::~Application() noexcept
{
	this->DestroyAssets();
	snake::safeRelease(this->m_pDWriteFactory);
	snake::safeRelease(this->m_pD2DFactory);
}

bool snake::Application::Init(HINSTANCE hInst, int nCmdShow)
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
		this->Error(errid::D2DFactory);
		return false;
	}

	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(this->m_pDWriteFactory),
		static_cast<IUnknown **>(static_cast<void *>(&this->m_pDWriteFactory))
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->Error(errid::DWriteFactory);
		return false;
	}

	// Initialise application class
	WNDCLASSEXW w{};

	w.cbSize = sizeof w;
	w.style = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc = &this->wproc;
	w.hInstance = hInst;
	w.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
	w.hIconSm = w.hIcon = ::LoadIconW(hInst, IDI_APPLICATION);
	w.lpszClassName = this->className.data();
	
	if (!::RegisterClassExW(&w)) [[unlikely]]
	{
		this->Error(errid::AppClass);
		return false;
	}

	// Get DPI
	m_pD2DFactory->GetDesktopDpi(&this->m_dpiX, &this->m_dpiY);

	// Create window
	this->m_hwnd = ::CreateWindowExW(
		0,
		this->className.data(),
		this->applicationName.data(),
		WS_OVERLAPPEDWINDOW ^ (WS_SIZEBOX | WS_MAXIMIZEBOX),
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
		this->Error(errid::Window);
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
		D2D1::SizeF(0.f, 0.f),
		D2D1::SizeU(6, 1)
	);
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		D2D1::SizeF(0.f, this->m_tileSzF.height),
		D2D1::SizeU(1, 5)
	);

	// Upper right
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		D2D1::SizeF(this->m_tileSzF.width * (this->fieldWidth - 6.f), 0.f),
		D2D1::SizeU(6, 1)
	);
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		D2D1::SizeF(this->m_tileSzF.width * (this->fieldWidth - 1.f), this->m_tileSzF.height),
		D2D1::SizeU(1, 5)
	);

	// Bottom left
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		D2D1::SizeF(0.f, this->m_tileSzF.height * (this->fieldHeight - 1.f)),
		D2D1::SizeU(6, 1)
	);
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		D2D1::SizeF(0.f, this->m_tileSzF.height * (this->fieldHeight - 6.f)),
		D2D1::SizeU(1, 5)
	);

	// Bottom right
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		D2D1::SizeF(
			this->m_tileSzF.width  * (this->fieldWidth  - 6.f),
			this->m_tileSzF.height * (this->fieldHeight - 1.f)
		),
		D2D1::SizeU(6, 1)
	);
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		D2D1::SizeF(
			this->m_tileSzF.width  * (this->fieldWidth  - 1.f),
			this->m_tileSzF.height * (this->fieldHeight - 6.f)
		),
		D2D1::SizeU(1, 5)
	);

	// Center piece
	this->m_tiles.obstacleTiles.emplace_back(
		this->m_tileSzF,
		D2D1::SizeF(
			this->m_tileSzF.width  * float(long(this->fieldWidth  - 8.f) / 2),
			this->m_tileSzF.height * float(long(this->fieldHeight - 6.f) / 2)
		),
		D2D1::SizeU(8, 6)
	);

	// Initialize snake data structure
	this->initSnakeData();

	// Create render target
	if (!this->CreateAssets()) [[unlikely]]
	{
		return false;
	}


	::ShowWindow(this->m_hwnd, nCmdShow);
	::UpdateWindow(this->m_hwnd);

	if (!this->m_snakeLogic.startSnakeLoop()) [[unlikely]]
	{
		return false;
	}

	return true;
}


int snake::Application::MsgLoop() noexcept
{
	MSG msg;
	BOOL br;
	while ((br = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (br == -1) [[unlikely]]
		{
			this->sError(errid::Unknown);
			return -1;
		}
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}
	return int(msg.wParam);
}

void snake::Application::Error(const wchar_t * str) const noexcept
{
	::MessageBoxW(
		this->m_hwnd,
		str,
		this->applicationName.data(),
		MB_ICONERROR | MB_OK
	);
}
void snake::Application::sError(const wchar_t * str) noexcept
{
	::MessageBoxW(nullptr, str, Application::applicationName.data(), MB_ICONERROR | MB_OK);
}

void snake::Application::Error(errid id) const noexcept
{
	using eT = std::underlying_type_t<errid>;

	auto oid = eT(id);
	if (oid >= eT(errid::errid_enum_size)) [[unlikely]]
		oid = eT(errid::Unknown);
	
	::MessageBoxW(
		this->m_hwnd,
		this->s_errorIds[oid],
		this->applicationName.data(),
		MB_ICONERROR | MB_OK
	);
}
void snake::Application::sError(errid id) noexcept
{
	using eT = std::underlying_type_t<errid>;
	
	auto oid = eT(id);
	if (oid >= eT(errid::errid_enum_size)) [[unlikely]]
		oid = eT(errid::Unknown);

	::MessageBoxW(
		nullptr,
		snake::Application::s_errorIds[oid], 
		Application::applicationName.data(),
		MB_ICONERROR | MB_OK
	);
}

bool snake::Application::CreateAssets() noexcept
{
	// Create Render target
	if (this->m_pRT)
		return true;

	RECT r;
	::GetClientRect(this->m_hwnd, &r);
	D2D1_SIZE_U size = D2D1::SizeU(r.right - r.left, r.bottom - r.top);
	auto hr = this->m_pD2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(this->m_hwnd, size),
		&this->m_pRT
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->Error(errid::D2DRT);
		return false;
	}

	// Create assets
	
	// Create bitmaps

	auto [itWidth, itHeight] = this->dipxyi<D2D1_SIZE_U>(tileSz, tileSz);
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
		D2D1::SizeU(itWidth, itHeight),
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
		this->Error(errid::D2DAssets);
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
		D2D1::SizeU(itWidth, itHeight),
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
		this->Error(errid::D2DAssets);
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
		D2D1::SizeU(itWidth, itHeight),
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
		this->Error(errid::D2DAssets);
		return false;
	}

	// Load food tile bitmaps
	std::size_t tMemSize{};
	if (!this->p_loadD2D1BitmapFromResource(
		MAKEINTRESOURCEW(IDB_SNAKE_FOOD_TILE1),
		D2D1::SizeU(itWidth, itHeight),
		this->m_bmps.snakeFoodTiles[0],
		nullptr,
		&tMemSize
	)) [[unlikely]]
	{
		this->Error(errid::D2DAssets);
		return false;
	}

	std::unique_ptr<COLORREF> picMem{ new COLORREF[tMemSize / sizeof(COLORREF)] };

	
	for (std::size_t i = 1; i < this->m_bmps.snakeFoodTiles.size(); ++i)
	{
		if (!this->p_loadD2D1BitmapFromResource(
			MAKEINTRESOURCEW(IDB_SNAKE_FOOD_TILE1 + i),
			D2D1::SizeU(itWidth, itHeight),
			this->m_bmps.snakeFoodTiles[i],
			picMem.get()
		))
		{
			this->Error(errid::D2DAssets);
			return false;
		}
	}

	// Create bitmap brushes
	if (!this->m_bmpBrushes.CreateAssets(this->m_pRT, this->m_bmps)) [[unlikely]]
	{
		this->Error(errid::D2DAssetsBmBrushes);
		return false;
	}

	for (auto & i : this->m_tiles.obstacleTiles)
	{
		i.CreateAssets(this->m_bmpBrushes.obstacleTile);
	}
	for (auto & i : this->m_tiles.snakeBodyTiles)
	{
		i.CreateAssets(this->m_bmpBrushes.snakeBodyTile);
	}
	this->m_tiles.snakeHeadTile.CreateAssets(this->m_bmpBrushes.snakeHeadTile);

	this->m_tiles.snakeFoodTile.CreateAssets(this->m_bmpBrushes.snakeFoodTiles[5]);
	
	// Create fonts
	if (!this->m_text.CreateAssets(this->m_pRT, this->m_pDWriteFactory)) [[unlikely]]
	{
		this->Error(errid::DWAssetsFonts);
		return false;
	}

	return true;
}
void snake::Application::DestroyAssets() noexcept
{
	// Destroy font resources
	this->m_text.DestroyAssets();
	
	// Reset tiles' resources
	this->m_tiles.DestroyAssets();

	// Destroy bitmap brushes
	this->m_bmpBrushes.DestroyAssets();
	// Destroy bitmaps
	this->m_bmps.DestroyAssets();

	snake::safeRelease(this->m_pRT);
}

void snake::Application::OnRender() noexcept
{
	this->CreateAssets();

	PAINTSTRUCT ps;
	[[maybe_unused]] auto hdc = ::BeginPaint(this->m_hwnd, &ps);

	// Begin also render target painting
	this->m_pRT->BeginDraw();
	this->m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
	this->m_pRT->Clear(D2D1::ColorF(60.f / 255.f, 45.f / 255.f, 159.f / 255.f));

	// Get width and height
	/*auto [fwidth, fheight] = this->m_pRT->GetSize();
	auto [width, height] = POINT{ LONG(fwidth), LONG(fheight) };
	*/

	tile::OnRender(this->m_tiles.obstacleTiles , this->m_pRT);
	// Render food tile first
	this->m_tiles.snakeFoodTile.OnRender(this->m_pRT);

	tile::OnRender(this->m_tiles.snakeBodyTiles, this->m_pRT);
	this->m_tiles.snakeHeadTile.OnRender(this->m_pRT);

	this->m_text.OnRender(this->m_tileSzF, this->m_pRT);

	if (this->m_pRT->EndDraw() == HRESULT(D2DERR_RECREATE_TARGET)) [[unlikely]]
		this->DestroyAssets();

	::EndPaint(this->m_hwnd, &ps);
}
void snake::Application::OnResize(UINT width, UINT height) const noexcept
{
	if (this->m_pRT) [[likely]]
	{
		this->m_pRT->Resize(D2D1::SizeU(width, height));
	}
}

LRESULT snake::Application::OnKeyPress(WPARAM wp, [[maybe_unused]] LPARAM lp) noexcept
{
	logic::direction dir{};
	switch (wp)
	{
	case VK_LEFT:
		dir = logic::direction::left;
		break;
	case VK_RIGHT:
		dir = logic::direction::right;
		break;
	case VK_UP:
		dir = logic::direction::up;
		break;
	case VK_DOWN:
		dir = logic::direction::down;
		break;
	default:
		return 0;
	}

	// Check if the user wants to make a valid move
	if (dir != this->m_snakeLogic.m_sInfo.scoring.snakeDir &&
		enumIsClose(enumDiff(dir, this->m_snakeLogic.m_sInfo.scoring.snakeDir), logic::direction_enum_size))
	{
		this->m_snakeLogic.changeDirection(dir);
		this->m_snakeLogic.stepNow();
	}

	return 0;
}
LRESULT snake::Application::OnKeyRelease(WPARAM wp, LPARAM lp) noexcept
{
	return ::DefWindowProcW(this->m_hwnd, WM_KEYUP, wp, lp);
}

snake::tile snake::Application::makeSnakeTile(long cx, long cy) const noexcept
{
	return tile(
		this->m_tileSzF,
		this->calcTile(cx, cy),
		D2D1::SizeU(1, 1)
	);
}
void snake::Application::moveTile(tile & t, long cx, long cy) const noexcept
{
	t.move(this->calcTile(cx, cy));
}

void snake::Application::initSnakeData()
{
	this->m_tiles.snakeBodyTiles.clear();

	this->m_tiles.snakeHeadTile = this->makeSnakeTile(46, 25);
	this->m_tiles.snakeBodyTiles.emplace_back(this->makeSnakeTile(47, 25));
	this->m_tiles.snakeBodyTiles.emplace_back(this->makeSnakeTile(48, 25));
	this->m_tiles.snakeBodyTiles.emplace_back(this->makeSnakeTile(49, 25));
	this->m_tiles.snakeBodyTiles.emplace_back(this->makeSnakeTile(50, 25));

	this->m_tiles.snakeFoodTile = this->makeSnakeTile(20, 19);
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
