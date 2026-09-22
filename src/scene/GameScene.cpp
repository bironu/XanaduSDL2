#include "scene/GameScene.h"
#include "message.h"
#include "pause.h"
#include "sdl/LegacyPlatform.h"
#include <SDL3/SDL_events.h>
#include <cctype>

namespace {

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
    pause_update(tick);
    presentLegacyFrame();

    // ポーズ中はここでtrueを返し続けてonIdleのポーリングを続けさせる。
    // Scene::onIdle()はタスク未登録時false(=Application::run()が
    // SDL_WaitEventでブロックする)を返すため、falseのままだと誰も
    // イベントを起こさない待機(SE再生終了待ち等)では、そのイベントが
    // 来るまでpause_update()自体が二度と呼ばれず、ポーズが永遠に
    // 解除されなくなる
    if (pause_active()) {
        return true;
    }

    return Scene::onIdle(tick);
}

void GameScene::dispatch(const SDL_Event &event)
{
	// ポーズ中(XanaduPause)は元のコンテキストへの入力を止める
	// (旧PauseSceneがSceneスタックの最上段で入力を奪っていたのと同じ役割)
	if (pause_active()) {
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
