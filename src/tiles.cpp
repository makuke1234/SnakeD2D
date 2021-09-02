#include "tiles.hpp"

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

bool snake::tile::CreateAssets(ID2D1HwndRenderTarget * pRT, ID2D1Bitmap * pBitmap) noexcept
{
	if (this->m_pBmBrush)
		return true;

	auto hr = pRT->CreateBitmapBrush(
		pBitmap,
		&this->m_pBmBrush
	);
	if (FAILED(hr)) [[unlikely]]
		return false;

	// Set extend mode to wrap / "tile"
	this->m_pBmBrush->SetExtendModeX(D2D1_EXTEND_MODE_WRAP);
	this->m_pBmBrush->SetExtendModeY(D2D1_EXTEND_MODE_WRAP);

	return true;
}
void snake::tile::DestroyAssets() noexcept
{
	snake::SafeRelease(this->m_pBmBrush);
}

void snake::tile::OnRender(ID2D1HwndRenderTarget * pRT) noexcept
{
	pRT->FillRectangle(
		this->m_tilesRect,
		this->m_pBmBrush
	);
}

[[nodiscard]] D2D1_SIZE_U snake::tile::getCoords(D2D1_SIZE_F const & tileSz) const noexcept
{
	return D2D1::SizeU(this->m_tilesRect.left / tileSz.width + 0.5f, this->m_tilesRect.top / tileSz.height + 0.5f);
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

