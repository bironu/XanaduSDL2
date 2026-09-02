#ifndef SDLMIXMUSIC_H_
#define SDLMIXMUSIC_H_

struct Mix_Music;

namespace SDL_
{
namespace Mix_
{

class Music {
public:
	Music(const char *);
	~Music();

	Mix_Music *get() const { return music_; }

private:
	Mix_Music * const music_;
};

} // namespace Mix_
} // namespace SDL_

#endif // SDLMIXMUSIC_H_
