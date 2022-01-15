#include "common.hpp"

void snake::playSndRsc(std::uint16_t resource, HINSTANCE hInst) noexcept
{
	::PlaySoundW(MAKEINTRESOURCEW(resource), hInst, SND_RESOURCE);
}
void snake::playSndRscAsync(std::uint16_t resource, HINSTANCE hInst) noexcept
{
	::PlaySoundW(MAKEINTRESOURCEW(resource), hInst, SND_RESOURCE | SND_ASYNC);
}
void snake::stopSndRscAsync() noexcept
{
	::PlaySoundW(nullptr, nullptr, 0);
}

bool snake::getScreenSize(HWND window, SIZE & screen) noexcept
{
	HMONITOR monitor = ::MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
	MONITORINFO miw{};
	miw.cbSize = sizeof miw;
	if (::GetMonitorInfoW(monitor, &miw) == FALSE)
	{
		return false;
	}

	screen.cx = miw.rcMonitor.right  - miw.rcMonitor.left;
	screen.cy = miw.rcMonitor.bottom - miw.rcMonitor.top;

	return true;
}
