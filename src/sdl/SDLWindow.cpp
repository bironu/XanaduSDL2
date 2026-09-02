#include "sdl/SDLWindow.h"
#include "app/Application.h"
#include <SDL2/SDL_video.h>

namespace SDL_
{

Window::Window(const char* title, int x, int y, int w, int h, Uint32 flags)
	: window_(::SDL_CreateWindow(title, x, y, w, h, flags))
	, renderer_(*this, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC)
{
}

Window::~Window()
{
	if(isWindow()){
		::SDL_DestroyWindow(window_);
	}
}

void Window::swap()
{
	renderer_.present();
}

} // SDL_
