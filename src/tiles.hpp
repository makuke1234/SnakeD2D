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
		~tile() noexcept;

		bool CreateAssets(ID2D1HwndRenderTarget * pRT, ID2D1Bitmap * pBitmap) noexcept;
		void DestroyAssets() noexcept;

		void OnRender(ID2D1HwndRenderTarget * pRT) noexcept;

		template<class VecT>
			requires std::is_same_v<std::decay_t<decltype(VecT()[0])>, tile>
		static void OnRender(VecT & tiles, ID2D1HwndRenderTarget * pRT) noexcept
		{
			for (auto & i : tiles)
				i.OnRender(pRT);
		}
	};
}
