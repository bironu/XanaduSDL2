#include "sdl/SDLMixMixer.h"
#include "sdl/SDLMixAudio.h"
#include "sdl/SDLMixTrack.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL_audio.h>

namespace SDL_
{
namespace Mix_
{

Mixer::Mixer()
	: mixer_(::MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr))
	, seTracks_()
	, musicTrack_(std::make_unique<Track>(mixer_))
{
}

Mixer::~Mixer()
{
	// Track(MIX_Track)はMIX_DestroyMixer()より先に破棄しておく必要があるため、
	// メンバの暗黙のデストラクト順(宣言の逆順)任せにせず明示的にここで破棄する
	musicTrack_.reset();
	seTracks_.clear();
	::MIX_DestroyMixer(mixer_);
}

int Mixer::allocateChannels(int size)
{
	seTracks_.clear();
	seTracks_.reserve(size);
	for (int i = 0; i < size; ++i) {
		seTracks_.push_back(std::make_unique<Track>(mixer_));
	}
	return static_cast<int>(seTracks_.size());
}

int Mixer::playSound(Audio &sound, int channel, int loops)
{
	if (seTracks_.empty()) {
		return -1;
	}
	int index = 0;
	if (channel >= 0 && static_cast<std::size_t>(channel) < seTracks_.size()) {
		index = channel;
	}
	else {
		// -1(空きチャンネル自動選択)相当: 再生中でないトラックを探す。
		// 見つからなければ先頭のトラックを横取りする。
		for (std::size_t i = 0; i < seTracks_.size(); ++i) {
			if (!::MIX_TrackPlaying(seTracks_[i]->get())) {
				index = static_cast<int>(i);
				break;
			}
		}
	}
	MIX_Track *track = seTracks_[index]->get();
	::MIX_SetTrackAudio(track, sound.get());
	::MIX_SetTrackLoops(track, loops);
	::MIX_PlayTrack(track, 0);
	return index;
}

bool Mixer::playMusic(Audio &sound, int loops)
{
	::MIX_SetTrackAudio(musicTrack_->get(), sound.get());
	::MIX_SetTrackLoops(musicTrack_->get(), loops);
	return ::MIX_PlayTrack(musicTrack_->get(), 0);
}

bool Mixer::stopMusic()
{
	return ::MIX_StopTrack(musicTrack_->get(), 0);
}

void Mixer::pauseMusic()
{
	::MIX_PauseTrack(musicTrack_->get());
}

void Mixer::resumeMusic()
{
	::MIX_ResumeTrack(musicTrack_->get());
}

void Mixer::rewindMusic()
{
	::MIX_SetTrackPlaybackPosition(musicTrack_->get(), 0);
}

bool Mixer::setSoundFonts(const char *)
{
	// TODO: SDL3_mixer(MIX_*)には旧Mix_SetSoundFontsに相当する
	// グローバルSoundFont設定APIが見当たらないため、移行時点では未対応。
	return false;
}

} // namespace Mix_
} // namespace SDL_
