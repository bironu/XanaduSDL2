#include "app/Application.h"
#include "scene/opening/OpeningScene.h"
#include "resources/Resources.h"
#include "resources/ImageId.h"
#include "resources/SoundId.h"
#include "resources/MusicId.h"
#include "xanadu.h"
#include "fade.h"
// #include "sdl/LegacyPlatform.h"
#include "sdl/SDLWindow.h"
#include "sdl/SDLImage.h"
#include "sdl/SDLMixMixer.h"

#include <SDL3/SDL_events.h>
#include <array>

const FRect OpeningScene::rect_overall_    = {   0.0f,   0.0f, 640.0f, 400.0f };
const uint32_t OpeningScene::SDL_OPENING_TIMER_EVENT = ::SDL_RegisterEvents(1);

namespace {

struct Visual
{
	ImageId imageId;
	int x;
	int y;
	SoundId se;
};

const std::array<Visual, 6> kVisuals1 = {{
	{ ImageId::xa1_opening_background,  0,   0, SoundId::opening0 },
	{ ImageId::xa1_opening_battler,   360, 144, SoundId::opening0 },
	{ ImageId::xa1_opening_witch,     144, 144, SoundId::opening0 },
	{ ImageId::xa1_opening_wizard,    464, 152, SoundId::opening0 },
	{ ImageId::xa1_opening_robber,     16, 168, SoundId::opening0 },
	{ ImageId::xa1_opening_swordman,  232, 144, SoundId::opening1 },
}};

const std::array<Visual, 3> kVisuals2 = {{
	{ ImageId::xa2_opening_title,      24,  24, SoundId::opening0 },
	{ ImageId::xa2_opening_subtitle,  112, 248, SoundId::opening0 },
	{ ImageId::xa2_opening_hero,      368,  16, SoundId::opening0 },
}};

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

    auto &app = getApplication();
    auto &mixer = app.getMixer();
    auto &res = getResources();

	// BGM mute
    mixer.stopMusic();

	// 効果音
    res.loadSound(mixer, SoundId::opening0);
    res.loadSound(mixer, SoundId::opening1);

	res.loadImage(ImageId::xa1_opening_background);
    res.loadImage(ImageId::xa1_opening_battler);
    res.loadImage(ImageId::xa1_opening_witch);
    res.loadImage(ImageId::xa1_opening_wizard);
    res.loadImage(ImageId::xa1_opening_robber);
    res.loadImage(ImageId::xa1_opening_swordman);
    res.loadImage(ImageId::xa2_opening_title);
    res.loadImage(ImageId::xa2_opening_subtitle);
    res.loadImage(ImageId::xa2_opening_hero);

	visual_image_.reset();
    clip_overall_ = std::make_shared<SDL_::Image>(rect_overall_.getWidth(), rect_overall_.getHeight());

    clip_overall_->fillRect(SDL_::Color::BLACK);
    auto mainWindow = app.getMainWindow();
    if (mainWindow) {
        mainWindow->requestUpdate();
    }
    // kTimerInterval間隔で onEnter() を呼び出すタイマーをセットする。
    app.setTimer(kTimerInterval, [this](Uint32 interval) -> Uint32 {
        SDL_Event event = {SDL_OPENING_TIMER_EVENT};
        ::SDL_PushEvent(&event);
        return interval;
    });
}

void OpeningScene::onResume(uint32_t /*tick*/)
{
}

void OpeningScene::onSuspend()
{
}

void OpeningScene::onDestroy(uint32_t /*tick*/)
{
    auto &app = getApplication();
    auto &mixer = app.getMixer();
    auto &res = getResources();
    app.killTimer();

	// BGM mute
    mixer.stopMusic();

	// 効果音
    res.unloadSound(SoundId::opening0);
    res.unloadSound(SoundId::opening1);

	res.unloadImage(ImageId::xa1_opening_background);
    res.unloadImage(ImageId::xa1_opening_battler);
    res.unloadImage(ImageId::xa1_opening_witch);
    res.unloadImage(ImageId::xa1_opening_wizard);
    res.unloadImage(ImageId::xa1_opening_robber);
    res.unloadImage(ImageId::xa1_opening_swordman);
    res.unloadImage(ImageId::xa2_opening_title);
    res.unloadImage(ImageId::xa2_opening_subtitle);
    res.unloadImage(ImageId::xa2_opening_hero);
}

