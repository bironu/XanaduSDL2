#ifndef SDLTTFFONT_H_
#define SDLTTFFONT_H_

#include "misc/Uncopyable.h"
#include "geo/Vector2.h"
#include <SDL2/SDL_ttf.h>
#include <memory>

namespace SDL_
{
class Color;
class Image;
class TtfFont
{
public:
	UNCOPYABLE(TtfFont);
	TtfFont(const char *file, int ptsize)
		: font_(::TTF_OpenFont(file, ptsize))
		, fontSize_(ptsize)
	{
	}
	~TtfFont() {
		if(font_){
			::TTF_CloseFont(font_);
		}
	}

	int getFontSize() const { return fontSize_; }
	TTF_Font *get() const { return font_; }
	bool is_font() const { return font_; }

	bool getTextSize(const char *text, geo::Sizei *size);
	std::shared_ptr<SDL_::Image> renderSolidText(const char *text, const SDL_::Color &fg);
	std::shared_ptr<SDL_::Image> renderShadedText(const char *text, const SDL_::Color &fg, const SDL_::Color &bg);
	std::shared_ptr<SDL_::Image> renderBlendedText(const char *text, const SDL_::Color &fg);

private:
	TTF_Font * const font_;
	const int fontSize_;
};

} // namespace SDL_

#endif // SDLTTFFONT_H_
