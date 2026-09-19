#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/SoundId.h"
#include "resources/MusicId.h"
#include "resources/SoundFontId.h"
#include "sdl/SDLImage.h"
#include "sdl/SDLJoystick.h"
#include "sdl/SDLMixAudio.h"
#include "sdl/SDLMixMixer.h"
#include <SDL3/SDL_events.h>
#include <cstring>

#define IMAGE_ROOT "../bmp/"
#define AUDIO_ROOT "../audio/"
#define FONT_ROOT "../font/"

namespace
{
	const std::unordered_map<ImageId, std::string> mapImagePath_ = {
        {ImageId::picture_agl , "picture/agl.bmp"},
        {ImageId::picture_armory , "picture/armory.bmp"},
        {ImageId::picture_castle , "picture/castle.bmp"},
        {ImageId::picture_cave , "picture/cave.bmp"},
        {ImageId::picture_chr , "picture/chr.bmp"},
        {ImageId::picture_dex , "picture/dex.bmp"},
        {ImageId::picture_item , "picture/item.bmp"},
        {ImageId::picture_kanji , "picture/kanji.bmp"},
        {ImageId::picture_logo , "picture/logo.bmp"},
        {ImageId::picture_mgr , "picture/mgr.bmp"},
        {ImageId::picture_scroll , "picture/scroll.bmp"},
        {ImageId::picture_shield , "picture/shield.bmp"},
        {ImageId::picture_shop , "picture/shop.bmp"},
        {ImageId::picture_slayer , "picture/slayer.bmp"},
        {ImageId::picture_str , "picture/str.bmp"},
        {ImageId::picture_temple , "picture/temple.bmp"},
        {ImageId::picture_weapon , "picture/weapon.bmp"},
        {ImageId::picture_wis , "picture/wis.bmp"},
        {ImageId::user_boss_st , "user/boss_st.bmp"},
        {ImageId::user_breath , "user/breath.bmp"},
        {ImageId::user_damage , "user/damage.bmp"},
        {ImageId::user_effect , "user/effect.bmp"},
        {ImageId::user_font , "user/font.bmp"},
        {ImageId::user_frame , "user/frame.bmp"},
        {ImageId::user_goods , "user/goods.bmp"},
        {ImageId::user_magic , "user/magic.bmp"},
        {ImageId::user_pattern , "user/pattern.bmp"},
        {ImageId::user_user0 , "user/user0.bmp"},
        {ImageId::user_user1 , "user/user1.bmp"},
        {ImageId::user_user2 , "user/user2.bmp"},
        {ImageId::user_user3 , "user/user3.bmp"},
        {ImageId::xa1_boss_0 , "xa1/boss_0.bmp"},
        {ImageId::xa1_boss_1 , "xa1/boss_1.bmp"},
        {ImageId::xa1_boss_2 , "xa1/boss_2.bmp"},
        {ImageId::xa1_boss_3 , "xa1/boss_3.bmp"},
        {ImageId::xa1_boss_4 , "xa1/boss_4.bmp"},
        {ImageId::xa1_field , "xa1/field.bmp"},
        {ImageId::xa1_frame , "xa1/frame.bmp"},
        {ImageId::xa1_monst_0 , "xa1/monst_0.bmp"},
        {ImageId::xa1_monst_1 , "xa1/monst_1.bmp"},
        {ImageId::xa1_monst_2 , "xa1/monst_2.bmp"},
        {ImageId::xa1_monst_3 , "xa1/monst_3.bmp"},
        {ImageId::xa1_monst_4 , "xa1/monst_4.bmp"},
        {ImageId::xa1_monst_5 , "xa1/monst_5.bmp"},
        {ImageId::xa1_monst_6 , "xa1/monst_6.bmp"},
        {ImageId::xa1_monst_7 , "xa1/monst_7.bmp"},
        {ImageId::xa1_monst_8 , "xa1/monst_8.bmp"},
        {ImageId::xa1_monst_9 , "xa1/monst_9.bmp"},
        {ImageId::xa1_monst_a , "xa1/monst_a.bmp"},
        {ImageId::xa1_shrine , "xa1/shrine.bmp"},
        {ImageId::xa1_train , "xa1/train.bmp"},
        {ImageId::xa1_opening_background , "xa1/opening/background.bmp"},
        {ImageId::xa1_opening_battler , "xa1/opening/battler.bmp"},
        {ImageId::xa1_opening_robber , "xa1/opening/robber.bmp"},
        {ImageId::xa1_opening_swordman , "xa1/opening/swordman.bmp"},
        {ImageId::xa1_opening_witch , "xa1/opening/witch.bmp"},
        {ImageId::xa1_opening_wizard , "xa1/opening/wizard.bmp"},
        {ImageId::xa2_boss_0 , "xa2/boss_0.bmp"},
        {ImageId::xa2_boss_1 , "xa2/boss_1.bmp"},
        {ImageId::xa2_boss_2 , "xa2/boss_2.bmp"},
        {ImageId::xa2_boss_3 , "xa2/boss_3.bmp"},
        {ImageId::xa2_boss_4 , "xa2/boss_4.bmp"},
        {ImageId::xa2_boss_5 , "xa2/boss_5.bmp"},
        {ImageId::xa2_boss_6 , "xa2/boss_6.bmp"},
        {ImageId::xa2_boss_7 , "xa2/boss_7.bmp"},
        {ImageId::xa2_boss_8 , "xa2/boss_8.bmp"},
        {ImageId::xa2_boss_9 , "xa2/boss_9.bmp"},
        {ImageId::xa2_boss_a , "xa2/boss_a.bmp"},
        {ImageId::xa2_boss_b , "xa2/boss_b.bmp"},
        {ImageId::xa2_field , "xa2/field.bmp"},
        {ImageId::xa2_monst_0 , "xa2/monst_0.bmp"},
        {ImageId::xa2_monst_1 , "xa2/monst_1.bmp"},
        {ImageId::xa2_monst_2 , "xa2/monst_2.bmp"},
        {ImageId::xa2_monst_3 , "xa2/monst_3.bmp"},
        {ImageId::xa2_monst_4 , "xa2/monst_4.bmp"},
        {ImageId::xa2_monst_5 , "xa2/monst_5.bmp"},
        {ImageId::xa2_monst_6 , "xa2/monst_6.bmp"},
        {ImageId::xa2_monst_7 , "xa2/monst_7.bmp"},
        {ImageId::xa2_monst_8 , "xa2/monst_8.bmp"},
        {ImageId::xa2_monst_9 , "xa2/monst_9.bmp"},
        {ImageId::xa2_monst_a , "xa2/monst_a.bmp"},
        {ImageId::xa2_outoflevel , "xa2/outoflevel.bmp"},
        {ImageId::xa2_shrine , "xa2/shrine.bmp"},
        {ImageId::xa2_ending_background , "xa2/ending/background.bmp"},
        {ImageId::xa2_opening_hero , "xa2/opening/hero.bmp"},
        {ImageId::xa2_opening_subtitle , "xa2/opening/subtitle.bmp"},
        {ImageId::xa2_opening_title , "xa2/opening/title.bmp"},
    };
    const std::unordered_map<SoundId, std::string> mapSoundPath_ = {
        {SoundId::boss_hit, "audio/wave/boss_hit.wav"},
        {SoundId::attack, "audio/wave/attack.wav"},
        {SoundId::boss_die, "audio/wave/boss_die.wav"},
        {SoundId::c_corros, "audio/wave/c_corros.wav"},
        {SoundId::c_death, "audio/wave/c_death.wav"},
        {SoundId::c_deluge, "audio/wave/c_deluge.wav"},
        {SoundId::c_fire, "audio/wave/c_fire.wav"},
        {SoundId::c_mittar, "audio/wave/c_mittar.wav"},
        {SoundId::c_needle, "audio/wave/c_needle.wav"},
        {SoundId::c_poison, "audio/wave/c_poison.wav"},
        {SoundId::c_thunder, "audio/wave/c_thunder.wav"},
        {SoundId::c_tilte, "audio/wave/c_tilte.wav"},
        {SoundId::dead, "audio/wave/dead.wav"},
        {SoundId::dig, "audio/wave/dig.wav"},
        {SoundId::elixer, "audio/wave/elixer.wav"},
        {SoundId::encount, "audio/wave/encount.wav"},
        {SoundId::failed, "audio/wave/failed.wav"},
        {SoundId::get, "audio/wave/get.wav"},
        {SoundId::invoke, "audio/wave/invoke.wav"},
        {SoundId::lost_key, "audio/wave/lost_key.wav"},
        {SoundId::magic, "audio/wave/magic.wav"},
        {SoundId::opening0, "audio/wave/opening0.wav"},
        {SoundId::opening1, "audio/wave/opening1.wav"},
        {SoundId::opening2, "audio/wave/opening2.wav"},
        {SoundId::peep, "audio/wave/peep.wav"},
        {SoundId::poison, "audio/wave/poison.wav"},
        {SoundId::trapped, "audio/wave/trapped.wav"},
        {SoundId::treasure, "audio/wave/treasure.wav"},
        {SoundId::victory, "audio/wave/victory.wav"},
        {SoundId::warp2, "audio/wave/warp2.wav"},
    };
    const std::unordered_map<MusicId, std::string> mapMusicPath_ = {
        {MusicId::GMINIT, "audio/midi/GMINIT.mid"},
        {MusicId::xa2opening, "audio/midi/xa2opening.mid"},
        {MusicId::xanadu, "audio/midi/xanadu.mid"},
        {MusicId::xana2_XANA2_01, "audio/midi/xana2/XANA2_01.mid"},
        {MusicId::xana2_XANA2_02, "audio/midi/xana2/XANA2_02.mid"},
        {MusicId::xana2_XANA2_03, "audio/midi/xana2/XANA2_03.mid"},
        {MusicId::xana2_XANA2_04, "audio/midi/xana2/XANA2_04.mid"},
        {MusicId::xana2_XANA2_05, "audio/midi/xana2/XANA2_05.mid"},
        {MusicId::xana2_XANA2_06, "audio/midi/xana2/XANA2_06.mid"},
        {MusicId::xana2_XANA2_07, "audio/midi/xana2/XANA2_07.mid"},
        {MusicId::xana2_XANA2_08, "audio/midi/xana2/XANA2_08.mid"},
        {MusicId::xana2_XANA2_09, "audio/midi/xana2/XANA2_09.mid"},
        {MusicId::xana2_XANA2_10, "audio/midi/xana2/XANA2_10.mid"},
        {MusicId::xana2_XANA2_11, "audio/midi/xana2/XANA2_11.mid"},
        {MusicId::xana2_XANA2_HE, "audio/midi/xana2/XANA2_HE.mid"},
        {MusicId::xana2_XANA2_SH, "audio/midi/xana2/XANA2_SH.mid"},
        {MusicId::xana2_XANA2_TE, "audio/midi/xana2/XANA2_TE.mid"},
        {MusicId::xana2dl_xana201, "audio/midi/xana2dl/xana201.mid"},
        {MusicId::xana2dl_xana202, "audio/midi/xana2dl/xana202.mid"},
        {MusicId::xana2dl_xana203, "audio/midi/xana2dl/xana203.mid"},
        {MusicId::xana2dl_xana204, "audio/midi/xana2dl/xana204.mid"},
        {MusicId::xana2dl_xana205, "audio/midi/xana2dl/xana205.mid"},
        {MusicId::xana2dl_xana206, "audio/midi/xana2dl/xana206.mid"},
        {MusicId::xana2dl_xana207, "audio/midi/xana2dl/xana207.mid"},
        {MusicId::xana2dl_xana208, "audio/midi/xana2dl/xana208.mid"},
        {MusicId::xana2dl_xana209, "audio/midi/xana2dl/xana209.mid"},
        {MusicId::xana2dl_xana210, "audio/midi/xana2dl/xana210.mid"},
        {MusicId::xana2dl_xana211, "audio/midi/xana2dl/xana211.mid"},
        {MusicId::xana2dl_xana212, "audio/midi/xana2dl/xana212.mid"},
        {MusicId::xana2dl_xana213, "audio/midi/xana2dl/xana213.mid"},
        {MusicId::xana2dl_xana214, "audio/midi/xana2dl/xana214.mid"},
        {MusicId::xana2dl_xana215, "audio/midi/xana2dl/xana215.mid"},
        {MusicId::xana2dl_xana216, "audio/midi/xana2dl/xana216.mid"},
        {MusicId::xana2dl_xana217, "audio/midi/xana2dl/xana217.mid"},
        {MusicId::xana2dl_xana218, "audio/midi/xana2dl/xana218.mid"},
        {MusicId::xana2dl_xana219, "audio/midi/xana2dl/xana219.mid"},
        {MusicId::xana2dl_xana220, "audio/midi/xana2dl/xana220.mid"},
        {MusicId::xana2dl_xana221, "audio/midi/xana2dl/xana221.mid"},
        {MusicId::xana2dl_xana222, "audio/midi/xana2dl/xana222.mid"},
        {MusicId::xana2dl_xana223, "audio/midi/xana2dl/xana223.mid"},
        {MusicId::xana2dl_xana224, "audio/midi/xana2dl/xana224.mid"},
        {MusicId::xana2dl_xana225, "audio/midi/xana2dl/xana225.mid"},
        {MusicId::xana2dl_xana226, "audio/midi/xana2dl/xana226.mid"},
        {MusicId::xana2dl_xana227, "audio/midi/xana2dl/xana227.mid"},
        {MusicId::xana2dl_xana228, "audio/midi/xana2dl/xana228.mid"},
        {MusicId::xana2dl_xana229, "audio/midi/xana2dl/xana229.mid"},
        {MusicId::xana2dl_xana230, "audio/midi/xana2dl/xana230.mid"},
    };
}