bool OpeningScene::onIdle(uint32_t tick)
{
    return Scene::onIdle(tick);
}

void OpeningScene::dispatch(const SDL_Event &event)
{
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
        onKeyDown(event.key);
        break;

    case SDL_EVENT_WINDOW_EXPOSED:
        onWindowExpose(event.window);
        break;

    default:
        if (event.type == SDL_OPENING_TIMER_EVENT) {
            onTimer();
        }
        break;
    }
}

void OpeningScene::onKeyDown(const SDL_KeyboardEvent &key)
{
	if (!waitingForKey_) {
		return;
	}
	if (key.repeat == 0) {
		restoreContext();
	}
}

void OpeningScene::onWindowExpose(const SDL_WindowEvent &window)
{
    onDraw();
    SDL_Log("Window exposed event: windowID=%u, data1=%d, data2=%d", window.windowID, window.data1, window.data2);
}

void OpeningScene::onTimer()
{
    SDL_Log("OpeningScene::onTimer: call ");
    uint32_t interval = onEnter();
    onDraw();
    if (interval == 0) {
        auto &app = getApplication();
        app.killTimer();
    }
}

void OpeningScene::onDraw()
{
    auto &app = getApplication();
    auto mainWindow = app.getMainWindow();
    auto backBuffer = mainWindow->getBackBuffer();
    backBuffer->blitScaled(clip_overall_, nullptr, nullptr, SDL_SCALEMODE_PIXELART);
    mainWindow->swap();
}

uint32_t OpeningScene::onEnter()
{
	const Visual *visuals = user.environment.scenario == 0 ? kVisuals1.data() : kVisuals2.data();
    const int numVisuals = user.environment.scenario == 0 ? kVisuals1.size() : kVisuals2.size();

    if (step_ >= numVisuals) {
		waitForever();
		return 0;
	}

	// 実行中のフェードがあれば1ステップぶんだけ進める
	if (!fade_.isDone()) {
		fade_.step();
		if (fade_.isDone()) {
			advanceFade();
		}
		return kTimerInterval;
	}

	// この絵の最初のフェード(黒地からの実写化)を開始する
    auto &app = getApplication();
    auto &mixer = app.getMixer();
    auto &res = getResources();
	const int x = visuals[step_].x;
	const int y = visuals[step_].y;

	visual_image_ = res.getImage(visuals[step_].imageId);
	mixer.playSound(*res.getSound(visuals[step_].se), -1, 0);

	fadeMask_ = 0x000000;
	fade_.start(clip_overall_, x, y, visual_image_, fadeMask_);

    return kTimerInterval;
}

void OpeningScene::advanceFade()
{
	const Visual *visuals = user.environment.scenario == 0 ? kVisuals1.data() : kVisuals2.data();
	const int x = visuals[step_].x;
	const int y = visuals[step_].y;

	switch (fadeMask_) {
	case 0x000000:
		fadeMask_ = 0xFF00FF;
		fade_.start(clip_overall_, x, y, visual_image_, fadeMask_);
		break;

	case 0xFF00FF:
		fadeMask_ = 0xFFFFFF;
		fade_.start(clip_overall_, x, y, visual_image_, fadeMask_);
		break;

	case 0xFFFFFF:
		step_++;
		fadeMask_ = 0x000000;
		break;
	}
}

void OpeningScene::waitForever()
{
	waitingForKey_ = true;
	// bgm_play(bgm_data.theme[user.environment.scenario].opening);
}

void OpeningScene::restoreContext()
{
	// load_background(IMAGE_DIR "/user/frame.bmp");
	// update(rect_overall);
    waitingForKey_ = false;
	resume_context();
}
