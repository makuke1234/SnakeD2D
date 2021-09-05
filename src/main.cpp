#include "common.hpp"
#include "window.hpp"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, PSTR cmdArgs, int nCmdShow)
{
	snake::Application app(cmdArgs);
	if (app.initApp(hInst, nCmdShow)) [[likely]]
		return app.msgLoop();
	else [[unlikely]]
		return -1;
}
