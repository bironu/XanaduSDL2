#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/StringId.h"
#include "resources/SoundFontId.h"
#include "sdl/SDLImage.h"
#include "sdl/SDLJoystick.h"
#include <SDL2/SDL_events.h>
#include <cstring>

#define IMAGE_ROOT "../bmp/"
#define AUDIO_ROOT "../audio/"

Resources::Resources()
	: windowWidth_()
	, windowHeight_()
	, screenWidth_()
	, screenHeight_()
	, lang_()
	// , luaString_()
	// , luaImage_()
	, mapImage_()
	, mapJoystick_()
{
}

Resources::~Resources()
{
}

std::shared_ptr<SDL_::Image> Resources::getImage(const ImageId &id) const
{
	auto i = mapImage_.find(id);
	if (i != mapImage_.end()) {
		return i->second;
	}
	else {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Image ID not found. %d\n", id);
		return std::shared_ptr<SDL_::Image>();
	}
}

const char *Resources::getString(const StringId &id) const
{
	// const char *result = (*luaString_)["strings"][id.c_str()].get_or<const char *>(nullptr);
	// if (!result) {
	// 	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "String ID not found. %s\n", id.c_str());
	// }
	return "";
}

const char *Resources::getFontFileName() const
{
//	return (*luaString_)["font_name"].get<const char *>();
	return "../font/ipag.ttf";
}

const char *Resources::getSoundFontFileName(const SoundFontId &id) const
{
	switch (id) {
	case SoundFontId::hi_def:
		return AUDIO_ROOT "HiDef.sf2";
	default:
		return "";
	}
}

void Resources::addJoyDevice(const SDL_JoyDeviceEvent &jdevice)
{
	mapJoystick_.insert(std::make_pair(jdevice.which, std::make_shared<SDL_::Joystick>(jdevice.which)));
}

void Resources::removeJoyDevice(const SDL_JoyDeviceEvent &jdevice)
{
	for (auto &pair : mapJoystick_) {
		if(pair.second->getInstanceID() == jdevice.which) {
			mapJoystick_.erase(pair.first);
			break;
		}
	}
}

std::shared_ptr<SDL_::Joystick> Resources::getJoystick(int index) const
{
	auto i = mapJoystick_.find(index);
	if (i != mapJoystick_.end()) {
		return i->second;
	}
	else {
		return nullptr;
	}
}

void Resources::loadString(const std::string &lang)
{
	// const std::string langPath = "res/lua/lang/" + lang + ".lua";
	// luaString_ = std::make_unique<sol::state>();
	// luaString_->open_libraries(sol::lib::base, sol::lib::package);
	// luaString_->script_file(langPath);
}

