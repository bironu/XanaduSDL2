#include "sdl/LegacyPlatform.h"
#include "xanadu.h"
#include "audio.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "sdl/SDLBitmapFont.h"
#include "sdl/SDLTexture.h"
#include "sdl/SDLRenderer.h"
#include "sdl/SDLWindow.h"
#include "app/Application.h"
#include <memory>

// ---- 画面バッファ ----
// 旧main.c(Windows)/x11/main.cではOS側のウィンドウ用DIB/XImageを確保し、
// clip_overallをその上のimage_tとして構築していた。SDL2版ではclip_overallと
// clip_main等の各領域は、いずれも独立したSDL_::Imageとして確保する
// (Imageはコピー不可のため、旧subsection_image()のような「親バッファの
//  一部を指すビュー」は作れない)。各領域はpresentLegacyFrame()で
// rect_*の位置に合わせてclip_overallへ合成してから画面に転送する。

std::shared_ptr<SDL_::Image> clip_overall;
std::shared_ptr<SDL_::Image> clip_main;
std::shared_ptr<SDL_::Image> clip_message;
std::shared_ptr<SDL_::Image> clip_status;
std::shared_ptr<SDL_::Image> clip_shrine;
std::shared_ptr<SDL_::Image> clip_user_guage;
std::shared_ptr<SDL_::Image> clip_boss_guage;
std::shared_ptr<SDL_::Image> clip_endingroll;

const rectangle_t rect_overall    = {   0,   0, 640, 400 };
const rectangle_t rect_main       = {  16,  16, 360, 360 };
const rectangle_t rect_message    = { 392, 304, 240,  80 };
const rectangle_t rect_status     = { 392,  16, 240, 272 };
const rectangle_t rect_shrine     = {  16,  96, 608, 240 };
const rectangle_t rect_user_guage = {  56,  32, 240,  40 };
const rectangle_t rect_boss_guage = { 384,  32, 240,  40 };
const rectangle_t rect_endingroll = {  80,  96, 560, 240 };

// ---- スプライト/フォント等のリソース ----
// フレームテーブルは各ロード関数(load_user_image, load_tile_image等)が
// 必要になった時点で埋める。frame_magics/frame_goods/frame_brownbox/
// frame_whitebox/frame_specials/mask_damaged/pattern_guage/pattern_status は
// 対応するローダが未移植のため、現状は未ロード(nullptr)のまま。

std::shared_ptr<SDL_::Image> frame_user[10];
std::shared_ptr<SDL_::Image> frame_monsters[N_MONSTERS][4];
std::shared_ptr<SDL_::Image> frame_magics[N_MAGICS * 2];
std::shared_ptr<SDL_::Image> frame_tiles[N_TILES];
std::shared_ptr<SDL_::Image> frame_goods[N_GOODS];
std::shared_ptr<SDL_::Image> frame_brownbox[4];
std::shared_ptr<SDL_::Image> frame_whitebox[4];
std::shared_ptr<SDL_::Image> frame_specials[N_SPECIALS];
std::shared_ptr<SDL_::Image> mask_damaged;
std::shared_ptr<SDL_::Image> pattern_guage;
std::shared_ptr<SDL_::Image> pattern_status;
std::shared_ptr<SDL_::Image> visual_image;

namespace {
std::unique_ptr<SDL_::BitmapFont> legacyFont;
}

void initLegacyGraphics(Resources &res)
{
	clip_overall     = create_image(rect_overall.width, rect_overall.height);
	clip_main        = create_image(rect_main.width, rect_main.height);
	clip_message     = create_image(rect_message.width, rect_message.height);
	clip_status      = create_image(rect_status.width, rect_status.height);
	clip_shrine      = create_image(rect_shrine.width, rect_shrine.height);
	clip_user_guage  = create_image(rect_user_guage.width, rect_user_guage.height);
	clip_boss_guage  = create_image(rect_boss_guage.width, rect_boss_guage.height);
	clip_endingroll  = create_image(rect_endingroll.width, rect_endingroll.height);

	auto fontAtlas = res.getImage(ImageId::user_font);
	if (fontAtlas) {
		legacyFont = std::make_unique<SDL_::BitmapFont>(*fontAtlas);
	}
}

void presentLegacyFrame()
{
	if (!clip_overall) {
		return;
	}

	draw_image(clip_overall, rect_main.x,       rect_main.y,       clip_main);
	draw_image(clip_overall, rect_message.x,    rect_message.y,    clip_message);
	draw_image(clip_overall, rect_status.x,     rect_status.y,     clip_status);
	draw_image(clip_overall, rect_shrine.x,     rect_shrine.y,     clip_shrine);
	draw_image(clip_overall, rect_user_guage.x, rect_user_guage.y, clip_user_guage);
	draw_image(clip_overall, rect_boss_guage.x, rect_boss_guage.y, clip_boss_guage);
	draw_image(clip_overall, rect_endingroll.x, rect_endingroll.y, clip_endingroll);

	auto &renderer = Application::instance().getMainWindow()->getRenderer();
	auto texture = std::make_shared<SDL_::Texture>(renderer, *clip_overall);
	renderer.clear();
	renderer.copy(texture, nullptr, nullptr);
}

// ---- 描画・タイマー・音声のプラットフォームフック ----

int draw_text(std::shared_ptr<SDL_::Image> dst, int x, int y, const char *s, const SDL_::Color &color)
{
	if (dst && legacyFont) {
		auto text = legacyFont->renderSolidText(s, color);
		if (text) {
			draw_sprite(dst, x, y, text);
		}
	}
	return static_cast<int>(strlen(s));
}

void update_region(int, int, int, int)
{
}

void update_immediately(void)
{
}

int load_background(const char *filename)
{
	auto img = load_image(filename);
	if (!img) {
		return 1;
	}
	draw_image(clip_overall, 0, 0, img);
	update(rect_overall);
	return 0;
}

// タイマー・音声はまだSDL2側の実装がなく、何もしない
// (SDL_AddTimer/Mix_*等を使ったオーディオ統合は今後の課題)

void set_timer(int, void (*)(void))
{
}

void kill_timer(void)
{
}

void set_timer_proc(void (*)(void))
{
}

void beep(void)
{
}

int bgm_enabled(void)
{
	return 0;
}

int se_enabled(void)
{
	return 0;
}

void bgm_play(const char *)
{
}

void bgm_stop(void)
{
}

void bgm_pause(void)
{
}

void bgm_restart(void)
{
}

void bgm_tempo(int)
{
}

void bgm_random(int)
{
}

int bgm_mute(void)
{
	user.config.mute = !user.config.mute;
	return user.config.mute;
}

void se_play(int)
{
}

void se_load(int, const char *)
{
}
