#ifndef keyboard_H
#define keyboard_H

#include "app/Application.h"
#include <SDL3/SDL_scancode.h>
#include <cctype>

inline bool isKeyDown(SDL_Scancode scancode)
{
	return Application::instance().getKeybordState()[scancode];
}

inline bool isCtrlDown()
{
	return isKeyDown(SDL_SCANCODE_LCTRL) || isKeyDown(SDL_SCANCODE_RCTRL);
}

inline bool isShiftDown()
{
	return isKeyDown(SDL_SCANCODE_LSHIFT) || isKeyDown(SDL_SCANCODE_RSHIFT);
}

inline bool isReturnDown()
{
	return isKeyDown(SDL_SCANCODE_RETURN) || isKeyDown(SDL_SCANCODE_KP_ENTER);
}

// message.cpp の「直前に押された文字」を pause_clearkey に渡すためだけの変換。
// 英数字/CR/SPACE以外は SDL_SCANCODE_UNKNOWN (常にfalse) を返す。
inline SDL_Scancode scancodeFromChar(int c)
{
	if (c >= 'A' && c <= 'Z') return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (c - 'A'));
	if (c >= 'a' && c <= 'z') return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (c - 'a'));
	if (c >= '1' && c <= '9') return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (c - '1'));
	if (c == '0') return SDL_SCANCODE_0;
	if (c == '\r' || c == '\n') return SDL_SCANCODE_RETURN;
	if (c == ' ') return SDL_SCANCODE_SPACE;
	return SDL_SCANCODE_UNKNOWN;
}

#endif // keyboard_H
