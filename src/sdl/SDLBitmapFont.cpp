#include "sdl/SDLBitmapFont.h"
#include "sdl/SDLImage.h"
#include "sdl/SDLColor.h"
#include "geo/Rect.h"
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_pixels.h>
#include <cstring>

namespace SDL_
{

namespace
{
std::shared_ptr<Image> buildMask(const Image &atlas)
{
	SDL_Surface *converted = ::SDL_ConvertSurfaceFormat(atlas.get(), SDL_PIXELFORMAT_RGBA32, 0);
	auto mask = std::make_shared<Image>(converted);

	mask->lock();
	Uint32 *pixels = static_cast<Uint32 *>(mask->get()->pixels);
	const int count = mask->getWidth() * mask->getHeight();
	const SDL_PixelFormat *format = mask->get()->format;
	for (int i = 0; i < count; ++i) {
		Uint8 r, g, b, a;
		::SDL_GetRGBA(pixels[i], format, &r, &g, &b, &a);
		pixels[i] = (r == 0 && g == 0 && b == 0)
			? ::SDL_MapRGBA(format, 0, 0, 0, 0)
			: ::SDL_MapRGBA(format, 255, 255, 255, 255);
	}
	mask->unlock();
	mask->setBlendMode(SDL_BLENDMODE_BLEND);

	return mask;
}
} // namespace

BitmapFont::BitmapFont(const Image &atlas)
	: mask_(buildMask(atlas))
{
}

BitmapFont::~BitmapFont() = default;

std::shared_ptr<Image> BitmapFont::renderSolidText(const char *text, const Color &fg) const
{
	const std::size_t len = std::strlen(text);
	if (len == 0) {
		return nullptr;
	}

	auto dest = std::make_shared<Image>(static_cast<int>(len) * GlyphSize, GlyphSize);
	dest->setBlendMode(SDL_BLENDMODE_BLEND);
	dest->fillRect(Color(0, 0, 0, 0));

	mask_->setColorMod(fg.getRed(), fg.getGreen(), fg.getBlue());

	for (std::size_t i = 0; i < len; ++i) {
		const unsigned char ch = static_cast<unsigned char>(text[i]);
		// Matches the original draw_text(): control characters and spaces
		// still advance the cursor but draw nothing.
		if ((ch & 0x7f) <= 0x20 || ch >= GlyphCount) {
			continue;
		}
		const Rect src(ch * GlyphSize, 0, GlyphSize, GlyphSize);
		dest->blit(mask_, src, static_cast<Sint16>(i * GlyphSize), 0);
	}

	return dest;
}

} // namespace SDL_
