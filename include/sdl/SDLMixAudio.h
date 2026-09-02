#if !defined(SDLMIXAUDIO_H_)
#define SDLMIXAUDIO_H_

#include "misc/Uncopyable.h"

namespace SDL_
{
namespace Mix_
{
class Chunk;
class Music;
class Audio
{
public:
	UNCOPYABLE(Audio);
	Audio();
	~Audio();

	int allocateChannels(int);
	int playSound(Chunk &, int = -1, int = 0);
	bool playMusic(Music &, int = -1);
	bool stopMusic();
	void pauseMusic();
	void resumeMusic();
    void rewindMusic();
    bool setSoundFonts(const char *);

private:
	const int audioDevice_;

};

} // namespace Mix_
} // namespace SDL_

#endif // SDLMIXAUDIO_H_
