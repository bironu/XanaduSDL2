#if !defined(CAVESCENE_H_)
#define CAVESCENE_H_

#include "scene/GameScene.h"

class CaveScene final : public GameScene
{
public:
	CaveScene();

protected:
	void onDestroy(uint32_t tick) override;
	void onEnter() override;
	void onLeave() override;
};

#endif // CAVESCENE_H_
