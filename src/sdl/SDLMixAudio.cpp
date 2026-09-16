#include "sdl/SDLMixAudio.h"
#include "sdl/SDLMixMixer.h"
#include <SDL3_mixer/SDL_mixer.h>

namespace SDL_
{
namespace Mix_
{

Audio::Audio(Mixer &owner, const char *fileName)
	: audio_(::MIX_LoadAudio(owner.mixer(), fileName, false))
{
}

Audio::~Audio()
{
	::MIX_DestroyAudio(audio_);
}

} // namespace Mix_
} // namespace SDL_
