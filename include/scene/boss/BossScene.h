#if !defined(BOSSSCENE_H_)
#define BOSSSCENE_H_

#include "scene/GameScene.h"

class BossScene final : public GameScene
{
public:
	BossScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // BOSSSCENE_H_