Resources::Resources()
	: windowWidth_(640)
	, windowHeight_(480)
	, screenWidth_(640)
	, screenHeight_(480)
	, lang_("japanese")
	// , luaString_()
	// , luaImage_()
	, mapImage_()
	, mapJoystick_()
{
}

Resources::~Resources()
{
}

const char *Resources::getFontFileName() const
{
//	return (*luaString_)["font_name"].get<const char *>();
	return FONT_ROOT "ipag.ttf";
}

const char *Resources::getSoundFontFileName(const SoundFontId &id) const
{
	switch (id) {
	case SoundFontId::hi_def:
		return AUDIO_ROOT "HiDef.sf2";
	case SoundFontId::small_soundfont:
		return AUDIO_ROOT "Small Soundfont.sf2";
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

std::shared_ptr<SDL_::Joystick> Resources::getJoystick(uint32_t index) const
{
	auto i = mapJoystick_.find(index);
	if (i != mapJoystick_.end()) {
		return i->second;
	}
	else {
		return nullptr;
	}
}

bool Resources::loadImage(ImageId id)
{
    auto i = mapImage_.find(id);
    if (i != mapImage_.end()) {
        return true;
    }
    else {
        auto p = mapImagePath_.find(id);
        if (p != mapImagePath_.end()) {
            std::string path = IMAGE_ROOT + p->second;
            auto result = std::make_shared<SDL_::Image>(path.c_str());
            mapImage_.insert(std::make_pair(id, result));
            return true;
        }
        else {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Image ID not found. %d\n", id);
            return false;
        }
    }
}

void Resources::unloadImage(ImageId id)
{
    auto i = mapImage_.find(id);
    if (i != mapImage_.end()) {
        mapImage_.erase(i);
    }
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

bool Resources::loadSound(SDL_::Mix_::Mixer &mixer, SoundId id)
{
    auto i = mapSound_.find(id);
    if (i != mapSound_.end()) {
        return true;
    }
    else {
        // Implementation for loading sound
        if (auto p = mapSoundPath_.find(id); p != mapSoundPath_.end()) {
            std::string path = AUDIO_ROOT + p->second;
            auto result = std::make_shared<SDL_::Mix_::Audio>(mixer, path.c_str(), false);
            mapSound_.insert(std::make_pair(id, result));
            return true;
        }
        else {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Sound ID not found. %d\n", id);
        }
        return false;
    }
}

void Resources::unloadSound(SoundId id)
{
    auto i = mapSound_.find(id);
    if (i != mapSound_.end()) {
        mapSound_.erase(i);
    }
}

std::shared_ptr<SDL_::Mix_::Audio> Resources::getSound(const SoundId &id) const
{
    auto i = mapSound_.find(id);
    if (i != mapSound_.end()) {
        return i->second;
    }
    else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Sound ID not found. %d\n", id);
        return std::shared_ptr<SDL_::Mix_::Audio>();
    }
}

bool Resources::loadMusic(SDL_::Mix_::Mixer &mixer, MusicId id)
{
    auto i = mapMusic_.find(id);
    if (i != mapMusic_.end()) {
        return true;
    }
    else {
        if (auto p = mapMusicPath_.find(id); p != mapMusicPath_.end()) {
            std::string path = AUDIO_ROOT + p->second;
            auto result = std::make_shared<SDL_::Mix_::Audio>(mixer, path.c_str(), true);
            mapMusic_.insert(std::make_pair(id, result));
            return true;
        }
        else {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music ID not found. %d\n", id);
        }
        // Implementation for loading music
        return false;
    }
}

void Resources::unloadMusic(MusicId id)
{
    auto i = mapMusic_.find(id);
    if (i != mapMusic_.end()) {
        mapMusic_.erase(i);
    }
}
std::shared_ptr<SDL_::Mix_::Audio> Resources::getMusic(const MusicId &id) const
{
    auto i = mapMusic_.find(id);
    if (i != mapMusic_.end()) {
        return i->second;
    }
    else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Music ID not found. %d\n", id);
        return std::shared_ptr<SDL_::Mix_::Audio>();
    }
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
	// lang_ = "japanese";
    loadImage(ImageId::user_font);

	// loadString(lang_);
	// loadImage(lang_);
}
