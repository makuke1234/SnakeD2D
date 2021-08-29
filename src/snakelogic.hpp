#pragma once

#include "common.hpp"

namespace snake
{
	class Application;

	class logic
	{
	private:
		snake::Application & m_parentRef;

	public:
		logic() = delete;
		logic(Application & parentRef) noexcept;

	};

}