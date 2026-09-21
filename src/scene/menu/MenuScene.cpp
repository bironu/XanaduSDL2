#include "app/Application.h"
#include "sdl/SDLWindow.h"
#include "sdl/SDLMixMixer.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/MusicId.h"
#include "scene/menu/MenuScene.h"
#include "field.h" // FieldSceneが出来れば削除
#include "tower.h" // TowerSceneが出来れば削除
#include "boss.h" // BossSceneが出来れば削除

#include <SDL3/SDL_events.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

const FRect MenuScene::rect_overall_    = {   0.0f,   0.0f, 640.0f, 400.0f };
const FRect MenuScene::rect_main_       = {  16.0f,  16.0f, 360.0f, 360.0f };
MenuScene::State MenuScene::state_ = MenuScene::State::Generic;
std::array<std::string, MenuScene::kMaxUserEntry> MenuScene::userEntries_{};

MenuScene::MenuScene()
{
}

void MenuScene::dispatch(const SDL_Event &event)
{
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
        onKeyDown(event.key);
        break;
    case SDL_EVENT_WINDOW_EXPOSED:
        onWindowExpose(event.window);
        break;
    default:
        break;
    }
}

void MenuScene::onCreate(uint32_t /*tick*/)
{
    auto &res = getResources();
    res.loadImage(ImageId::picture_logo);
    res.loadImage(ImageId::xa1_frame);
    auto &app = getApplication();
    auto &mixer = app.getMixer();
    res.loadMusic(mixer, MusicId::GMINIT);
    clip_main_ = std::make_shared<SDL_::Image>(rect_main_.getWidth(), rect_main_.getHeight());
    clip_overall_ = std::make_shared<SDL_::Image>(rect_overall_.getWidth(), rect_overall_.getHeight());
}

void MenuScene::onResume(uint32_t /*tick*/)
{
	onEnter();
    auto &app = getApplication();
    auto &mixer = app.getMixer();
    auto &res = getResources();
    mixer.playMusic(*res.getMusic(MusicId::GMINIT));
}

void MenuScene::onSuspend()
{
    auto &app = getApplication();
    auto &mixer = app.getMixer();
    auto &res = getResources();
    mixer.stopMusic();
	onLeave();
}

void MenuScene::onDestroy(uint32_t /*tick*/)
{
    auto &res = getResources();
    res.unloadImage(ImageId::picture_logo);
    res.unloadImage(ImageId::xa1_frame);
    res.unloadMusic(MusicId::GMINIT);
}

void MenuScene::onKeyDown(const SDL_KeyboardEvent &key)
{
	if (key.repeat != 0) {
		return;
	}

	switch (state_) {
	case State::Generic: onGenericKey(key); break;
	case State::Load:    onLoadKey(key);    break;
	case State::Debug:   onDebugKey(key);   break;
	case State::Boss:    onBossKey(key);    break;
	case State::Version: onVersionKey(key); break;
	case State::Game:    break; // onEnter()内で即座に遷移するため滞留しない
	}
}

void MenuScene::onWindowExpose(const SDL_WindowEvent &window)
{
    auto &app = getApplication();
    auto mainWindow = app.getMainWindow();
    if (!mainWindow) {
        return;
    }
    if (window.windowID == mainWindow->getWindowId()) {
        auto &res = getResources();
        auto backBuffer = mainWindow->getBackBuffer();
        // backBuffer->fillRect(SDL_::Color::BLACK);
        backBuffer->blitScaled(clip_overall_, nullptr, nullptr, SDL_SCALEMODE_PIXELART);
        mainWindow->swap();
    }
    SDL_Log("Window exposed event: windowID=%u, data1=%d, data2=%d", window.windowID, window.data1, window.data2);
}

