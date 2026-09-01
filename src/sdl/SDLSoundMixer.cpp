#include "sdl/SDLSoundMixer.h"
#include "sdl/SDLSoundChunk.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_audio.h>

namespace SDL_ {

SoundMixer::SoundMixer()
	: audioDevice_(::Mix_OpenAudioDevice(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 256, nullptr, true))
{
}

SoundMixer::~SoundMixer()
{
	::Mix_CloseAudio();
}

int SoundMixer::allocateChannels(int size)
{
	return ::Mix_AllocateChannels(size);
}

int SoundMixer::playChannel(int channel, SoundChunk &chunk, int loops)
{
	return ::Mix_PlayChannel(channel, chunk.get(), loops);
}

} // namespace SDL_
