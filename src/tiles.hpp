#pragma once

#include "common.hpp"

namespace snake
{
	class tile
	{
	private:
		friend class Application;

		D2D1_RECT_F m_tilesRect{};

		ID2D1BitmapBrush * m_pBmBrush{ nullptr };

	public:
		tile() noexcept = default;
		tile(D2D1_SIZE_F const & tileSz, D2D1_SIZE_F const & startpos, D2D1_SIZE_U const & numTiles) noexcept;
		constexpr tile(tile const & other) noexcept
			: m_tilesRect(other.m_tilesRect)
		{}
		constexpr tile(tile && other) noexcept
			: m_tilesRect(other.m_tilesRect), m_pBmBrush(other.m_pBmBrush)
		{
			other.m_tilesRect = {};
			other.m_pBmBrush = nullptr;
		}
		tile & operator=(tile const & other) noexcept
		{
			this->m_tilesRect = other.m_tilesRect;
			return *this;
		}
		tile & operator=(tile && other) noexcept
		{
			this->~tile();
			this->m_tilesRect = other.m_tilesRect;
			this->m_pBmBrush = other.m_pBmBrush;
			other.m_tilesRect = {};
			other.m_pBmBrush = nullptr;
			return *this;
		}
		~tile() noexcept;

		[[nodiscard]] static ID2D1BitmapBrush * CreateBmBrush(ID2D1HwndRenderTarget * pRT, ID2D1Bitmap * pBm) noexcept;
		void createAssets(ID2D1BitmapBrush * pBmBrush) noexcept;
		void destroyAssets() noexcept;

		void onRender(ID2D1HwndRenderTarget * pRT) const noexcept;

		template<class VecT>
			requires std::is_same_v<std::decay_t<decltype(VecT()[0])>, tile>
		static void onRender(VecT const & tiles, ID2D1HwndRenderTarget * pRT) noexcept
		{
			for (auto const & i : tiles)
				i.onRender(pRT);
		}

		[[nodiscard]] D2D1_SIZE_U getCoords(D2D1_SIZE_F const & tileSz) const noexcept;
		[[nodiscard]] constexpr D2D1_RECT_F const & getBounds() const noexcept
		{
			return this->m_tilesRect;
		}
		void move(D2D1_SIZE_F const & newpos) noexcept;

		[[nodiscard]] static constexpr bool s_collides(D2D1_RECT_F const & r1, D2D1_RECT_F const & r2) noexcept
		{
			constexpr float d{ .1f };
			return ((r1.left + d) < r2.right) && (r1.right > (r2.left + d)) && ((r1.top + d) < r2.bottom) && (r1.bottom > (r2.top + d));
		}
		[[nodiscard]] constexpr bool collides(D2D1_RECT_F const & otherRect) const noexcept
		{
			return this->s_collides(this->m_tilesRect, otherRect);
		}
	};
}
