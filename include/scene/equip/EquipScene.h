#if !defined(EQUIPSCENE_H_)
#define EQUIPSCENE_H_

#include "scene/GameScene.h"

class EquipScene final : public GameScene
{
public:
	EquipScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // EQUIPSCENE_H_
