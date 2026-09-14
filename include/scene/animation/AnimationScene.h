#if !defined(ANIMATIONSCENE_H_)
#define ANIMATIONSCENE_H_

#include "scene/GameScene.h"

class AnimationScene final : public GameScene
{
public:
	AnimationScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // ANIMATIONSCENE_H_
