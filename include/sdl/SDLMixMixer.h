#if !defined(SDLMIXMIXER_H_)
#define SDLMIXMIXER_H_

#include "misc/Uncopyable.h"
#include <memory>
#include <string>
#include <vector>

struct MIX_Mixer;

namespace SDL_
{
namespace Mix_
{

class Audio;
class Track;

// MIX_Mixer(再生デバイス+トラック群)のラッパー。
class Mixer final
{
public:
	UNCOPYABLE(Mixer);
	Mixer();
	~Mixer();

    MIX_Mixer *get() const { return mixer_; }

	int allocateChannels(int);
	int playSound(Audio &, int = -1, int = 0);
	bool playMusic(Audio &, int = -1);
	bool stopMusic();
	void pauseMusic();
	void resumeMusic();
    void rewindMusic();
    bool setSoundFonts(const char *);

	// AudioがMIDI読み込み時にMIX_LoadAudioWithProperties()へ
	// デコーダ固有プロパティとして渡すために参照する
	const std::string &getSoundFontPath() const { return soundFontPath_; }

private:
	MIX_Mixer * const mixer_;
	std::vector<std::unique_ptr<Track>> seTracks_;
	std::unique_ptr<Track> musicTrack_;
	std::string soundFontPath_;

};

} // namespace Mix_
} // namespace SDL_

#endif // SDLMIXMIXER_H_
