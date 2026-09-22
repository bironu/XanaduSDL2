#include "scene/GameScene.h"
#include "app/Application.h"
#include "keyboard.h"
#include "message.h"
#include "sdl/LegacyPlatform.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_timer.h>
#include <cctype>

namespace {

// GameSceneのタイマー機構が使う専用イベント型。起動後に一度だけ確保される。
// 全GameScene派生クラスで共有するが、実際にアクティブなSceneは常に高々
// 1つ(Application::timer_も単一)なので、種別を分ける必要はない
Uint32 gameTimerEventType()
{
	static const Uint32 type = SDL_RegisterEvents(1);
	return type;
}

constexpr uint32_t kPauseMaxWaitMs = 500; // 旧WAIT_INTERVAL(pauseForのタイムアウト)

// thunk_key_event に渡す文字コードへの変換(Shift状態を反映したUS配列相当の文字)。
int toCharCode(const SDL_KeyboardEvent &key)
{
	const bool shift = (key.mod & SDL_KMOD_SHIFT) != 0;

	switch (key.key) {
	case SDLK_RETURN: case SDLK_KP_ENTER: return '\r';
	case SDLK_BACKSPACE:                  return '\b';
	case SDLK_ESCAPE:                     return 27;
	default: break;
	}

	if (key.key >= SDLK_A && key.key <= SDLK_Z) {
		return shift ? std::toupper(key.key) : key.key;
	}

	if (key.key >= SDLK_0 && key.key <= SDLK_9) {
		if (shift) {
			static const char shifted[] = ")!@#$%^&*(";
			return shifted[key.key - SDLK_0];
		}
		return key.key;
	}

	if (key.key >= 0x20 && key.key <= 0x7e) {
		return key.key;
	}

	return 0;
}

} // namespace

GameScene::GameScene(int contextId)
	: contextId_(contextId)
{
}

void GameScene::onCreate(uint32_t /*tick*/)
{
}

void GameScene::onDestroy(uint32_t /*tick*/)
{
}

void GameScene::onResume(uint32_t /*tick*/)
{
	onEnter();
}

void GameScene::onSuspend()
{
	onLeave();
}

bool GameScene::onIdle(uint32_t tick)
{
    updateWait(tick);
    presentLegacyFrame();

    // 待機中はここでtrueを返し続けてonIdleのポーリングを続けさせる。
    // Scene::onIdle()はタスク未登録時false(=Application::run()が
    // SDL_WaitEventでブロックする)を返すため、falseのままだと誰も
    // イベントを起こさない待機(SE再生終了待ち等)では、そのイベントが
    // 来るまでupdateWait()自体が二度と呼ばれず、待機が永遠に
    // 解除されなくなる
    if (waiting_) {
        return true;
    }

    return Scene::onIdle(tick);
}

void GameScene::dispatch(const SDL_Event &event)
{
	// 待機中(GameScene::wait)は元のコンテキストへの入力を止める
	// (旧PauseSceneがSceneスタックの最上段で入力を奪っていたのと同じ役割)
	if (waiting_) {
		return;
	}

	if (event.type == gameTimerEventType()) {
		if (onTimer_) {
			onTimer_();
		}
		return;
	}

	switch (event.type) {
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:
		if (event.type == SDL_EVENT_KEY_DOWN && event.key.repeat == 0 && thunk_key_event) {
			const int c = toCharCode(event.key);
			if (c != 0) {
				(*thunk_key_event)(c);
			}
		}
		break;

	default:
		break;
	}
}

void GameScene::setGameTimer(int intervalMs, std::function<void()> onTimer)
{
	onTimer_ = std::move(onTimer);
	getApplication().setTimer(intervalMs, [](Uint32 interval) -> Uint32 {
		SDL_Event event{};
		event.type = gameTimerEventType();
		SDL_PushEvent(&event);
		return interval; // 同じ間隔で繰り返す(one-shotにはしない)
	});
}

void GameScene::killGameTimer()
{
	getApplication().killTimer();
	onTimer_ = nullptr;
}

void GameScene::wait(std::function<bool()> isDone, std::function<void()> onComplete, uint32_t maxWaitMs)
{
	killGameTimer(); // 呼び出し元の周期処理を止める。再開はonCompleteの責任

	if (maxWaitMs > 0) {
		const uint32_t deadline = SDL_GetTicks() + maxWaitMs;
		auto inner = isDone;
		isDone = [inner, deadline]() { return inner() || SDL_GetTicks() >= deadline; };
	}

	waiting_ = true;
	waitIsDone_ = std::move(isDone);
	waitOnComplete_ = std::move(onComplete);
}

void GameScene::pauseFor(int clearkey, std::function<void()> onComplete)
{
	const SDL_Scancode key = static_cast<SDL_Scancode>(clearkey);
	wait([key]() { return !isKeyDown(key); }, std::move(onComplete), kPauseMaxWaitMs);
}

void GameScene::updateWait(uint32_t /*tick*/)
{
	if (!waiting_ || !waitIsDone_ || !waitIsDone_()) {
		return;
	}
	waiting_ = false;
	waitIsDone_ = nullptr;
	auto onComplete = std::move(waitOnComplete_);
	waitOnComplete_ = nullptr;
	if (onComplete) {
		onComplete();
	}
}
