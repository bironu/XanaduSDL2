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
enum class StringId;
enum class SoundFontId;

namespace SDL_
{
class Image;
class Joystick;
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

	std::shared_ptr<SDL_::Image> getImage(const ImageId &) const;
	const char *getString(const StringId &) const;
	std::shared_ptr<SDL_::Joystick> getJoystick(uint32_t) const;

	void loadString(const std::string &lang);
	void loadImage(const std::string &lang);
	void clearImage();
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
	std::unordered_map<uint32_t, std::shared_ptr<SDL_::Joystick>> mapJoystick_;
};

#endif // RESOURCES_H_
