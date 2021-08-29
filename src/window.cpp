#include "window.hpp"

#include <tgmath.h>

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
				snake::Application::sError(L"Error initialising window!");
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
		This->p_calcBorder();
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

void snake::Application::p_calcBorder() noexcept
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
}

snake::Application::Application(PSTR cmdArgs) noexcept
	: m_cmdArgs(cmdArgs)
{}
snake::Application::~Application() noexcept
{
	this->DestroyAssets();
	snake::SafeRelease(this->m_pD2DFactory);
}

bool snake::Application::Init(HINSTANCE hInst, int nCmdShow)
{
	// Init factory
	auto hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		IID_ID2D1Factory,
		reinterpret_cast<void **>(&this->m_pD2DFactory)
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->Error(errid::D2DFactory);
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
		WS_OVERLAPPEDWINDOW ^ (/*WS_SIZEBOX |*/ WS_MAXIMIZEBOX),
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

	this->p_calcBorder();
	::SetWindowPos(
		this->m_hwnd,
		nullptr,
		0,
		0,
		this->m_minSize.x,
		this->m_minSize.y,
		SWP_NOZORDER | SWP_NOMOVE
	);

	D2D1_SIZE_F tileSzF;
	{
		auto [rX, rY] = this->dipxy<D2D1_SIZE_F>(tileSz, tileSz);
		float dX = rX - float(long(rX)), dY = rY - float(long(rY));
		tileSzF = D2D1::SizeF(tileSz - this->revdipx(dX), tileSz - this->revdipy(dY));
	}

	this->m_obstacleTiles.emplace_back(
		tileSzF,
		D2D1::SizeF(0.f, 0.f),
		D2D1::SizeU(6, 1)
	);
	this->m_obstacleTiles.emplace_back(
		tileSzF,
		D2D1::SizeF(0.f, 0.f),
		D2D1::SizeU(1, 6)
	);
	this->m_obstacleTiles.emplace_back(
		tileSzF,
		D2D1::SizeF(tileSzF.width * (this->fieldWidth - 6.f), 0.f),
		D2D1::SizeU(6, 1)
	);
	this->m_obstacleTiles.emplace_back(
		tileSzF,
		D2D1::SizeF(tileSzF.width * (this->fieldWidth - 1.f), 0.f),
		D2D1::SizeU(1, 6)
	);

	this->m_snakeBodyTiles.emplace_back(
		tileSzF,
		D2D1::SizeF(tileSzF.width * 10.f, tileSzF.height),
		D2D1::SizeU(10, 35)
	);
	this->m_snakeHeadTile = tile(
		tileSzF,
		D2D1::SizeF(tileSzF.width * 20.f, tileSzF.height * 2.f),
		D2D1::SizeU(1, 1)
	);


	// Create render target
	if (!this->CreateAssets()) [[unlikely]]
	{
		return false;
	}

	::ShowWindow(this->m_hwnd, nCmdShow);
	::UpdateWindow(this->m_hwnd);

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
			this->sError(L"Unknown error occurred!");
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

	hr = this->m_pRT->CreateBitmap(
		D2D1::SizeU(itWidth, itHeight),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX,
			this->m_dpiY
		),
		&this->m_pObstacleTileBm
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->Error(errid::D2DAssets);
		return false;
	}

	hr = this->m_pRT->CreateBitmap(
		D2D1::SizeU(itWidth, itHeight),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX,
			this->m_dpiY
		),
		&this->m_pSnakeBodyTileBm
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->Error(errid::D2DAssets);
		return false;
	}

	hr = this->m_pRT->CreateBitmap(
		D2D1::SizeU(itWidth, itHeight),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX,
			this->m_dpiY
		),
		&this->m_pSnakeHeadTileBm
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->Error(errid::D2DAssets);
		return false;
	}

	hr = this->m_pRT->CreateBitmap(
		D2D1::SizeU(itWidth, itHeight),
		D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			this->m_dpiX,
			this->m_dpiY
		),
		&this->m_pSnakeFoodTileBm
	);
	if (FAILED(hr)) [[unlikely]]
	{
		this->Error(errid::D2DAssets);
		return false;
	}

	// Create raw bitmap buffer
	auto rawArray = new COLORREF[itWidth * itHeight];
	for (UINT y = 0; y < itHeight; ++y)
	{
		for (UINT x = 0; x < itWidth; ++x)
		{
			// Draw dark grey
			if (x < borderW || x >= (itWidth - borderW) || y < borderH || y >= (itHeight - borderH))
			{
				rawArray[y * itWidth + x] = RGB(63, 63, 63);
			}
			// Draw light gray
			else
			{
				rawArray[y * itWidth + x] = RGB(127, 127, 127);
			}
		}
	}
	this->m_pObstacleTileBm->CopyFromMemory(nullptr, rawArray, itWidth * sizeof(COLORREF));

	for (UINT y = 0; y < itHeight; ++y)
	{
		for (UINT x = 0; x < itWidth; ++x)
		{
			// Draw black
			if (x < borderW || x >= (itWidth - borderW) || y < borderH || y >= (itHeight - borderH))
			{
				rawArray[y * itWidth + x] = RGB(0, 0, 0);
			}
			// Draw red
			else
			{
				rawArray[y * itWidth + x] = RGB(255, 0, 0);
			}
		}
	}
	this->m_pSnakeBodyTileBm->CopyFromMemory(nullptr, rawArray, itWidth * sizeof(COLORREF));

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
				rawArray[y * itWidth + x] = RGB(255, 255, 0);
			}
		}
	}
	this->m_pSnakeHeadTileBm->CopyFromMemory(nullptr, rawArray, itWidth * sizeof(COLORREF));

	delete[] rawArray;



	for (auto & i : this->m_obstacleTiles)
	{
		if (!i.CreateAssets(this->m_pRT, this->m_pObstacleTileBm)) [[unlikely]]
			return false;
	}
	for (auto & i : this->m_snakeBodyTiles)
	{
		if (!i.CreateAssets(this->m_pRT, this->m_pSnakeBodyTileBm)) [[unlikely]]
			return false;
	}
	if (!this->m_snakeHeadTile.CreateAssets(this->m_pRT, this->m_pSnakeHeadTileBm)) [[unlikely]]
		return false;

	/*if (!this->m_snakeFoodTile.CreateAssets(this->m_pRT, this->m_pSnakeFoodTileBm)) [[unlikely]]
		return false;
	*/

	return true;
}
void snake::Application::DestroyAssets() noexcept
{
	for (auto & i : this->m_obstacleTiles)
		i.DestroyAssets();
	for (auto & i : this->m_snakeBodyTiles)
		i.DestroyAssets();
	this->m_snakeHeadTile.DestroyAssets();
	this->m_snakeFoodTile.DestroyAssets();

	snake::SafeRelease(this->m_pRT);
}

void snake::Application::OnRender() noexcept
{
	this->CreateAssets();

	PAINTSTRUCT ps;
	[[maybe_unused]] auto hdc = ::BeginPaint(this->m_hwnd, &ps);

	// Begin also render target painting
	this->m_pRT->BeginDraw();
	this->m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
	this->m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

	// Get width and height
	/*auto [fwidth, fheight] = this->m_pRT->GetSize();
	auto [width, height] = POINT{ LONG(fwidth), LONG(fheight) };
	*/

	tile::OnRender(this->m_obstacleTiles, this->m_pRT);
	tile::OnRender(this->m_snakeBodyTiles, this->m_pRT);
	this->m_snakeHeadTile.OnRender(this->m_pRT);
	//this->m_snakeFoodTile.OnRender(this->m_pRT);

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
