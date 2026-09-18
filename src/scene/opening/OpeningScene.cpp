#include "scene/opening/OpeningScene.h"
#include "xanadu.h"
#include "fade.h"
#include "sdl/LegacyPlatform.h"

#include <SDL3/SDL_events.h>
#include <cstdio>

namespace {

struct Visual
{
	const char *image;
	int x;
	int y;
	int se;
};

const Visual kVisuals1[] = {
	{ "xa1/opening/background.bmp",  0,   0, SE_SOMEWHAT1 },
	{ "xa1/opening/battler.bmp",   360, 144, SE_SOMEWHAT1 },
	{ "xa1/opening/witch.bmp",     144, 144, SE_SOMEWHAT1 },
	{ "xa1/opening/wizard.bmp",    464, 152, SE_SOMEWHAT1 },
	{ "xa1/opening/robber.bmp",     16, 168, SE_SOMEWHAT1 },
	{ "xa1/opening/swordman.bmp",  232, 144, SE_SOMEWHAT2 },
	{ nullptr, 0, 0, 0 }
};

const Visual kVisuals2[] = {
	{ "xa2/opening/title.bmp",      24,  24, SE_SOMEWHAT1 },
	{ "xa2/opening/subtitle.bmp",  112, 248, SE_SOMEWHAT1 },
	{ "xa2/opening/hero.bmp",      368,  16, SE_SOMEWHAT1 },
	{ nullptr, 0, 0, 0 }
};

} // namespace

OpeningScene::OpeningScene()
	: step_(0)
	, fadeMask_(0)
	, waitingForKey_(false)
{
}

void OpeningScene::onCreate(uint32_t /*tick*/)
{
	step_ = 0;
	fadeMask_ = 0;
	waitingForKey_ = false;

	// clip_main等(メニュー画面の文字列など)がclip_overallへ合成されると、
	// このシーンが画面全体に直接描く演出の上に被さってしまうため止めておく。
	// resume_context()で本当にこのシーンが終わる時(onSuspend())に戻す。
	setLegacyPanelCompositingEnabled(false);

	// BGM mute
	bgm_play("");

	// 効果音
	se_load(SE_SOMEWHAT1, se_data.opening0);
	se_load(SE_SOMEWHAT2, se_data.opening1);

	visual_image = nullptr;

	fill_image(clip_overall, 0, 0, clip_overall->getWidth(), clip_overall->getHeight(),
	           SDL_::Color::BLACK);
	update(rect_overall);
	update_immediately();
}

void OpeningScene::onResume(uint32_t /*tick*/)
{
	onEnter();
}

void OpeningScene::onSuspend()
{
	// FadeSceneへ一時的に処理を譲るだけ(演出がまだ続く)ならfalseのままにし、
	// このシーン自体が本当に終わる(isFinished())時にだけ合成を元へ戻す
	if (isFinished()) {
		setLegacyPanelCompositingEnabled(true);
	}
}

void OpeningScene::dispatch(const SDL_Event &event)
{
	if (!waitingForKey_) {
		return;
	}
	if (event.type == SDL_EVENT_KEY_DOWN && event.key.repeat == 0) {
		restoreContext();
	}
}

void OpeningScene::onEnter()
{
	if (get_keystate(VK_SPACE) || get_keystate(VK_RETURN)) {
		restoreContext();
		return;
	}

	const Visual *visuals = user.environment.scenario == 0 ? kVisuals1 : kVisuals2;

	if (visuals[step_].image == nullptr) {
		waitForever();
		return;
	}

	const int x = visuals[step_].x;
	const int y = visuals[step_].y;
	switch (fadeMask_) {
	case 0x000000:
		{
			char path[BUFSIZ];
			sprintf(path, IMAGE_DIR "/%s", visuals[step_].image);

			visual_image = load_image(path);
		}
		se_play(visuals[step_].se);
		// through

	case 0x0000FF:
		extend_context(init_fade(clip_overall, x, y, visual_image, fadeMask_));
		fadeMask_ = 0xFF00FF;
		break;

	case 0xFF00FF:
		extend_context(init_fade(clip_overall, x, y, visual_image, fadeMask_));
		fadeMask_ = 0xFFFFFF;
		break;

	case 0xFFFFFF:
		extend_context(init_fade(clip_overall, x, y, visual_image, fadeMask_));
		step_++;
		fadeMask_ = 0x000000;
		break;

	default:
		restoreContext();
		break;
	}
}

void OpeningScene::waitForever()
{
	waitingForKey_ = true;
	bgm_play(bgm_data.theme[user.environment.scenario].opening);
}

void OpeningScene::restoreContext()
{
	load_background(IMAGE_DIR "/user/frame.bmp");
	update(rect_overall);
	resume_context();
}
