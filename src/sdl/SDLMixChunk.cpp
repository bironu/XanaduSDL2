#include "sdl/SDLMixChunk.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>

namespace SDL_
{
namespace Mix_
{

Chunk::Chunk(const char *fileName)
	: chunk_(::Mix_LoadWAV(fileName))
{
}

Chunk::~Chunk()
{
	::Mix_FreeChunk(chunk_);
}

} // namespace Mix_
} // namespace SDL_
