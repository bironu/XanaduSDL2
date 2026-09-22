#include "sdl/LegacyPlatform.h"
#include "xanadu.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/SoundId.h"
#include "resources/MusicId.h"
#include "sdl/SDLBitmapFont.h"
#include "sdl/SDLTexture.h"
#include "sdl/SDLRenderer.h"
#include "sdl/SDLWindow.h"
#include "sdl/SDLMixAudio.h"
#include "sdl/SDLMixMixer.h"
#include "resources/SoundFontId.h"
#include "app/Application.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>
#include <memory>
#include <string>

// ---- 画面バッファ ----
// 旧main.c(Windows)/x11/main.cではOS側のウィンドウ用DIB/XImageを確保し、
// clip_overallをその上のimage_tとして構築していた。SDL3版ではclip_overallと
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

const rectangle_t rect_overall    = {   0,   0, 640, 400 };
const rectangle_t rect_main       = {  16,  16, 360, 360 };
const rectangle_t rect_message    = { 392, 304, 240,  80 };
const rectangle_t rect_status     = { 392,  16, 240, 272 };
const rectangle_t rect_shrine     = {  16,  96, 608, 240 };
const rectangle_t rect_user_guage = {  56,  32, 240,  40 };
const rectangle_t rect_boss_guage = { 384,  32, 240,  40 };

// ---- スプライト/フォント等のリソース ----
// フレームテーブルは各ロード関数(load_user_image, load_tile_image等)が
// 必要になった時点で埋める。frame_magics/frame_goods/frame_brownbox/
// frame_whitebox/frame_specials/mask_damaged/pattern_guage/pattern_status は
// 対応するローダが未移植のため、現状は未ロード(nullptr)のまま。

SDL_::SubImage frame_user[10];
SDL_::SubImage frame_monsters[N_MONSTERS][4];
std::shared_ptr<SDL_::Image> frame_magics[N_MAGICS * 2];
SDL_::SubImage frame_tiles[N_TILES];
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

// ---- 旧C実装のset_timer/kill_timer(WM_TIMER相当)のSDL3版 ----
// SDL_AddTimer()のコールバックは専用スレッドで呼ばれるため、そこから直接
// timer_proc(ゲーム状態やSDL_Rendererを触る)を呼ぶのは安全ではない。
// そのため、コールバックはSDL_RegisterEvents()で確保した専用のSDL_Eventを
// SDL_PushEvent()で積むだけに留め、実際のtimer_proc呼び出しはメインスレッドの
// イベントループ(Application::handlePreEvent -> dispatchLegacyTimerEvent)側で行う。
//
// generation_はset_timer/kill_timerのたびに変化する世代番号。SDL_RemoveTimer()
// が間に合わず、既にキューへ積まれてしまった古いタイマーのイベントが後から
// 処理されても、生成時のgenerationと現在のgenerationが一致しなければ無視する。
Uint32 legacyTimerEventType()
{
	static const Uint32 type = SDL_RegisterEvents(1);
	return type;
}

void (*legacyTimerProc)(void) = nullptr;
SDL_TimerID legacyTimerId = 0;
Sint32 legacyTimerGeneration = 0;

Uint32 SDLCALL legacyTimerCallback(void *userdata, SDL_TimerID /*timerID*/, Uint32 interval)
{
	SDL_Event event{};
	event.type = legacyTimerEventType();
	event.user.code = static_cast<Sint32>(reinterpret_cast<intptr_t>(userdata));
	SDL_PushEvent(&event);
	return interval; // 同じ間隔で繰り返す(one-shotにはしない)
}
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

	auto &renderer = Application::instance().getMainWindow()->getRenderer();
	auto texture = std::make_shared<SDL_::Texture>(renderer, *clip_overall);
	renderer.clear();
	renderer.copy(texture, nullptr, nullptr);
	renderer.present();
}

// ---- 描画・タイマー・音声のプラットフォームフック ----

int draw_text(std::shared_ptr<SDL_::Image> dst, int x, int y, const char *s, const SDL_::Color &color)
{
	if (dst && legacyFont) {
		legacyFont->drawText(dst, x, y, s, color);
	}
	return static_cast<int>(strlen(s));
}

void update_region(int, int, int, int)
{
    update_immediately();
}

void update_immediately(void)
{
    presentLegacyFrame();
}

int load_background(ImageId id)
{
	Resources::instance().loadImage(id);
	auto img = Resources::instance().getImage(id);
	if (!img) {
		return 1;
	}
	draw_image(clip_overall, 0, 0, img);
	update(rect_overall);
	return 0;
}

void set_timer(int interval, void (*timer_proc)(void))
{
	kill_timer();
	legacyTimerProc = timer_proc;
	legacyTimerId = SDL_AddTimer(static_cast<Uint32>(interval), legacyTimerCallback,
	                              reinterpret_cast<void *>(static_cast<intptr_t>(legacyTimerGeneration)));
}

void kill_timer(void)
{
	if (legacyTimerId) {
		SDL_RemoveTimer(legacyTimerId);
		legacyTimerId = 0;
	}
	++legacyTimerGeneration; // 積まれた古いイベントを無効化する
	legacyTimerProc = nullptr;
}

void set_timer_proc(void (*timer_proc)(void))
{
	// 動作中のタイマーはそのままに、次に発火した際に呼ぶ関数だけ差し替える
	// (battle.cpp等が同じ間隔のまま処理内容だけ切り替えるのに使う)
	legacyTimerProc = timer_proc;
}

bool dispatchLegacyTimerEvent(const SDL_Event &event)
{
	if (event.type != legacyTimerEventType()) {
		return false;
	}
	if (event.user.code == legacyTimerGeneration && legacyTimerProc) {
		legacyTimerProc();
	}
	return true;
}

void beep(void)
{
}

int bgm_enabled(void)
{
	return !user.config.mute;
}

int se_enabled(void)
{
	return !user.config.mute;
}

void bgm_tempo(int)
{
	// SDL3_mixerのMIX_Track APIにはMIDIテンポを変更する機能が見当たらないため未対応
}

void bgm_random(int)
{
	// SDL3_mixerのMIX_Track APIにはMIDIのピッチをランダム化する機能が見当たらないため未対応
}

int bgm_mute(void)
{
	user.config.mute = !user.config.mute;
	auto &mixer = Application::instance().getMixer();
	if (user.config.mute) {
		mixer.pauseMusic();
	}
	else {
		mixer.resumeMusic();
	}
	return user.config.mute;
}

void playSound(SoundId id)
{
	if (user.config.mute) {
		return;
	}
	if (auto chunk = Resources::instance().getSound(id)) {
		Application::instance().getMixer().playSound(*chunk, -1, 0);
	}
}

void playBgm(MusicId id)
{
	Resources::instance().playBgm(Application::instance().getMixer(), id);
}