void MenuScene::onEnter()
{
	const SDL_::Color pixel1 = user.environment.scenario == 0 ? SDL_::Color::RED : SDL_::Color::WHITE;
	const SDL_::Color pixel2 = user.environment.scenario != 0 ? SDL_::Color::RED : SDL_::Color::WHITE;

	// bgm_play(bgm_data.start_menu); // BGM
    auto &app = getApplication();
    auto &mixer = app.getMixer();
    mixer.stopMusic();

    clip_overall_->fillRect(SDL_::Color::BLACK);
    clip_main_->fillRect(SDL_::Color::BLACK);

    auto &res = getResources();
    // Frame
    clip_overall_->blit(res.getImage(ImageId::xa1_frame), 0, 0);
	// Logo
    clip_main_->blit(res.getImage(ImageId::picture_logo), 100, 290);

	switch (state_) {
	case State::Game:
		state_ = State::Generic;
		switch_context(init_training_ground(user.environment.scenario));
		return;

	case State::Debug:
		initDebug();
		drawText(0, 4, "Debug mode", SDL_::Color::RED);
		drawItem(2, 0, '+', "SCENARIO 1", pixel1);
		drawItem(3, 0, '*', "SCENARIO 2", pixel2);
		{
			SDL_::Color pixels[11];
			for (int i = 0; i < 11; i++) { pixels[i] = SDL_::Color::WHITE; }
			pixels[user.environment.dungeon_level] = SDL_::Color::RED;
			if (user.environment.scenario == 0) {
				drawItem( 4, 0, '1', "Level 1",  pixels[0]);
				drawItem( 5, 0, '2', "Level 2",  pixels[1]);
				drawItem( 6, 0, '3', "Level 3",  pixels[2]);
				drawItem( 7, 0, '4', "Level 4",  pixels[3]);
				drawItem( 8, 0, '5', "Level 5",  pixels[4]);
				drawItem( 9, 0, '6', "Level 6",  pixels[5]);
				drawItem(10, 0, '7', "Level 7",  pixels[6]);
				drawItem(11, 0, '8', "Level 8",  pixels[7]);
				drawItem(12, 0, '9', "Level 9",  pixels[8]);
				drawItem(13, 0, 'A', "Level 10", pixels[9]);
				drawItem(14, 0, 'B', "Training Ground", pixels[10]);
			} else {
				drawItem( 4, 0, '1', "Maple Ford",  pixels[0]);
				drawItem( 5, 0, '2', "Filane",      pixels[1]);
				drawItem( 6, 0, '3', "Poigone",     pixels[2]);
				drawItem( 7, 0, '4', "Gandic",      pixels[3]);
				drawItem( 8, 0, '5', "Nuldour",     pixels[4]);
				drawItem( 9, 0, '6', "Alf",         pixels[5]);
				drawItem(10, 0, '7', "Alcanek",     pixels[6]);
				drawItem(11, 0, '8', "Altel",       pixels[7]);
				drawItem(12, 0, '9', "Klepsydar",   pixels[8]);
				drawItem(13, 0, 'A', "Rilvan",      pixels[9]);
				drawItem(14, 0, 'B', "Shhangri-La", pixels[10]);
			}
		}
		drawItem(15, 0, 'F', "Enter field", SDL_::Color::WHITE);
		drawItem(16, 0, 'T', "Enter tower", SDL_::Color::WHITE);
		drawItem(17, 0, 'R', "Return back", SDL_::Color::RED);
		break;

	case State::Load:
		{
			const int n = initLoadMenu();
			drawText(0, 4, "Load game", SDL_::Color::RED);
			int i = 0;
			for (; i < n; i++) {
				drawItem(2 + i, 0, 'A' + i, userEntries_[i].c_str(), SDL_::Color::WHITE);
			}
			drawItem(2 + i, 0, 'R', "Return back", SDL_::Color::RED);
		}
		break;

	case State::Boss:
		initDebug();
		drawText(0, 4, "Boss menu", SDL_::Color::RED);
		if (user.environment.scenario == 0) {
			drawItem( 2, 0, 'A', "Kraken Giant",  SDL_::Color::WHITE);
			drawItem( 3, 0, 'B', "Grell Giant",   SDL_::Color::WHITE);
			drawItem( 4, 0, 'C', "Karttikeya",    SDL_::Color::WHITE);
			drawItem( 5, 0, 'D', "Silver Dragon", SDL_::Color::WHITE);
			drawItem( 6, 0, 'E', "Big Kraken",    SDL_::Color::WHITE);
			drawItem( 7, 0, 'F', "King Dragon",   SDL_::Color::WHITE);
			drawItem( 8, 0, 'R', "Return back",   SDL_::Color::RED);
		} else {
			drawItem( 2, 0, 'A', "Marivoux",      SDL_::Color::WHITE);
			drawItem( 3, 0, 'B', "Peluton",       SDL_::Color::WHITE);
			drawItem( 4, 0, 'C', "Great Kraken",  SDL_::Color::WHITE);
			drawItem( 5, 0, 'D', "Zschokke",      SDL_::Color::WHITE);
			drawItem( 6, 0, 'E', "White Dragon",  SDL_::Color::WHITE);
			drawItem( 7, 0, 'F', "Bogres",        SDL_::Color::WHITE);
			drawItem( 8, 0, 'G', "Red Dragon",    SDL_::Color::WHITE);
			drawItem( 9, 0, 'H', "Guin",          SDL_::Color::WHITE);
			drawItem(10, 0, 'I', "Hydra",         SDL_::Color::WHITE);
			drawItem(11, 0, 'J', "Buzzati",       SDL_::Color::WHITE);
			drawItem(12, 0, 'K', "Boiardo",       SDL_::Color::WHITE);
			drawItem(13, 0, 'L', "King Dragon",   SDL_::Color::WHITE);
			drawItem(14, 0, 'R', "Return back",   SDL_::Color::RED);
		}
		break;

	case State::Version:
		drawText( 1,  8, "XANADU", SDL_::Color::WHITE);
		drawText( 3,  1, "REVISION:", SDL_::Color::RED);
		drawText( 3, 10, "1.1.4", SDL_::Color::WHITE);
		drawText( 4,  1, "  SYSTEM:", SDL_::Color::RED);
		drawText( 4, 10, "SDL3", SDL_::Color::WHITE);
		drawText( 5,  1, " DISPLAY:", SDL_::Color::RED);
		drawText( 5, 10, "32bpp", SDL_::Color::WHITE);
		drawText( 6,  1, "     BGM:", SDL_::Color::RED);
		drawText( 6, 10, bgm_enabled() ? "OK" : "Disable", SDL_::Color::WHITE);
		drawText( 7,  1, "     S.E:", SDL_::Color::RED);
		drawText( 7, 10, se_enabled() ? "OK" : "Disable", SDL_::Color::WHITE);

		drawText(10,  0, "XANADU WAS ORIGINALLY", SDL_::Color::WHITE);
		drawText(11,  0, "RELEASED IN 1985", SDL_::Color::WHITE);
		drawText(12,  0, "BY FALCOM.", SDL_::Color::WHITE);

		drawItem(14,  0, 'R', "Return back", SDL_::Color::RED);
		break;

	case State::Generic:
	default:
		drawText(0, 4, "Start menu", SDL_::Color::RED);
		drawItem( 2, 0, 'L', "Load game",  SDL_::Color::WHITE);
		drawItem( 3, 0, '1', "SCENARIO 1", pixel1);
		drawItem( 4, 0, '2', "SCENARIO 2", pixel2);
		drawItem( 5, 0, 'N', "New game",   SDL_::Color::WHITE);
		drawItem( 6, 0, 'D', "Debug mode", SDL_::Color::WHITE);
		drawItem( 7, 0, 'B', "Boss stage", SDL_::Color::WHITE);
		drawItem( 8, 0, 'O', "Opening", SDL_::Color::WHITE);
		drawItem( 9, 0, 'E', "Ending(LONG)", SDL_::Color::WHITE);
		drawItem(10, 0, 'V', "Version info", SDL_::Color::WHITE);
		drawText(12, 0, "Please Num-Lock *OFF*", SDL_::Color::RED);
	}
    clip_overall_->blit(clip_main_, rect_main.x, rect_main.y);
    auto mainWindow = getApplication().getMainWindow();
    if (mainWindow) {
        mainWindow->requestUpdate();
        SDL_Log("MenuScene::onEnter: requested expose for main window");
    }
}

