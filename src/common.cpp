#include "common.hpp"

void snake::playSndRsc(LPCWSTR resourceName, HINSTANCE hInst) noexcept
{
	::PlaySoundW(resourceName, hInst, SND_RESOURCE);
}
void snake::playSndRscAsync(LPCWSTR resourceName, HINSTANCE hInst) noexcept
{
	::PlaySoundW(resourceName, hInst, SND_RESOURCE | SND_ASYNC);
}
void snake::stopSndRscAsync() noexcept
{
	::PlaySoundW(nullptr, nullptr, 0);
}
