#if !defined(SDLMIXMIXER_H_)
#define SDLMIXMIXER_H_

#include "misc/Uncopyable.h"
#include <memory>
#include <vector>

struct MIX_Mixer;

namespace SDL_
{
namespace Mix_
{

class Audio;
class Track;

// MIX_Mixer(再生デバイス+トラック群)のラッパー。
class Mixer
{
public:
	UNCOPYABLE(Mixer);
	Mixer();
	~Mixer();

	// SDL3_mixer(MIX_Mixer/MIX_Track/MIX_Audio)へ移行したことに伴い新設した
	// アクセサ。AudioがMIX_LoadAudio()するために必要となる。
	MIX_Mixer *mixer() const { return mixer_; }

	int allocateChannels(int);
	int playSound(Audio &, int = -1, int = 0);
	bool playMusic(Audio &, int = -1);
	bool stopMusic();
	void pauseMusic();
	void resumeMusic();
    void rewindMusic();
    bool setSoundFonts(const char *);

private:
	MIX_Mixer * const mixer_;
	std::vector<std::unique_ptr<Track>> seTracks_;
	std::unique_ptr<Track> musicTrack_;

};

} // namespace Mix_
} // namespace SDL_

#endif // SDLMIXMIXER_H_