void MenuScene::onLeave()
{
}

void MenuScene::drawText(int row, int col, const char *s, const SDL_::Color &pixel)
{
	draw_text(clip_main_, col * 16, row * 16, s, pixel);
}

void MenuScene::drawItem(int row, int col, int key, const char *s, const SDL_::Color &pixel)
{
	char buf[3] = "*:";
	buf[0] = key;
	draw_text(clip_main_, col * 16, row * 16, buf, SDL_::Color::RED);
	col += 2;
	draw_text(clip_main_, col * 16, row * 16, s, pixel);
}

void MenuScene::onGenericKey(const SDL_KeyboardEvent &key)
{
	switch (key.key) {
	case SDLK_L: state_ = State::Load; onEnter(); break;
	case SDLK_1: user.environment.scenario = 0; onEnter(); break;
	case SDLK_2: user.environment.scenario = 1; onEnter(); break;
	case SDLK_N:
		state_ = State::Game;
		extend_context(CONTEXT_OPENING);
		return;
	case SDLK_D: state_ = State::Debug; onEnter(); break;
	case SDLK_B: state_ = State::Boss; onEnter(); break;
	case SDLK_O: extend_context(CONTEXT_OPENING); break;
	case SDLK_E: extend_context(CONTEXT_ENDING); break;
	case SDLK_V: state_ = State::Version; onEnter(); break;
	default: return;
	}
}