void Resources::loadImage(const std::string &lang)
{
	mapImage_.clear();
	mapImage_.emplace(ImageId::picture_agl , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/agl.bmp"));
	mapImage_.emplace(ImageId::picture_armory , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/armory.bmp"));
	mapImage_.emplace(ImageId::picture_castle , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/castle.bmp"));
	mapImage_.emplace(ImageId::picture_cave , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/cave.bmp"));
	mapImage_.emplace(ImageId::picture_chr , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/chr.bmp"));
	mapImage_.emplace(ImageId::picture_dex , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/dex.bmp"));
	mapImage_.emplace(ImageId::picture_foods , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/foods.bmp"));
	mapImage_.emplace(ImageId::picture_guilds , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/guilds.bmp"));
	mapImage_.emplace(ImageId::picture_healers , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/healers.bmp"));
	mapImage_.emplace(ImageId::picture_inn , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/inn.bmp"));
	mapImage_.emplace(ImageId::picture_int , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/int.bmp"));
	mapImage_.emplace(ImageId::picture_item , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/item.bmp"));
	mapImage_.emplace(ImageId::picture_kanji , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/kanji.bmp"));
	mapImage_.emplace(ImageId::picture_logo , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/logo.bmp"));
	mapImage_.emplace(ImageId::picture_mgr , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/mgr.bmp"));
	mapImage_.emplace(ImageId::picture_scroll , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/scroll.bmp"));
	mapImage_.emplace(ImageId::picture_shield , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/shield.bmp"));
	mapImage_.emplace(ImageId::picture_shop , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/shop.bmp"));
	mapImage_.emplace(ImageId::picture_slayer , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/slayer.bmp"));
	mapImage_.emplace(ImageId::picture_str , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/str.bmp"));
	mapImage_.emplace(ImageId::picture_temple , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/temple.bmp"));
	mapImage_.emplace(ImageId::picture_weapon , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/weapon.bmp"));
	mapImage_.emplace(ImageId::picture_wis , std::make_shared<SDL_::Image>(IMAGE_ROOT "picture/wis.bmp"));
	mapImage_.emplace(ImageId::user_boss_st , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/boss_st.bmp"));
	mapImage_.emplace(ImageId::user_breath , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/breath.bmp"));
	mapImage_.emplace(ImageId::user_damage , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/damage.bmp"));
	mapImage_.emplace(ImageId::user_effect , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/effect.bmp"));
	mapImage_.emplace(ImageId::user_font , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/font.bmp"));
	mapImage_.emplace(ImageId::user_frame , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/frame.bmp"));
	mapImage_.emplace(ImageId::user_goods , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/goods.bmp"));
	mapImage_.emplace(ImageId::user_magic , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/magic.bmp"));
	mapImage_.emplace(ImageId::user_pattern , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/pattern.bmp"));
	mapImage_.emplace(ImageId::user_user0 , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/user0.bmp"));
	mapImage_.emplace(ImageId::user_user1 , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/user1.bmp"));
	mapImage_.emplace(ImageId::user_user2 , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/user2.bmp"));
	mapImage_.emplace(ImageId::user_user3 , std::make_shared<SDL_::Image>(IMAGE_ROOT "user/user3.bmp"));
	mapImage_.emplace(ImageId::xa1_boss_0 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/boss_0.bmp"));
	mapImage_.emplace(ImageId::xa1_boss_1 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/boss_1.bmp"));
	mapImage_.emplace(ImageId::xa1_boss_2 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/boss_2.bmp"));
	mapImage_.emplace(ImageId::xa1_boss_3 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/boss_3.bmp"));
	mapImage_.emplace(ImageId::xa1_boss_4 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/boss_4.bmp"));
	mapImage_.emplace(ImageId::xa1_field , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/field.bmp"));
	mapImage_.emplace(ImageId::xa1_frame , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/frame.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_0 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_0.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_1 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_1.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_2 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_2.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_3 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_3.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_4 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_4.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_5 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_5.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_6 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_6.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_7 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_7.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_8 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_8.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_9 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_9.bmp"));
	mapImage_.emplace(ImageId::xa1_monst_a , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/monst_a.bmp"));
	mapImage_.emplace(ImageId::xa1_shrine , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/shrine.bmp"));
	mapImage_.emplace(ImageId::xa1_train , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/train.bmp"));
	mapImage_.emplace(ImageId::xa1_opening_background , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/opening/background.bmp"));
	mapImage_.emplace(ImageId::xa1_opening_battler , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/opening/battler.bmp"));
	mapImage_.emplace(ImageId::xa1_opening_robber , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/opening/robber.bmp"));
	mapImage_.emplace(ImageId::xa1_opening_swordman , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/opening/swordman.bmp"));
	mapImage_.emplace(ImageId::xa1_opening_witch , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/opening/witch.bmp"));
	mapImage_.emplace(ImageId::xa1_opening_wizard , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa1/opening/wizard.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_0 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_0.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_1 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_1.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_2 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_2.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_3 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_3.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_4 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_4.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_5 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_5.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_6 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_6.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_7 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_7.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_8 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_8.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_9 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_9.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_a , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_a.bmp"));
	mapImage_.emplace(ImageId::xa2_boss_b , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/boss_b.bmp"));
	mapImage_.emplace(ImageId::xa2_field , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/field.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_0 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_0.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_1 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_1.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_2 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_2.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_3 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_3.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_4 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_4.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_5 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_5.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_6 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_6.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_7 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_7.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_8 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_8.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_9 , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_9.bmp"));
	mapImage_.emplace(ImageId::xa2_monst_a , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/monst_a.bmp"));
	mapImage_.emplace(ImageId::xa2_outoflevel , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/outoflevel.bmp"));
	mapImage_.emplace(ImageId::xa2_shrine , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/shrine.bmp"));
	mapImage_.emplace(ImageId::xa2_ending_background , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/ending/background.bmp"));
	mapImage_.emplace(ImageId::xa2_opening_hero , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/opening/hero.bmp"));
	mapImage_.emplace(ImageId::xa2_opening_subtitle , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/opening/subtitle.bmp"));
	mapImage_.emplace(ImageId::xa2_opening_title , std::make_shared<SDL_::Image>(IMAGE_ROOT "xa2/opening/title.bmp"));

}

void Resources::clearImage()
{
	mapImage_.clear();
}

void Resources::reload()
{
	// sol::state lua;
	// lua.open_libraries(sol::lib::base, sol::lib::package);
	// lua.script_file("res/lua/system.lua");

	// windowWidth_ = lua["window"]["width"].get<int>();
	// windowHeight_ = lua["window"]["height"].get<int>();
	// screenWidth_ = lua["screen"]["width"].get<int>();
	// screenHeight_ = lua["screen"]["height"].get<int>();
	// lang_ = lua["system"]["lang"].get<const char *>();
	windowWidth_ = 640;
	windowHeight_ = 480;
	screenWidth_ = 640;
	screenHeight_ = 480;
	lang_ = "japanese";

	loadString(lang_);
	loadImage(lang_);
}
