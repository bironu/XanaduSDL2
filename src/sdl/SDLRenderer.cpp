#include "sdl/SDLRenderer.h"
#include "geo/Rect.h"
#include "sdl/SDLImage.h"
#include "sdl/SDLTexture.h"
#include "sdl/SDLWindow.h"
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>

namespace SDL_
{

Renderer::Renderer(const Image &surface)
	: renderer_(SDL_CreateSoftwareRenderer(surface.get()))
	, clip_enable_(false)
{

}

Renderer::Renderer(const Window &window, Uint32 flags)
	: renderer_(::SDL_CreateRenderer(window.get(), -1, flags))
	, clip_enable_(false)
{

}

Renderer::~Renderer()
{
	if(renderer_){
		::SDL_DestroyRenderer(renderer_);
	}
}

void Renderer::copy(std::shared_ptr<Texture> texture, const Rect *srcrect, const Rect *dstrect)
{
	::SDL_RenderCopy(get(), texture->get(), srcrect, dstrect);
}

void Renderer::copyEx(std::shared_ptr<Texture> texture, const Rect *srcrect, const Rect *dstrect, const double angle, const Point *center, const SDL_RendererFlip flip)
{
	::SDL_RenderCopyEx(get(), texture->get(), srcrect, dstrect, angle, center, flip);
}

void Renderer::setTarget(std::shared_ptr<Texture> texture)
{
	::SDL_SetRenderTarget(get(), texture->get());
}

} // SDL_
