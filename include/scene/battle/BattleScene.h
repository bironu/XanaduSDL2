#if !defined(BATTLESCENE_H_)
#define BATTLESCENE_H_

#include "scene/GameScene.h"

class BattleScene final : public GameScene
{
public:
	BattleScene();

protected:
	void onCreate(uint32_t tick) override;
	void onDestroy(uint32_t tick) override;
	void onEnter() override;
	void onLeave() override;
};

#endif // BATTLESCENE_H_
