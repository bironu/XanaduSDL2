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
};

#endif // RESOURCES_H_
