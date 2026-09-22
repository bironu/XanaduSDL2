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
#include <unordered_set>

#define IMAGE_ROOT "../bmp/"
#define AUDIO_ROOT "../audio/"
#define FONT_ROOT "../font/"

// このID-pathのmapはいずれLuaスクリプトの方へ移動したい
namespace
{
	const std::unordered_map<ImageId, std::string> mapImagePath_ = {
        {ImageId::picture_agl , IMAGE_ROOT "picture/agl.bmp"},
        {ImageId::picture_armory , IMAGE_ROOT "picture/armory.bmp"},
        {ImageId::picture_castle , IMAGE_ROOT "picture/castle.bmp"},
        {ImageId::picture_cave , IMAGE_ROOT "picture/cave.bmp"},
        {ImageId::picture_chr , IMAGE_ROOT "picture/chr.bmp"},
        {ImageId::picture_dex , IMAGE_ROOT "picture/dex.bmp"},
        {ImageId::picture_foods , IMAGE_ROOT "picture/foods.bmp"},
        {ImageId::picture_guilds , IMAGE_ROOT "picture/guilds.bmp"},
        {ImageId::picture_healers , IMAGE_ROOT "picture/healers.bmp"},
        {ImageId::picture_inn , IMAGE_ROOT "picture/inn.bmp"},
        {ImageId::picture_int , IMAGE_ROOT "picture/int.bmp"},
        {ImageId::picture_item , IMAGE_ROOT "picture/item.bmp"},
        {ImageId::picture_kanji , IMAGE_ROOT "picture/kanji.bmp"},
        {ImageId::picture_logo , IMAGE_ROOT "picture/logo.bmp"},
        {ImageId::picture_mgr , IMAGE_ROOT "picture/mgr.bmp"},
        {ImageId::picture_scroll , IMAGE_ROOT "picture/scroll.bmp"},
        {ImageId::picture_shield , IMAGE_ROOT "picture/shield.bmp"},
        {ImageId::picture_shop , IMAGE_ROOT "picture/shop.bmp"},
        {ImageId::picture_slayer , IMAGE_ROOT "picture/slayer.bmp"},
        {ImageId::picture_str , IMAGE_ROOT "picture/str.bmp"},
        {ImageId::picture_temple , IMAGE_ROOT "picture/temple.bmp"},
        {ImageId::picture_weapon , IMAGE_ROOT "picture/weapon.bmp"},
        {ImageId::picture_wis , IMAGE_ROOT "picture/wis.bmp"},
        {ImageId::user_boss_st , IMAGE_ROOT "user/boss_st.bmp"},
        {ImageId::user_breath , IMAGE_ROOT "user/breath.bmp"},
        {ImageId::user_damage , IMAGE_ROOT "user/damage.bmp"},
        {ImageId::user_effect , IMAGE_ROOT "user/effect.bmp"},
        {ImageId::user_font , IMAGE_ROOT "user/font.bmp"},
        {ImageId::user_frame , IMAGE_ROOT "user/frame.bmp"},
        {ImageId::user_goods , IMAGE_ROOT "user/goods.bmp"},
        {ImageId::user_magic , IMAGE_ROOT "user/magic.bmp"},
        {ImageId::user_pattern , IMAGE_ROOT "user/pattern.bmp"},
        {ImageId::user_user0 , IMAGE_ROOT "user/user0.bmp"},
        {ImageId::user_user1 , IMAGE_ROOT "user/user1.bmp"},
        {ImageId::user_user2 , IMAGE_ROOT "user/user2.bmp"},
        {ImageId::user_user3 , IMAGE_ROOT "user/user3.bmp"},
        {ImageId::xa1_boss_0 , IMAGE_ROOT "xa1/boss_0.bmp"},
        {ImageId::xa1_boss_1 , IMAGE_ROOT "xa1/boss_1.bmp"},
        {ImageId::xa1_boss_2 , IMAGE_ROOT "xa1/boss_2.bmp"},
        {ImageId::xa1_boss_3 , IMAGE_ROOT "xa1/boss_3.bmp"},
        {ImageId::xa1_boss_4 , IMAGE_ROOT "xa1/boss_4.bmp"},
        {ImageId::xa1_field , IMAGE_ROOT "xa1/field.bmp"},
        {ImageId::xa1_frame , IMAGE_ROOT "xa1/frame.bmp"},
        {ImageId::xa1_monst_0 , IMAGE_ROOT "xa1/monst_0.bmp"},
        {ImageId::xa1_monst_1 , IMAGE_ROOT "xa1/monst_1.bmp"},
        {ImageId::xa1_monst_2 , IMAGE_ROOT "xa1/monst_2.bmp"},
        {ImageId::xa1_monst_3 , IMAGE_ROOT "xa1/monst_3.bmp"},
        {ImageId::xa1_monst_4 , IMAGE_ROOT "xa1/monst_4.bmp"},
        {ImageId::xa1_monst_5 , IMAGE_ROOT "xa1/monst_5.bmp"},
        {ImageId::xa1_monst_6 , IMAGE_ROOT "xa1/monst_6.bmp"},
        {ImageId::xa1_monst_7 , IMAGE_ROOT "xa1/monst_7.bmp"},
        {ImageId::xa1_monst_8 , IMAGE_ROOT "xa1/monst_8.bmp"},
        {ImageId::xa1_monst_9 , IMAGE_ROOT "xa1/monst_9.bmp"},
        {ImageId::xa1_monst_a , IMAGE_ROOT "xa1/monst_a.bmp"},
        {ImageId::xa1_shrine , IMAGE_ROOT "xa1/shrine.bmp"},
        {ImageId::xa1_train , IMAGE_ROOT "xa1/train.bmp"},
        {ImageId::xa1_opening_background , IMAGE_ROOT "xa1/opening/background.bmp"},
        {ImageId::xa1_opening_battler , IMAGE_ROOT "xa1/opening/battler.bmp"},
        {ImageId::xa1_opening_robber , IMAGE_ROOT "xa1/opening/robber.bmp"},
        {ImageId::xa1_opening_swordman , IMAGE_ROOT "xa1/opening/swordman.bmp"},
        {ImageId::xa1_opening_witch , IMAGE_ROOT "xa1/opening/witch.bmp"},
        {ImageId::xa1_opening_wizard , IMAGE_ROOT "xa1/opening/wizard.bmp"},
        {ImageId::xa2_boss_0 , IMAGE_ROOT "xa2/boss_0.bmp"},
        {ImageId::xa2_boss_1 , IMAGE_ROOT "xa2/boss_1.bmp"},
        {ImageId::xa2_boss_2 , IMAGE_ROOT "xa2/boss_2.bmp"},
        {ImageId::xa2_boss_3 , IMAGE_ROOT "xa2/boss_3.bmp"},
        {ImageId::xa2_boss_4 , IMAGE_ROOT "xa2/boss_4.bmp"},
        {ImageId::xa2_boss_5 , IMAGE_ROOT "xa2/boss_5.bmp"},
        {ImageId::xa2_boss_6 , IMAGE_ROOT "xa2/boss_6.bmp"},
        {ImageId::xa2_boss_7 , IMAGE_ROOT "xa2/boss_7.bmp"},
        {ImageId::xa2_boss_8 , IMAGE_ROOT "xa2/boss_8.bmp"},
        {ImageId::xa2_boss_9 , IMAGE_ROOT "xa2/boss_9.bmp"},
        {ImageId::xa2_boss_a , IMAGE_ROOT "xa2/boss_a.bmp"},
        {ImageId::xa2_boss_b , IMAGE_ROOT "xa2/boss_b.bmp"},
        {ImageId::xa2_field , IMAGE_ROOT "xa2/field.bmp"},
        {ImageId::xa2_monst_0 , IMAGE_ROOT "xa2/monst_0.bmp"},
        {ImageId::xa2_monst_1 , IMAGE_ROOT "xa2/monst_1.bmp"},
        {ImageId::xa2_monst_2 , IMAGE_ROOT "xa2/monst_2.bmp"},
        {ImageId::xa2_monst_3 , IMAGE_ROOT "xa2/monst_3.bmp"},
        {ImageId::xa2_monst_4 , IMAGE_ROOT "xa2/monst_4.bmp"},
        {ImageId::xa2_monst_5 , IMAGE_ROOT "xa2/monst_5.bmp"},
        {ImageId::xa2_monst_6 , IMAGE_ROOT "xa2/monst_6.bmp"},
        {ImageId::xa2_monst_7 , IMAGE_ROOT "xa2/monst_7.bmp"},
        {ImageId::xa2_monst_8 , IMAGE_ROOT "xa2/monst_8.bmp"},
        {ImageId::xa2_monst_9 , IMAGE_ROOT "xa2/monst_9.bmp"},
        {ImageId::xa2_monst_a , IMAGE_ROOT "xa2/monst_a.bmp"},
        {ImageId::xa2_outoflevel , IMAGE_ROOT "xa2/outoflevel.bmp"},
        {ImageId::xa2_shrine , IMAGE_ROOT "xa2/shrine.bmp"},
        {ImageId::xa2_ending_background , IMAGE_ROOT "xa2/ending/background.bmp"},
        {ImageId::xa2_opening_hero , IMAGE_ROOT "xa2/opening/hero.bmp"},
        {ImageId::xa2_opening_subtitle , IMAGE_ROOT "xa2/opening/subtitle.bmp"},
        {ImageId::xa2_opening_title , IMAGE_ROOT "xa2/opening/title.bmp"},
    };

