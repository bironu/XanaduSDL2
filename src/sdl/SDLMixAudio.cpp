#include "sdl/SDLMixAudio.h"
#include "sdl/SDLMixChunk.h"
#include "sdl/SDLMixMusic.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_audio.h>

namespace SDL_
{
namespace Mix_
{

Audio::Audio()
	: audioDevice_(::Mix_OpenAudioDevice(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 256, nullptr, true))
{
}

Audio::~Audio()
{
	::Mix_CloseAudio();
}

int Audio::allocateChannels(int size)
{
	return ::Mix_AllocateChannels(size);
}

int Audio::playSound(Chunk &chunk, int channel, int loops)
{
	return ::Mix_PlayChannel(channel, chunk.get(), loops);
}

bool Audio::playMusic(Music &music, int loops)
{
	return ::Mix_PlayMusic(music.get(), loops) == 0;
}

bool Audio::stopMusic()
{
	return ::Mix_HaltMusic() == 0;
}

void Audio::pauseMusic()
{
	::Mix_PauseMusic();
}

void Audio::resumeMusic()
{
	::Mix_ResumeMusic();
}

void Audio::rewindMusic()
{
    ::Mix_RewindMusic();
}

bool Audio::setSoundFonts(const char *paths)
{
    if (paths) {
        return ::Mix_SetSoundFonts(paths) != 0;
    }
    else {
        return false;
    }
}

} // namespace Mix_
} // namespace SDL_
