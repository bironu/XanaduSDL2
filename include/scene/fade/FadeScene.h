#if !defined(FADESCENE_H_)
#define FADESCENE_H_

#include "scene/GameScene.h"

class FadeScene final : public GameScene
{
public:
	FadeScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // FADESCENE_H_
