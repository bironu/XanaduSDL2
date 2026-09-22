#ifndef RESOURCES_H_
#define RESOURCES_H_

#include "misc/Uncopyable.h"
#include <cstdint>
#include <memory>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>

enum class ImageId;
// enum class StringId;
enum class SoundFontId;
enum class SoundId;
enum class MusicId;

namespace SDL_
{
    class Image;
    class Joystick;
    namespace Mix_
    {
        class Audio;
        class Mixer;
    }
}

// namespace sol
// {
// class state;
// }

struct SDL_JoyDeviceEvent;
class Resources final
{
public:
	UNCOPYABLE(Resources);
	Resources();
	~Resources();

	// 旧C時代の自由関数(dungeon.cpp/cave.cpp/user.cpp/boss.cpp/shop.cpp/
	// inventory.cpp 等)からリソースへアクセスするための参照。Application::instance()
	// と同じ橋渡し用の簡易実装。インスタンスは常に高々1つしか生成されない前提。
	static Resources &instance() { return *instance_; }

	void setWindowWidth(int width) { windowWidth_ = width; }
	void setWindowHeight(int height) { windowHeight_ = height; }
	int getWindowWidth() const { return windowWidth_; }
	int getWindowHeight() const { return windowHeight_; }
	int getScreenWidth() const { return screenWidth_; }
	int getScreenHeight() const { return screenHeight_; }

	const char *getFontFileName() const;
	const char *getSoundFontFileName(const SoundFontId &) const;

	void addJoyDevice(const SDL_JoyDeviceEvent &);
	void removeJoyDevice(const SDL_JoyDeviceEvent &);
	std::shared_ptr<SDL_::Joystick> getJoystick(uint32_t) const;

	bool loadImage(ImageId id);
    void unloadImage(ImageId id);
	std::shared_ptr<SDL_::Image> getImage(const ImageId &) const;

    bool loadSound(SDL_::Mix_::Mixer &mixer, SoundId id);
    void unloadSound(SoundId id);
    std::shared_ptr<SDL_::Mix_::Audio> getSound(const SoundId &) const;

    bool loadMusic(SDL_::Mix_::Mixer &mixer, MusicId id);
    void unloadMusic(MusicId id);
    std::shared_ptr<SDL_::Mix_::Audio> getMusic(const MusicId &) const;

    // 現在再生中(またはロード済み)のBGMをidで指定し直す。同じidが既に再生中なら
    // 何もしない(曲の鳴らし直しを防ぐ)。idを切り替えた場合、旧トラックは
    // Mixerへの再配線(playMusic)が完了した後でunloadMusicする。
    bool playBgm(SDL_::Mix_::Mixer &mixer, MusicId id);

    void reload();

private:
	int windowWidth_;
	int windowHeight_;
	int screenWidth_;
	int screenHeight_;
	std::string lang_;
	// std::unique_ptr<sol::state> luaString_;
	// std::unique_ptr<sol::state> luaImage_;
	mutable std::unordered_map<ImageId, std::shared_ptr<SDL_::Image>> mapImage_;
    mutable std::unordered_map<SoundId, std::shared_ptr<SDL_::Mix_::Audio>> mapSound_;
    mutable std::unordered_map<MusicId, std::shared_ptr<SDL_::Mix_::Audio>> mapMusic_;
	std::unordered_map<uint32_t, std::shared_ptr<SDL_::Joystick>> mapJoystick_;
    MusicId currentMusicId_;

	static Resources *instance_; // TODO: singletonはやめる
};

#endif // RESOURCES_H_