    // 背景画像等ではなく、スプライトとして透過合成が必要な画像だけ、
    // Image読み込み時に左上ピクセル色でカラーキーを設定する
    // (SDL_::Image::Image(file, isColorKey)参照)
    const std::unordered_set<ImageId> kColorKeyImages = {
        // EndingScene::drawKanjiText()がグリフを不透明合成する際の背景色、
        // かつmsg_(クレジットロールのテキストバッファ)自身のカラーキーの元にもなる
        ImageId::picture_kanji,
        ImageId::user_breath,
        ImageId::user_damage,
        ImageId::user_effect,
        ImageId::user_goods,
        ImageId::user_magic,
        ImageId::user_user0,
        ImageId::user_user1,
        ImageId::user_user2,
        ImageId::user_user3,
        ImageId::xa1_boss_0,
        ImageId::xa1_boss_1,
        ImageId::xa1_boss_2,
        ImageId::xa1_boss_3,
        ImageId::xa1_boss_4,
        ImageId::xa1_monst_0,
        ImageId::xa1_monst_1,
        ImageId::xa1_monst_2,
        ImageId::xa1_monst_3,
        ImageId::xa1_monst_4,
        ImageId::xa1_monst_5,
        ImageId::xa1_monst_6,
        ImageId::xa1_monst_7,
        ImageId::xa1_monst_8,
        ImageId::xa1_monst_9,
        ImageId::xa1_monst_a,
        ImageId::xa2_boss_0,
        ImageId::xa2_boss_1,
        ImageId::xa2_boss_2,
        ImageId::xa2_boss_3,
        ImageId::xa2_boss_4,
        ImageId::xa2_boss_5,
        ImageId::xa2_boss_6,
        ImageId::xa2_boss_7,
        ImageId::xa2_boss_8,
        ImageId::xa2_boss_9,
        ImageId::xa2_boss_a,
        ImageId::xa2_boss_b,
        ImageId::xa2_monst_0,
        ImageId::xa2_monst_1,
        ImageId::xa2_monst_2,
        ImageId::xa2_monst_3,
        ImageId::xa2_monst_4,
        ImageId::xa2_monst_5,
        ImageId::xa2_monst_6,
        ImageId::xa2_monst_7,
        ImageId::xa2_monst_8,
        ImageId::xa2_monst_9,
        ImageId::xa2_monst_a,
    };

