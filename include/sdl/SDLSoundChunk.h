#ifndef SDLSOUNDCHUNK_H_
#define SDLSOUNDCHUNK_H_

struct Mix_Chunk;

namespace SDL_ {

class SoundChunk {
public:
	SoundChunk(const char *);
	~SoundChunk();

	Mix_Chunk *get() const { return chunk_; }

private:
	Mix_Chunk * const chunk_;
};

} // namespace SDL_

#endif // SDLSOUNDCHUNK_H_
