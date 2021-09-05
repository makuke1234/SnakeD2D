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
