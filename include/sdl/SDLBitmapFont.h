#ifndef SDLBITMAPFONT_H_
#define SDLBITMAPFONT_H_

#include "misc/Uncopyable.h"
#include <memory>

namespace SDL_
{
class Image;
class Color;

// Ported from the original draw_text()/fonts[128] found in init.c: a fixed
// 16x16-per-glyph bitmap font sheet (user/font.bmp, ImageId::user_font) laid
// out as 128 ASCII glyphs in a single row. The original treated each glyph
// as a stencil (draw_plane): any non-background pixel was painted with
// whatever color the caller asked for, regardless of the glyph's own pixel
// value. This class reproduces that by baking the sheet down to a
// white-ink/transparent-background mask once, then tinting it per call via
// surface color modulation.
class BitmapFont
{
public:
	UNCOPYABLE(BitmapFont);
	static constexpr int GlyphSize = 16;
	static constexpr int GlyphCount = 128;

	explicit BitmapFont(const Image &atlas);
	~BitmapFont();

	std::shared_ptr<Image> renderSolidText(const char *text, const Color &fg) const;

private:
	std::shared_ptr<Image> mask_;
};

} // namespace SDL_

#endif // SDLBITMAPFONT_H_
