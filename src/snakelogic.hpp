#pragma once

#include "common.hpp"

namespace snake
{
	class Application;

	class logic
	{
	public:
		enum class direction : std::uint8_t
		{
			left,
			up,
			right,
			down
		};

	private:
		snake::Application & m_parentRef;
		direction snakeDirection{ direction::left };

		struct p_snakeLoopThreadInfo
		{
			logic & This;
			HANDLE hThread{ nullptr };
			bool end{ false };
		} m_ti{ *this };
		static constexpr std::size_t p_snakeLoopThreadStackSize{ 10 * sizeof(std::size_t) };
		static DWORD WINAPI sp_snakeLoopThread(LPVOID lp) noexcept;

	public:
		logic() = delete;
		logic(Application & parentRef) noexcept;

		void moveSnake(direction dir) noexcept;
		void moveAndGrowSnake(direction dir);

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
	};

}
