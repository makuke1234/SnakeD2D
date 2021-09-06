#include "snakelogic.hpp"
#include "window.hpp"
#include "resource.h"

#include <ctime>

DWORD WINAPI snake::Logic::sp_snakeLoopThread(LPVOID lp) noexcept
{
	auto inf = static_cast<snakeInfo *>(lp);

	while (!inf->endSignal)
	{
		while (inf->scoring.time > 0.f)
		{
			auto st = std::clock();
			Sleep(20);
			inf->scoring.time -= float(std::clock() - st) / 1000.f;
		}
		inf->scoring.time = inf->scoring.curTime;

		switch (inf->scoring.mode)
		{
		case snakeInfo::modes::normal:
		{
			// Check for "bad" collisions
			bool collides{ false };
			auto const & headRect = inf->This.m_appref.m_tiles.snakeHeadTile.getBounds();
			for (auto const & i : inf->This.m_appref.m_tiles.obstacleTiles)
			{
				if (i.collides(headRect))
				{
					collides = true;
					break;
				}
			}
			if (!collides)
			{
				for (auto const & i : inf->This.m_appref.m_tiles.snakeBodyTiles)
				{
					if (i.collides(headRect))
					{
						collides = true;
						break;
					}
				}
			}

			if (collides)
			{
				inf->scoring.mode = snakeInfo::modes::game_over;
				inf->scoring.time = 0.f;
				break;
			}

			// Check for "good" collisions with food
			auto & foodTile = inf->This.m_appref.m_tiles.snakeFoodTile;
			if (foodTile.collides(headRect))
			{
				// Eat food and generate new
				inf->This.m_appref.genFood(foodTile);

				// Play eating sound async
				inf->This.m_appref.playSndAsync(IDW_SOUND_EAT);

				// Grow snake
				inf->This.moveAndGrowSnake();

				// Make time unit smaller
				inf->This.m_sInfo.scoring.score += 1;

				auto forward = [](float x)
				{
					return 1.f / x + .1f;
				};
				auto backward = [](float x)
				{
					return 1.f / (x - .1f);
				};

				inf->This.m_sInfo.scoring.curTime = forward(backward(inf->This.m_sInfo.scoring.curTime) + .1f);
			}
			else
			{
				// Move snake normally
				inf->This.moveSnake();
			}

			// After checking "good" collisions, check score
			if (inf->scoring.score >= inf->scoring.winningScore)
			{
				inf->scoring.mode = snakeInfo::modes::win;
				inf->scoring.time = 0.f;
			}
			break;
		}
		case snakeInfo::modes::game_over:
			// Play game over sound
			inf->This.m_appref.playSnd(IDW_SOUND_GAME_OVER);

			// Draw game over text


			// Update screen
			::InvalidateRect(inf->This.m_appref.m_hwnd, nullptr, FALSE);

			// Wait for mode change
			while (inf->scoring.mode == snakeInfo::modes::game_over)
			{
				Sleep(50);
				if (inf->endSignal)
					goto sp_snakeLoopThreadFinish;
			}

			break;
		case snakeInfo::modes::win:
			// Play winning sound
			inf->This.m_appref.playSnd(IDW_SOUND_WIN);

			// Draw winning text


			// Update screen
			::InvalidateRect(inf->This.m_appref.m_hwnd, nullptr, FALSE);

			// Wait for mode change
			while (inf->scoring.mode == snakeInfo::modes::win)
			{
				Sleep(50);
				if (inf->endSignal)
					goto sp_snakeLoopThreadFinish;
			}

			break;
		}
		// Update screen
		::InvalidateRect(inf->This.m_appref.m_hwnd, nullptr, FALSE);
	}

sp_snakeLoopThreadFinish: ;

	inf->hThread = nullptr;
	return 0;
}

snake::Logic::Logic(Application & parentRef) noexcept
	: m_appref(parentRef)
{}
snake::Logic::~Logic() noexcept
{
	this->stopSnakeLoop();
}

void snake::Logic::changeDirection(direction newdir) noexcept
{
	this->m_sInfo.scoring.snakeDir = newdir;
}
void snake::Logic::moveSnake() const noexcept
{

	auto prevtile = this->m_appref.m_tiles.snakeHeadTile;
	auto [uposx, uposy] = prevtile.getCoords(this->m_appref.m_tileSzF);
	auto posx{ long(uposx) }, posy{ long(uposy) };
	
	switch (this->m_sInfo.scoring.snakeDir)
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

	constexpr auto ipWidth{ std::intptr_t(this->m_appref.fieldWidth) },
		ipHeight{ std::intptr_t(this->m_appref.fieldHeight) };
	// Make sure that position stays in bounds
	if (posx < 0)
		posx += ipWidth;
	else if (posx >= ipWidth)
		posx -= ipWidth;

	if (posy < 0)
		posy += ipHeight;
	else if (posy >= ipHeight)
		posy -= ipHeight;

	this->m_appref.moveTile(this->m_appref.m_tiles.snakeHeadTile, posx, posy);
	this->m_appref.m_tiles.snakeBodyTiles.emplace_front(std::move(this->m_appref.m_tiles.snakeBodyTiles.back()));
	this->m_appref.m_tiles.snakeBodyTiles.pop_back();
	this->m_appref.m_tiles.snakeBodyTiles.front() = prevtile;
}
void snake::Logic::moveAndGrowSnake() const
{
	auto tail{ this->m_appref.m_tiles.snakeBodyTiles.back() };
	tail.createAssets(this->m_appref.m_bmpBrushes.snakeBodyTile);
	this->moveSnake();
	this->m_appref.m_tiles.snakeBodyTiles.emplace_back(std::move(tail));
}

bool snake::Logic::startSnakeLoop() noexcept
{
	if (this->m_sInfo.hThread != nullptr)
		return true;
	else
	{
		this->m_sInfo.endSignal = false;
		this->m_sInfo.hThread = ::CreateThread(
			nullptr,
			this->p_snakeLoopThreadStackSize,
			&this->sp_snakeLoopThread,
			&this->m_sInfo,
			0,
			nullptr
		);
		if (this->m_sInfo.hThread == nullptr)
			return false;
		else
			return true;
	}
}
bool snake::Logic::stopSnakeLoop() noexcept
{
	if (this->m_sInfo.hThread == nullptr)
		return false;
	
	this->m_sInfo.endSignal = true;
	while (this->m_sInfo.hThread != nullptr)
		Sleep(1);
	
	return true;
}
void snake::Logic::stepNow() noexcept
{
	this->m_sInfo.scoring.time = 0.f;
}

void snake::Logic::resetScoring() noexcept
{
	this->m_sInfo.scoring = snakeInfo::scoringStruct();
}
