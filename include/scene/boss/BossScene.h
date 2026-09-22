#if !defined(BOSSSCENE_H_)
#define BOSSSCENE_H_

#include "scene/GameScene.h"

class BossScene final : public GameScene
{
public:
	BossScene();

protected:
	void onCreate(uint32_t tick) override;
	void onDestroy(uint32_t tick) override;
	void onEnter() override;
	void onLeave() override;
};

#endif // BOSSSCENE_H_