void MenuScene::onDebugKey(const SDL_KeyboardEvent &key)
{
	const bool shift = (key.mod & SDL_KMOD_SHIFT) != 0;

	switch (key.key) {
	// SCENARIO 1: USキー配列ではShift+'='(=SDLK_EQUALS)が'+'になる。
	// JIS配列など、レイアウトによっては'+'やSDLK_SEMICOLON自体が
	// 素で送出される場合もあるため両方を受理する。
	case SDLK_SEMICOLON: user.environment.scenario = shift ? 1 : 0; break;
	case SDLK_PLUS:      user.environment.scenario = 0; break;
	case SDLK_EQUALS:    if (shift) { user.environment.scenario = 0; } break;
	case SDLK_COLON:     user.environment.scenario = 1; break;
	case SDLK_ASTERISK:  user.environment.scenario = 1; break;
	case SDLK_1: user.environment.dungeon_level =  0; break;
	case SDLK_2: user.environment.dungeon_level =  1; break;
	case SDLK_3: user.environment.dungeon_level =  2; break;
	case SDLK_4: user.environment.dungeon_level =  3; break;
	case SDLK_5: user.environment.dungeon_level =  4; break;
	case SDLK_6: user.environment.dungeon_level =  5; break;
	case SDLK_7: user.environment.dungeon_level =  6; break;
	case SDLK_8:
		// SCENARIO 2: USキー配列のShift+'8'('*')もここで受理する。
		if (shift) { user.environment.scenario = 1; } else { user.environment.dungeon_level = 7; }
		break;
	case SDLK_9: user.environment.dungeon_level =  8; break;
	case SDLK_A: user.environment.dungeon_level =  9; break;
	case SDLK_B: user.environment.dungeon_level = 10; break;
	case SDLK_F:
		init_level(user.environment.dungeon_level, nullptr);
		switch_context(CONTEXT_FIELD);
		return;
	case SDLK_T:
		init_level(user.environment.dungeon_level, nullptr);
		user.x = 0;
		user.y = 4 * 40;
		switch_context(CONTEXT_TOWER);
		return;
	case SDLK_O:
		init_level(-1, nullptr);
		user.point = field_offset_XY(4, 3);
		switch_context(CONTEXT_FIELD);
		return;
	case SDLK_R:
		state_ = State::Generic;
		break;
	}
	onEnter();
}

void MenuScene::onLoadKey(const SDL_KeyboardEvent &key)
{
	switch (key.key) {
	case SDLK_A: case SDLK_B: case SDLK_C: case SDLK_D:
	case SDLK_E: case SDLK_F: case SDLK_G: case SDLK_H:
		if (loadGame(key.key - SDLK_A) == 0) {
			// ゲームを再開する
			load_user_image();
			init_level(user.environment.dungeon_level, user_path.empty() ? nullptr : user_path.c_str());
			if (in_tower())
				switch_context(init_tower());
			else
				switch_context(init_field());
		}
		break;
	case SDLK_R:
		state_ = State::Generic;
		onEnter();
		break;
	}
}

void MenuScene::onBossKey(const SDL_KeyboardEvent &key)
{
	int bossId = 0;
	switch (key.key) {
	case SDLK_A: bossId =  0; break;
	case SDLK_B: bossId =  1; break;
	case SDLK_C: bossId =  2; break;
	case SDLK_D: bossId =  3; break;
	case SDLK_E: bossId =  4; break;
	case SDLK_F: bossId =  5; break;
	case SDLK_G: bossId =  6; break;
	case SDLK_H: bossId =  7; break;
	case SDLK_I: bossId =  8; break;
	case SDLK_J: bossId =  9; break;
	case SDLK_K: bossId = 10; break;
	case SDLK_L: bossId = 11; break;
	case SDLK_R:
		state_ = State::Generic;
		onEnter();
		return;
	default:
		return;
	}
	extend_context(init_boss(bossId));
}

