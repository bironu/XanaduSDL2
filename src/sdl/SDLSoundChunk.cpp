#include "sdl/SDLSoundChunk.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>

namespace SDL_ {

SoundChunk::SoundChunk(const char *fileName)
	: chunk_(Mix_LoadWAV(fileName))
{
}

SoundChunk::~SoundChunk()
{
	::Mix_FreeChunk(chunk_);
}

} // namespace SDL_
