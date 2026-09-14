#if !defined(SDLIMAGE_H_)
#define SDLIMAGE_H_

#include "geo/Rect.h"
#include "geo/Point.h"
#include "geo/Vector2.h"
#include "sdl/SDLColor.h"
#include "misc/Uncopyable.h"
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_image.h>
#include <memory>
#include <cassert>

namespace SDL_
{

class Image
{
public:
	UNCOPYABLE(Image);
	explicit Image(SDL_Surface *surface)
		: surface_(surface)
	{
	}
	Image(int width, int height);
	explicit Image(const char * const file);
	~Image();

	SDL_Surface *get() const { return surface_;}

	operator bool(){ return surface_ != nullptr; }

	bool lock()
	{
		assert(isEnabled());
		if(!isLocked()){
			return (SDL_LockSurface(get()) == 0);
		}
		else{
			return true;
		}
	}
	void unlock()
	{
		assert(isEnabled());
		if(isLocked()){
			SDL_UnlockSurface(get());
		}
	}
	bool blitScaled(std::shared_ptr<Image> src, const Rect *srcrect, Rect *dstrect)
	{
		assert(isEnabled() && src->isEnabled());
		return SDL_BlitScaled(src->get(), srcrect, get(), dstrect) == 0;
	}
	bool blit(std::shared_ptr<Image> src, const Rect &srcrect, Sint16 nXDest, Sint16 nYDest)
	{
		Rect dstrect{nXDest, nYDest, 0, 0};
		return SDL_BlitSurface(src->get(), &srcrect, get(), &dstrect) == 0;
	}
	bool blit(std::shared_ptr<Image> src, Sint16 nXDest, Sint16 nYDest)
	{
		Rect dstrect{nXDest, nYDest, 0, 0};
		return SDL_BlitSurface(src->get(), nullptr, get(), &dstrect) == 0;
	}
	bool fillRect(const Rect &rect, const Uint32 color)
	{
		return SDL_FillRect(get(), &rect, color) == 0;
	}
	bool fillRect(const Rect &rect, const Color &color)
	{
		return fillRect(rect, mapRGBA(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha()));
	}
	bool fillRect(const Uint32 color)
	{
		assert(isEnabled());
		return SDL_FillRect(get(), nullptr, color) == 0;
	}
	bool fillRect(const Color &color)
	{
		assert(isEnabled());
		return fillRect(mapRGBA(color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha()));
	}

	const Rect getClipRect()
	{
		Rect result;
		SDL_GetClipRect(get(), &result);
		return result;
	}
	Uint32 getColorKey()
	{
		Uint32 result;
		SDL_GetColorKey(get(), &result);
		return result;
	}
	const Color getColorMod() {
		Uint8 r, g, b, a;
		SDL_GetSurfaceAlphaMod(get(), &a);
		SDL_GetSurfaceColorMod(get(), &r, &g, &b);
		return Color(r, g, b, a);
	}
	SDL_BlendMode getBlendMode()
	{
		SDL_BlendMode blendMode;
		SDL_GetSurfaceBlendMode(get(), &blendMode);
		return blendMode;
	}
	bool saveBMP(const char *file) { return ::SDL_SaveBMP(get(), file) == 0; }
	void setClipRect(const Rect &rect) { ::SDL_SetClipRect(get(), &rect); }
	void clearClipRect() { ::SDL_SetClipRect(get(), nullptr); }
	void setColorKey(Uint32 color) { ::SDL_SetColorKey(get(), SDL_TRUE, color); }
	void setColorKey(const Color &color) { ::SDL_SetColorKey(get(), SDL_TRUE, mapRGB(color.getRed(), color.getGreen(), color.getBlue())); }
	void clearColorKey() { ::SDL_SetColorKey(get(), SDL_FALSE, 0); }
	void setAlphaMod(Uint8 alpha) { ::SDL_SetSurfaceAlphaMod(get(), alpha); }
	void setBlendMode(SDL_BlendMode blendMode) { ::SDL_SetSurfaceBlendMode(get(), blendMode); }
	void setColorMod(Uint8 r, Uint8 g, Uint8 b) { ::SDL_SetSurfaceColorMod(get(), r, g, b); }
	//SDL_SetSurfacePalette
	//SDL_SetSurfaceRLE

	Uint32 mapRGB(Uint8 r, Uint8 g, Uint8 b){ assert(isEnabled()); return SDL_MapRGB(get()->format, r, g, b);}
	Uint32 mapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a){ assert(isEnabled()); return SDL_MapRGBA(get()->format, r, g, b, a);}
	bool isEnabled() const { return get() !=  nullptr;}
	bool isLocked() const { return SDL_MUSTLOCK(get()) == 0;}
	int getHeight() const { assert(isEnabled()); return get()->h;}
	int getWidth() const { assert(isEnabled()); return get()->w;}
	const geo::Sizei getSize() const { return {getWidth(), getHeight()}; }
	const SDL_PixelFormat *GetPixelFormat() const { assert(isEnabled()); return get()->format;}
	const Uint32 getFlags() const { assert(isEnabled()); return get()->flags;}
	const void *getPixels() const { return get()->pixels;}

	bool fillRoundedBox( int xo, int yo, int w, int h, int r, Uint32 color );

private:
	SDL_Surface * const surface_;
};

// 大きなシート画像(sheet)の一部(rect)を指す軽量な参照。スプライトシートの
// 1コマ分を、都度コピーを作らずにblit(sheet, rect, x, y)で直接描画するために使う
struct SubImage
{
	std::shared_ptr<Image> sheet;
	Rect rect;
};

} // SDL_

#endif // SDLIMAGE_H_
