#include "sdl/SDLImage.h"

namespace SDL_
{

Image::Image(int width, int height)
	: Image(SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN // OpenGL RGBA masks
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000
#else
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
#endif
			))
{
}
Image::Image(const char * const file)
	: Image(IMG_Load(file))
{
}

// 親シート(source)の矩形(rect)部分を独立したImageとして複製する。
// 旧C実装のsubsection_image()が「親バッファの一部を指すビュー」を作っていたのに対し、
// Imageはコピー不可のため実データを複製する。複製後、左上ピクセルの色を
// 透過色として設定する(旧tmpl_load()がBMP左上ピクセルをmaskにしていた慣習を踏襲)。
Image::Image(const std::shared_ptr<Image> &source, const Rect &rect)
	: Image(rect.w, rect.h)
{
	SDL_BlitSurface(source->get(), &rect, get(), nullptr);

	lock();
	const Uint32 topLeft = static_cast<const Uint32 *>(getPixels())[0];
	unlock();
	setColorKey(topLeft);
}

Image::~Image()
{
	if(isEnabled()){
		SDL_FreeSurface(get());
	}
}

bool Image::fillRoundedBox( int xo, int yo, int w, int h, int r, Uint32 color )
{
	SDL_Surface* dst = this->get();

	const int yd = dst->pitch / dst->format->BytesPerPixel;

	const int rpsqrt2 = static_cast<int>(r / sqrt(2.0));
	//const double r2 = r * r;

	w /= 2;
	h /= 2;

	xo += w;
	yo += h;

	w -= r;
	h -= r;

	if( w < 0 || h < 0 ) {
		return false;
	}

	SDL_LockSurface( dst );

	Uint32 *pixels = (Uint32*)( dst->pixels );

	int sy = (yo-h)*yd;
	int ey = (yo+h)*yd;
	int sx = (xo-w);
	int ex = (xo+w);
	for(int i = sy; i<=ey; i+=yd ) {
		for(int j = sx-r; j<=ex+r; j++ ) {
			pixels[i+j] = color;
		}
	}

	int d = -r;
	int x2m1 = -1;
	int y = r;
	for(int x=0; x <= rpsqrt2; x++ ) {
		x2m1 += 2;
		d += x2m1;
		if( d >= 0 ) {
			y--;
			d -= (y*2);
		}

		for(int i=sx-x; i<=ex+x; i++ ) {
			pixels[sy-y*yd + i] = color;
		}
		for(int i=sx-y; i<=ex+y; i++ ) {
			pixels[sy-x*yd + i] = color;
		}
		for(int i=sx-y; i<=ex+y; i++ ) {
			pixels[ey+x*yd + i] = color;
		}
		for(int i=sx-x; i<=ex+x; i++ ) {
			pixels[ey+y*yd + i] = color;
		}
	}
	SDL_UnlockSurface( dst );

	return true;
}

} // SDL_
