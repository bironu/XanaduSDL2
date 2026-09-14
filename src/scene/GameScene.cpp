#include "scene/GameScene.h"
#include "message.h"
#include "keystate.h"
#include <SDL2/SDL_events.h>
#include <cctype>

// 旧 src/x11/main.c にあった keystate_vector の実体。
// SDL2版フロントエンドではここが唯一の定義元になる。
int keystate_vector[256];

namespace {

// SDL_Keycode を get_keystate() が使う添字(keystate.h の非Windows側の定義)に変換する。
// US配列を前提としたベストエフォート実装であり、レイアウト依存の変換はまだ行っていない。
int toKeystateIndex(SDL_Keycode sym)
{
	switch (sym) {
	case SDLK_LSHIFT: case SDLK_RSHIFT: return VK_SHIFT;
	case SDLK_LCTRL:  case SDLK_RCTRL:  return VK_CONTROL;
	case SDLK_RETURN: case SDLK_KP_ENTER: return VK_RETURN;
	case SDLK_ESCAPE:   return VK_ESCAPE;
	case SDLK_HOME:     return VK_HOME;
	case SDLK_LEFT:     return VK_LEFT;
	case SDLK_UP:       return VK_UP;
	case SDLK_RIGHT:    return VK_RIGHT;
	case SDLK_DOWN:     return VK_DOWN;
	case SDLK_PAGEUP:   return VK_PRIOR;
	case SDLK_PAGEDOWN: return VK_NEXT;
	case SDLK_END:      return VK_END;
	case SDLK_SPACE:    return VK_SPACE;
	default:
		if (sym >= SDLK_a && sym <= SDLK_z) {
			return std::toupper(sym);
		}
		if (sym >= SDLK_0 && sym <= SDLK_9) {
			return sym;
		}
		return -1;
	}
}

// thunk_key_event に渡す文字コードへの変換(Shift状態を反映したUS配列相当の文字)。
int toCharCode(const SDL_Keysym &keysym)
{
	const bool shift = (keysym.mod & KMOD_SHIFT) != 0;

	switch (keysym.sym) {
	case SDLK_RETURN: case SDLK_KP_ENTER: return '\r';
	case SDLK_BACKSPACE:                  return '\b';
	case SDLK_ESCAPE:                     return 27;
	default: break;
	}

	if (keysym.sym >= SDLK_a && keysym.sym <= SDLK_z) {
		return shift ? std::toupper(keysym.sym) : keysym.sym;
	}

	if (keysym.sym >= SDLK_0 && keysym.sym <= SDLK_9) {
		if (shift) {
			static const char shifted[] = ")!@#$%^&*(";
			return shifted[keysym.sym - SDLK_0];
		}
		return keysym.sym;
	}

	if (keysym.sym >= 0x20 && keysym.sym <= 0x7e) {
		return keysym.sym;
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

void GameScene::dispatch(const SDL_Event &event)
{
	switch (event.type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		{
			const int index = toKeystateIndex(event.key.keysym.sym);
			if (index >= 0 && index < 256) {
				keystate_vector[index] = (event.type == SDL_KEYDOWN);
			}
		}
		if (event.type == SDL_KEYDOWN && event.key.repeat == 0 && thunk_key_event) {
			const int c = toCharCode(event.key.keysym);
			if (c != 0) {
				(*thunk_key_event)(c);
			}
		}
		break;

	default:
		break;
	}
}
