#ifndef SDLTEXTURE_H_
#define SDLTEXTURE_H_

#include "misc/Uncopyable.h"
#include "SDLColor.h"
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_render.h>

struct SDL_Texture;

namespace SDL_
{

class Renderer;
class Image;
class Texture
{
public:
	UNCOPYABLE(Texture);
	Texture(const Renderer &renderer, const Image &surface);
	Texture(const Renderer &renderer, Uint32 format, int access, int w, int h);
	~Texture();

	int getWidth() const { return width_; }
	int getHeight() const { return height_; }

	SDL_Texture *get() const { return texture_; }

	//int SDL_GL_BindTexture(SDL_Texture *texture, float *texw, float *texh);
	//int SDL_GL_UnbindTexture(SDL_Texture *texture);
	SDL_BlendMode getBlendMode()
	{
		SDL_BlendMode blendMode;
		SDL_GetTextureBlendMode(get(), &blendMode);
		return blendMode;
	}
	const Color getColor()
	{
		Uint8 r, g, b, a;
		::SDL_GetTextureColorMod(get(), &r, &g, &b);
		::SDL_GetTextureAlphaMod(get(), &a);
		return Color(r, g, b, a);
	}
	//int SDL_LockTexture(SDL_Texture* texture, const SDL_Rect* rect, void** pixels, int* pitch);
	//int SDL_QueryTexture(SDL_Texture* texture, Uint32* format, int* access, int* w, int* h);
	void setAlphaMod(Uint8 alpha) { ::SDL_SetTextureAlphaMod(get(), alpha); }
	void setBlendMode(SDL_BlendMode blendMode) { ::SDL_SetTextureBlendMode(get(), blendMode); }
	void setColorMod(Uint8 r, Uint8 g, Uint8 b) { ::SDL_SetTextureColorMod(get(), r, g, b); }
	//void SDL_UnlockTexture(SDL_Texture* texture);
	//int SDL_UpdateTexture(SDL_Texture* texture, const SDL_Rect* rect, const void* pixels, int pitch);
	//int SDL_UpdateYUVTexture(SDL_Texture* texture, const SDL_Rect* rect, const Uint8* Yplane, int Ypitch, const Uint8* Uplane, int Upitch, const Uint8* Vplane, int Vpitch);

private:
	SDL_Texture * const texture_;
	const int width_;
	const int height_;
};

} // SDL

#endif // SDLTEXTURE_H_
