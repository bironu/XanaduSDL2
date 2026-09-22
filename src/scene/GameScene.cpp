#include "scene/GameScene.h"
#include "message.h"
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
    presentLegacyFrame();
    return Scene::onIdle(tick);
}

void GameScene::dispatch(const SDL_Event &event)
{
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
