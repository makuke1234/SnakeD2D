#include "common.hpp"
#include "window.hpp"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR lpCmdArgs, int nCmdShow)
{
	snake::Application app(lpCmdArgs);
	if (app.initApp(hInst, nCmdShow)) [[likely]]
		return app.msgLoop();
	else [[unlikely]]
		return -1;
}
