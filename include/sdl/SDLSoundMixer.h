#ifndef SDLSOUNDMIXER_H_
#define SDLSOUNDMIXER_H_

#include "misc/Uncopyable.h"

namespace SDL_ {

class SoundChunk;
class SoundMixer
{
public:
	UNCOPYABLE(SoundMixer);
	SoundMixer();
	~SoundMixer();

	int allocateChannels(int);
	int playChannel(int, SoundChunk &, int);

private:
	const int audioDevice_;

};

} // namespace SDL_

#endif // SDLSOUNDMIXER_H_
