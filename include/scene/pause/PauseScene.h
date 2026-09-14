#if !defined(PAUSESCENE_H_)
#define PAUSESCENE_H_

#include "scene/GameScene.h"

class PauseScene final : public GameScene
{
public:
	PauseScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // PAUSESCENE_H_
