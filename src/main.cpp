#include "common.hpp"
#include "window.hpp"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, PSTR cmdArgs, int nCmdShow)
{
	snake::Application app(cmdArgs);
	if (app.Init(hInst, nCmdShow)) [[likely]]
		return app.MsgLoop();
	else [[unlikely]]
		return -1;
}
