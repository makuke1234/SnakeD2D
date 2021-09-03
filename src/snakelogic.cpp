#include "snakelogic.hpp"
#include "window.hpp"

DWORD WINAPI snake::logic::sp_snakeLoopThread(LPVOID lp) noexcept
{
	auto ti = static_cast<p_snakeLoopThreadInfo *>(lp);

	while (!ti->end)
	{
		while (ti->time > 0.f)
		{
			Sleep(50);
			ti->time -= .05f;
		}
		ti->This.moveSnake();
		::InvalidateRect(ti->This.m_parentRef.m_hwnd, nullptr, FALSE);
		ti->time = ti->curTime;
	}

	ti->hThread = nullptr;
	return 0;
}

snake::logic::logic(Application & parentRef) noexcept
	: m_parentRef(parentRef)
{}

void snake::logic::changeDirection(direction newdir) noexcept
{
	this->m_snakeDirection = newdir;
}
void snake::logic::moveSnake() const noexcept
{

	auto prevtile = this->m_parentRef.m_tiles.snakeHeadTile;
	auto [uposx, uposy] = prevtile.getCoords(this->m_parentRef.m_tileSzF);
	std::intptr_t posx{ std::intptr_t(uposx) }, posy{ std::intptr_t(uposy) };
	
	switch (this->m_snakeDirection)
	{
	case direction::left:
		--posx;
		break;
	case direction::up:
		--posy;
		break;
	case direction::right:
		++posx;
		break;
	case direction::down:
		++posy;
		break;
	}

	constexpr auto ipWidth{ std::intptr_t(this->m_parentRef.fieldWidth) },
		ipHeight{ std::intptr_t(this->m_parentRef.fieldHeight) };
	// Make sure that position stays in bounds
	if (posx < 0)
		posx += ipWidth;
	else if (posx >= ipWidth)
		posx -= ipWidth;

	if (posy < 0)
		posy += ipHeight;
	else if (posy >= ipHeight)
		posy -= ipHeight;

	this->m_parentRef.moveTile(this->m_parentRef.m_tiles.snakeHeadTile, posx, posy);
	this->m_parentRef.m_tiles.snakeBodyTiles.emplace_front(std::move(this->m_parentRef.m_tiles.snakeBodyTiles.back()));
	this->m_parentRef.m_tiles.snakeBodyTiles.pop_back();
	this->m_parentRef.m_tiles.snakeBodyTiles.front() = prevtile;
}
void snake::logic::moveAndGrowSnake() const
{
	auto tail{ this->m_parentRef.m_tiles.snakeBodyTiles.back() };
	tail.CreateAssets(this->m_parentRef.m_bmpBrushes.snakeBodyTile);
	this->moveSnake();
	this->m_parentRef.m_tiles.snakeBodyTiles.emplace_back(std::move(tail));
}

bool snake::logic::startSnakeLoop() noexcept
{
	if (this->m_ti.hThread != nullptr)
		return true;
	else
	{
		this->m_ti.end = false;
		this->m_ti.hThread = ::CreateThread(
			nullptr,
			this->p_snakeLoopThreadStackSize,
			&this->sp_snakeLoopThread,
			&this->m_ti,
			0,
			nullptr
		);
		if (this->m_ti.hThread == nullptr)
			return false;
		else
			return true;
	}
}
bool snake::logic::stopSnakeLoop() noexcept
{
	if (this->m_ti.hThread == nullptr)
		return false;
	
	this->m_ti.end = true;
	while (this->m_ti.hThread != nullptr)
		Sleep(1);
	
	return true;
}
void snake::logic::stepNow() noexcept
{
	this->m_ti.time = 0.f;
}
