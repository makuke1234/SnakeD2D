#include "tiles.hpp"
#include "window.hpp"

snake::Tile::Tile(dx::SzF const & tileSz, dx::SzF const & startpos, dx::SzU const & numTiles) noexcept
	: m_tilesRect{
		.left   = startpos.width,
		.top    = startpos.height,
		.right  = startpos.width  + tileSz.width  * dx::F(numTiles.width),
		.bottom = startpos.height + tileSz.height * dx::F(numTiles.height)
	}
{}
snake::Tile::~Tile() noexcept
{
	this->destroyAssets();
}

[[nodiscard]] dx::BmpBrush * snake::Tile::createBmpBrush(dx::HwndRT * pRT, dx::Bmp * pBm) noexcept
{
	dx::BmpBrush * bmBrush{ nullptr };
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
void snake::Tile::createAssets(dx::BmpBrush * pBmBrush) noexcept
{
	if (this->m_pBmBrush != nullptr)
		return;
	this->m_pBmBrush = pBmBrush;	
}
void snake::Tile::destroyAssets() noexcept
{
	this->m_pBmBrush = nullptr;
}

void snake::Tile::onRender(dx::HwndRT * pRT) const noexcept
{
	pRT->FillRectangle(
		this->m_tilesRect,
		this->m_pBmBrush
	);
}

[[nodiscard]] dx::SzU snake::Tile::getCoords(dx::SzF const & tileSz) const noexcept
{
	return snake::Application::s_calcFromTile(tileSz, this->m_tilesRect.left, this->m_tilesRect.top);
}
void snake::Tile::move(dx::SzF const & newpos) noexcept
{
	auto xdelta = this->m_tilesRect.right  - this->m_tilesRect.left,
	     ydelta = this->m_tilesRect.bottom - this->m_tilesRect.top;

	this->m_tilesRect.left   = newpos.width;
	this->m_tilesRect.top    = newpos.height;
	this->m_tilesRect.right  = newpos.width  + xdelta;
	this->m_tilesRect.bottom = newpos.height + ydelta;
}
