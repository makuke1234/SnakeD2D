#pragma once

#include "common.hpp"

namespace snake
{
	class Application;

	class logic
	{
	public:
		enum class direction : std::int8_t
		{
			left,
			up,
			right,
			down
		};
		static constexpr auto direction_enum_size{ std::underlying_type_t<direction>(4) };

	private:
		friend class snake::Application;

		snake::Application & m_parentRef;
		direction m_snakeDirection{ direction::left };

		struct snakeInfo
		{
			logic & This;
			enum class modes : std::uint8_t
			{
				normal,
				game_over,
				win
			} mode{ modes::normal };
			HANDLE hThread{ nullptr };
			bool end{ false };
			struct scoringStruct
			{
				float time{ 0.f }, curTime{ .5f };
				std::uint32_t score{ 0 };
				static constexpr std::uint32_t winningScore{ 1000000 };
			} scoring{};
		} m_ti{ *this };
		static constexpr std::size_t p_snakeLoopThreadStackSize{ 10 * sizeof(std::size_t) };
		static DWORD WINAPI sp_snakeLoopThread(LPVOID lp) noexcept;

	public:
		logic() = delete;
		logic(Application & parentRef) noexcept;
		~logic() noexcept;

		void changeDirection(direction newdir) noexcept;
		void moveSnake() const noexcept;
		void moveAndGrowSnake() const;

		//
		//	Starts snake loop thread
		//	@returns 'false' only and only if thread creation fails, otherwise 'true'
		//
		bool startSnakeLoop() noexcept;
		//
		//	Stops snake loop thread
		//	@return 'true' if thread was running, 'false' if thread was already terminated
		//
		bool stopSnakeLoop() noexcept;
		void stepNow() noexcept;

		void resetScoring() noexcept;
	};

}