    const std::unordered_map<SoundId, std::string> mapSoundPath_ = {
        {SoundId::boss_hit, AUDIO_ROOT "wave/boss_hit.wav"},
        {SoundId::attack, AUDIO_ROOT "wave/attack.wav"},
        {SoundId::boss_die, AUDIO_ROOT "wave/boss_die.wav"},
        {SoundId::c_corros, AUDIO_ROOT "wave/c_corros.wav"},
        {SoundId::c_death, AUDIO_ROOT "wave/c_death.wav"},
        {SoundId::c_deluge, AUDIO_ROOT "wave/c_deluge.wav"},
        {SoundId::c_fire, AUDIO_ROOT "wave/c_fire.wav"},
        {SoundId::c_mittar, AUDIO_ROOT "wave/c_mittar.wav"},
        {SoundId::c_needle, AUDIO_ROOT "wave/c_needle.wav"},
        {SoundId::c_poison, AUDIO_ROOT "wave/c_poison.wav"},
        {SoundId::c_thunder, AUDIO_ROOT "wave/c_thunder.wav"},
        {SoundId::c_tilte, AUDIO_ROOT "wave/c_tilte.wav"},
        {SoundId::dead, AUDIO_ROOT "wave/dead.wav"},
        {SoundId::dig, AUDIO_ROOT "wave/dig.wav"},
        {SoundId::elixer, AUDIO_ROOT "wave/elixer.wav"},
        {SoundId::encount, AUDIO_ROOT "wave/encount.wav"},
        {SoundId::failed, AUDIO_ROOT "wave/failed.wav"},
        {SoundId::get, AUDIO_ROOT "wave/get.wav"},
        {SoundId::invoke, AUDIO_ROOT "wave/invoke.wav"},
        {SoundId::lost_key, AUDIO_ROOT "wave/lost_key.wav"},
        {SoundId::magic, AUDIO_ROOT "wave/magic.wav"},
        {SoundId::opening0, AUDIO_ROOT "wave/opening0.wav"},
        {SoundId::opening1, AUDIO_ROOT "wave/opening1.wav"},
        {SoundId::opening2, AUDIO_ROOT "wave/opening2.wav"},
        {SoundId::peep, AUDIO_ROOT "wave/peep.wav"},
        {SoundId::poison, AUDIO_ROOT "wave/poison.wav"},
        {SoundId::trapped, AUDIO_ROOT "wave/trapped.wav"},
        {SoundId::treasure, AUDIO_ROOT "wave/treasure.wav"},
        {SoundId::victory, AUDIO_ROOT "wave/victory.wav"},
        {SoundId::warp2, AUDIO_ROOT "wave/warp2.wav"},
    };
    const std::unordered_map<MusicId, std::string> mapMusicPath_ = {
        {MusicId::GMINIT, AUDIO_ROOT "midi/GMINIT.mid"},
        {MusicId::xa2opening, AUDIO_ROOT "midi/xa2opening.mid"},
        {MusicId::xanadu, AUDIO_ROOT "midi/xanadu.mid"},
        {MusicId::xana2_XANA2_01, AUDIO_ROOT "midi/xana2/XANA2_01.mid"},
        {MusicId::xana2_XANA2_02, AUDIO_ROOT "midi/xana2/XANA2_02.mid"},
        {MusicId::xana2_XANA2_03, AUDIO_ROOT "midi/xana2/XANA2_03.mid"},
        {MusicId::xana2_XANA2_04, AUDIO_ROOT "midi/xana2/XANA2_04.mid"},
        {MusicId::xana2_XANA2_05, AUDIO_ROOT "midi/xana2/XANA2_05.mid"},
        {MusicId::xana2_XANA2_06, AUDIO_ROOT "midi/xana2/XANA2_06.mid"},
        {MusicId::xana2_XANA2_07, AUDIO_ROOT "midi/xana2/XANA2_07.mid"},
        {MusicId::xana2_XANA2_08, AUDIO_ROOT "midi/xana2/XANA2_08.mid"},
        {MusicId::xana2_XANA2_09, AUDIO_ROOT "midi/xana2/XANA2_09.mid"},
        {MusicId::xana2_XANA2_10, AUDIO_ROOT "midi/xana2/XANA2_10.mid"},
        {MusicId::xana2_XANA2_11, AUDIO_ROOT "midi/xana2/XANA2_11.mid"},
        {MusicId::xana2_XANA2_HE, AUDIO_ROOT "midi/xana2/XANA2_HE.mid"},
        {MusicId::xana2_XANA2_SH, AUDIO_ROOT "midi/xana2/XANA2_SH.mid"},
        {MusicId::xana2_XANA2_TE, AUDIO_ROOT "midi/xana2/XANA2_TE.mid"},
        {MusicId::xana2dl_xana201, AUDIO_ROOT "midi/xana2dl/xana201.mid"},
        {MusicId::xana2dl_xana202, AUDIO_ROOT "midi/xana2dl/xana202.mid"},
        {MusicId::xana2dl_xana203, AUDIO_ROOT "midi/xana2dl/xana203.mid"},
        {MusicId::xana2dl_xana204, AUDIO_ROOT "midi/xana2dl/xana204.mid"},
        {MusicId::xana2dl_xana205, AUDIO_ROOT "midi/xana2dl/xana205.mid"},
        {MusicId::xana2dl_xana206, AUDIO_ROOT "midi/xana2dl/xana206.mid"},
        {MusicId::xana2dl_xana207, AUDIO_ROOT "midi/xana2dl/xana207.mid"},
        {MusicId::xana2dl_xana208, AUDIO_ROOT "midi/xana2dl/xana208.mid"},
        {MusicId::xana2dl_xana209, AUDIO_ROOT "midi/xana2dl/xana209.mid"},
        {MusicId::xana2dl_xana210, AUDIO_ROOT "midi/xana2dl/xana210.mid"},
        {MusicId::xana2dl_xana211, AUDIO_ROOT "midi/xana2dl/xana211.mid"},
        {MusicId::xana2dl_xana212, AUDIO_ROOT "midi/xana2dl/xana212.mid"},
        {MusicId::xana2dl_xana213, AUDIO_ROOT "midi/xana2dl/xana213.mid"},
        {MusicId::xana2dl_xana214, AUDIO_ROOT "midi/xana2dl/xana214.mid"},
        {MusicId::xana2dl_xana215, AUDIO_ROOT "midi/xana2dl/xana215.mid"},
        {MusicId::xana2dl_xana216, AUDIO_ROOT "midi/xana2dl/xana216.mid"},
        {MusicId::xana2dl_xana217, AUDIO_ROOT "midi/xana2dl/xana217.mid"},
        {MusicId::xana2dl_xana218, AUDIO_ROOT "midi/xana2dl/xana218.mid"},
        {MusicId::xana2dl_xana219, AUDIO_ROOT "midi/xana2dl/xana219.mid"},
        {MusicId::xana2dl_xana220, AUDIO_ROOT "midi/xana2dl/xana220.mid"},
        {MusicId::xana2dl_xana221, AUDIO_ROOT "midi/xana2dl/xana221.mid"},
        {MusicId::xana2dl_xana222, AUDIO_ROOT "midi/xana2dl/xana222.mid"},
        {MusicId::xana2dl_xana223, AUDIO_ROOT "midi/xana2dl/xana223.mid"},
        {MusicId::xana2dl_xana224, AUDIO_ROOT "midi/xana2dl/xana224.mid"},
        {MusicId::xana2dl_xana225, AUDIO_ROOT "midi/xana2dl/xana225.mid"},
        {MusicId::xana2dl_xana226, AUDIO_ROOT "midi/xana2dl/xana226.mid"},
        {MusicId::xana2dl_xana227, AUDIO_ROOT "midi/xana2dl/xana227.mid"},
        {MusicId::xana2dl_xana228, AUDIO_ROOT "midi/xana2dl/xana228.mid"},
        {MusicId::xana2dl_xana229, AUDIO_ROOT "midi/xana2dl/xana229.mid"},
        {MusicId::xana2dl_xana230, AUDIO_ROOT "midi/xana2dl/xana230.mid"},
    };
}