void MenuScene::onVersionKey(const SDL_KeyboardEvent &key)
{
	if (key.key == SDLK_R) {
		state_ = State::Generic;
		onEnter();
	}
}

void MenuScene::initDebug()
{
	user_path.clear();

	user.environment.in_battle = 0;

	strcpy(user.status.name, "Nobody");
	user.status.max_HP = 6000000;
	user.status.HP     = 6000000;
	user.status.gold   = 6000000;
	user.status.food   = 1000000;

	for (int i = 0; i < MAX_GOODS; i++) {
		user.inventory[GOODS_WEAPON][i].stock = 1;
		user.inventory[GOODS_WEAPON][i].skill = 255;
		user.inventory[GOODS_SCROLL][i].stock = 1;
		user.inventory[GOODS_SCROLL][i].skill = 255;
		user.inventory[GOODS_ARMOUR][i].stock = 1;
		user.inventory[GOODS_ARMOUR][i].skill = 200;
		user.inventory[GOODS_SHIELD][i].stock = 1;
		user.inventory[GOODS_SHIELD][i].skill = 200;
		user.inventory[GOODS_MAGIC_ITEM][i].stock = 255;
		user.inventory[GOODS_MAGIC_ITEM][i].skill = 0;
	}
	user.inventory[GOODS_MAGIC_ITEM][1].skill = 255;
	user.inventory[GOODS_MAGIC_ITEM]
		[user.equipment[GOODS_MAGIC_ITEM]].skill = 30;

	user.status.STR = 100;
	user.status.INT = 100;
	user.status.WIS =  50;
	user.status.DEX = 100;
	user.status.AGL = 100;
	user.status.CHR = 100;
	user.status.MGR =  95;
	user.status.KEY = 200;
	user.status.ELX = 100;
	user.status.CRN =   4;
	user.status.KRM =   0;

	user.equipment[GOODS_WEAPON] = 15;
	user.equipment[GOODS_SCROLL] = 15;
	user.equipment[GOODS_ARMOUR] = 16;
	user.equipment[GOODS_SHIELD] = 16;

	user.status.fighter.rank = 15;
	user.status.wizard .rank = 15;

	user.environment.lighting = 255;
	user.environment.in_training_ground = 0;

	user.point = 0;
	user.x = 0;
	user.y = 0;

	match_user_name("Debugger");
}

int MenuScene::initLoadMenu()
{
	int n = 0;
    // USERS_DIR内のファイルを列挙し、そのファイル名をuserEntries_に格納する。最大でkMaxUserEntry個まで。
    if (!fs::exists(USERS_DIR) || !fs::is_directory(USERS_DIR)) {
        ::SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Directory %s does not exist or is not a directory", USERS_DIR);
        return 0;
    }
    for (const auto &entry : fs::directory_iterator(USERS_DIR)) {
        if (!entry.is_directory()) {
            continue; // ディレクトリ以外はスキップ
        }
        const auto filename = entry.path().filename().string();
        if (filename != "." && filename != "..") {
            userEntries_[n] = filename;
            n++;
            if (n >= kMaxUserEntry) {
                break;
            }
        }
    }

	return n;
}

int MenuScene::loadGame(int index)
{
	if (0 <= index && index < kMaxUserEntry && !userEntries_[index].empty()) {
		user_path = std::string(USERS_DIR) + "/" + userEntries_[index];

		if (!loadUser()) {
			// 失敗
			emit_error("Can't load user.dat!");
			user_path.clear();
			return 1;
		}
		return 0; // 成功
	}
	return 1;
}

bool MenuScene::loadUser()
{
	if (user_path.empty()) {
		return false;
	}

	std::ifstream ifs(user_path + "/user.dat", std::ios::binary);
	if (!ifs) {
		return false;
	}

	ifs.read(reinterpret_cast<char *>(&user), sizeof(user));
	return static_cast<bool>(ifs);
}
