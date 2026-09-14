#include "sdl/SDLMixMusic.h"
#include <SDL2/SDL_mixer.h>

namespace SDL_
{
namespace Mix_
{

Music::Music(const char *fileName)
	: music_(::Mix_LoadMUS(fileName))
{
}

Music::~Music()
{
	::Mix_FreeMusic(music_);
}

} // namespace Mix_
} // namespace SDL_