Resources *Resources::instance_ = nullptr;

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
	, currentMusicId_(MusicId::none)
{
	instance_ = this;
}

Resources::~Resources()
{
	instance_ = nullptr;
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
            std::string path = p->second;
            const bool isColorKey = kColorKeyImages.count(id) > 0;
            auto result = std::make_shared<SDL_::Image>(path.c_str(), isColorKey);
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
            std::string path = p->second;
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
            std::string path = p->second;
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

bool Resources::playBgm(SDL_::Mix_::Mixer &mixer, MusicId id)
{
    if (id == currentMusicId_ && mixer.isMusicPlaying()) {
        return true;
    }
    if (id == MusicId::none) {
        mixer.stopMusic();
        unloadMusic(currentMusicId_);
        currentMusicId_ = MusicId::none;
        return true;
    }
    if (!loadMusic(mixer, id)) {
        return false;
    }
    mixer.playMusic(*getMusic(id), -1);
    if (currentMusicId_ != id) {
        // トラックは既に新しいAudioへ再配線済みなので、ここで旧トラックを解放してよい
        unloadMusic(currentMusicId_);
    }
    currentMusicId_ = id;
    return true;
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
	windowHeight_ = 400;
	screenWidth_ = 640;
	screenHeight_ = 400;
	// lang_ = "japanese";
    loadImage(ImageId::user_font);

	// loadString(lang_);
	// loadImage(lang_);
}
