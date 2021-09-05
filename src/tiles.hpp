#pragma once

#include "common.hpp"

namespace snake
{
	class Tile
	{
	private:
		friend class Application;

		dx::RectF m_tilesRect{};

		dx::BmpBrush * m_pBmBrush{ nullptr };

	public:
		Tile() noexcept = default;
		Tile(dx::SzF const & tileSz, dx::SzF const & startpos, dx::SzU const & numTiles) noexcept;
		constexpr Tile(Tile const & other) noexcept
			: m_tilesRect(other.m_tilesRect)
		{}
		constexpr Tile(Tile && other) noexcept
			: m_tilesRect(other.m_tilesRect), m_pBmBrush(other.m_pBmBrush)
		{
			other.m_tilesRect = {};
			other.m_pBmBrush = nullptr;
		}
		Tile & operator=(Tile const & other) noexcept
		{
			this->m_tilesRect = other.m_tilesRect;
			return *this;
		}
		Tile & operator=(Tile && other) noexcept
		{
			this->~Tile();
			this->m_tilesRect = other.m_tilesRect;
			this->m_pBmBrush = other.m_pBmBrush;
			other.m_tilesRect = {};
			other.m_pBmBrush = nullptr;
			return *this;
		}
		~Tile() noexcept;

		[[nodiscard]] static dx::BmpBrush * createBmpBrush(dx::HwndRT * pRT, dx::Bmp * pBm) noexcept;
		void createAssets(dx::BmpBrush * pBmBrush) noexcept;
		void destroyAssets() noexcept;

		void onRender(dx::HwndRT * pRT) const noexcept;

		template<class VecT>
			requires std::is_same_v<std::decay_t<decltype(VecT()[0])>, Tile>
		static void onRender(VecT const & tiles, dx::HwndRT * pRT) noexcept
		{
			for (auto const & i : tiles)
				i.onRender(pRT);
		}

		[[nodiscard]] dx::SzU getCoords(dx::SzF const & tileSz) const noexcept;
		[[nodiscard]] constexpr dx::RectF const & getBounds() const noexcept
		{
			return this->m_tilesRect;
		}
		void move(dx::SzF const & newpos) noexcept;

		[[nodiscard]] static constexpr bool s_collides(dx::RectF const & r1, dx::RectF const & r2) noexcept
		{
			constexpr float d{ .1f };
			return ((r1.left + d) < r2.right) && (r1.right > (r2.left + d)) && ((r1.top + d) < r2.bottom) && (r1.bottom > (r2.top + d));
		}
		[[nodiscard]] constexpr bool collides(dx::RectF const & otherRect) const noexcept
		{
			return this->s_collides(this->m_tilesRect, otherRect);
		}
	};
}
