#if !defined(TOWERSCENE_H_)
#define TOWERSCENE_H_

#include "scene/GameScene.h"

class TowerScene final : public GameScene
{
public:
	TowerScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // TOWERSCENE_H_
