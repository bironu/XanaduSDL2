#include "sdl/SDLMixAudio.h"
#include "sdl/SDLMixMixer.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_properties.h>

namespace SDL_
{
namespace Mix_
{

namespace {

// MIX_LoadAudio()ではなくMIX_LoadAudioWithProperties()を使うのは、MIDI(BGM)を
// FluidSynthデコーダで再生する際にSoundFontのパスを指定するため。SDL3_mixerに
// 旧Mix_SetSoundFonts相当のグローバル設定APIが無いので、ロードごとにデコーダ
// 固有プロパティ(SDL_mixer.decoder.fluidsynth.soundfont_path)として渡す。
// WAV等(SE)の読み込みでは単に無視されるため、SE/BGM共通のこの経路で問題ない。
MIX_Audio *loadAudio(Mixer &owner, const char *fileName)
{
	SDL_IOStream *io = ::SDL_IOFromFile(fileName, "rb");
	if (!io) {
		return nullptr;
	}

	SDL_PropertiesID props = ::SDL_CreateProperties();
	::SDL_SetPointerProperty(props, "SDL_mixer.audio.load.iostream", io);
	::SDL_SetBooleanProperty(props, "SDL_mixer.audio.load.closeio", true);

	const std::string &soundFontPath = owner.getSoundFontPath();
	if (!soundFontPath.empty()) {
		::SDL_SetStringProperty(props, "SDL_mixer.decoder.fluidsynth.soundfont_path", soundFontPath.c_str());
	}

	MIX_Audio *audio = ::MIX_LoadAudioWithProperties(props);
	::SDL_DestroyProperties(props);
	return audio;
}

} // namespace

Audio::Audio(Mixer &owner, const char *fileName)
	: audio_(loadAudio(owner, fileName))
{
}

Audio::~Audio()
{
	::MIX_DestroyAudio(audio_);
}

} // namespace Mix_
} // namespace SDL_
