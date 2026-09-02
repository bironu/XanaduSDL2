#ifndef SDLMIXCHUNK_H_
#define SDLMIXCHUNK_H_

struct Mix_Chunk;

namespace SDL_
{
namespace Mix_
{

class Chunk {
public:
	Chunk(const char *);
	~Chunk();

	Mix_Chunk *get() const { return chunk_; }

private:
	Mix_Chunk * const chunk_;
};

} // namespace Mix_
} // namespace SDL_

#endif // SDLMIXCHUNK_H_
