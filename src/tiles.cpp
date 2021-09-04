#include "tiles.hpp"
#include "window.hpp"

snake::tile::tile(D2D1_SIZE_F const & tileSz, D2D1_SIZE_F const & startpos, D2D1_SIZE_U const & numTiles) noexcept
	: m_tilesRect(D2D1::RectF(
		startpos.width,
		startpos.height,
		startpos.width + tileSz.width * FLOAT(numTiles.width),
		startpos.height + tileSz.height * FLOAT(numTiles.height)
	))
{}
snake::tile::~tile() noexcept
{
	this->DestroyAssets();
}

[[nodiscard]] ID2D1BitmapBrush * snake::tile::CreateBmBrush(ID2D1HwndRenderTarget * pRT, ID2D1Bitmap * pBm) noexcept
{
	ID2D1BitmapBrush * bmBrush{ nullptr };
	auto hr = pRT->CreateBitmapBrush(
		pBm,
		&bmBrush
	);

	if (FAILED(hr)) [[unlikely]]
		return nullptr;

	bmBrush->SetExtendModeX(D2D1_EXTEND_MODE_WRAP);
	bmBrush->SetExtendModeY(D2D1_EXTEND_MODE_WRAP);

	return bmBrush;
}
void snake::tile::CreateAssets(ID2D1BitmapBrush * pBmBrush) noexcept
{
	if (this->m_pBmBrush != nullptr)
		return;
	this->m_pBmBrush = pBmBrush;	
}
void snake::tile::DestroyAssets() noexcept
{
	this->m_pBmBrush = nullptr;
}

void snake::tile::OnRender(ID2D1HwndRenderTarget * pRT) const noexcept
{
	pRT->FillRectangle(
		this->m_tilesRect,
		this->m_pBmBrush
	);
}

[[nodiscard]] D2D1_SIZE_U snake::tile::getCoords(D2D1_SIZE_F const & tileSz) const noexcept
{
	return snake::Application::s_revcalcTile(tileSz, this->m_tilesRect.left, this->m_tilesRect.top);
}
void snake::tile::move(D2D1_SIZE_F const & newpos) noexcept
{
	auto xdelta = this->m_tilesRect.right  - this->m_tilesRect.left,
		 ydelta = this->m_tilesRect.bottom - this->m_tilesRect.top;

	this->m_tilesRect.left   = newpos.width;
	this->m_tilesRect.top    = newpos.height;
	this->m_tilesRect.right  = newpos.width  + xdelta;
	this->m_tilesRect.bottom = newpos.height + ydelta;
}
