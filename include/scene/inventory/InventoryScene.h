#if !defined(INVENTORYSCENE_H_)
#define INVENTORYSCENE_H_

#include "scene/GameScene.h"

class InventoryScene final : public GameScene
{
public:
	InventoryScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // INVENTORYSCENE_H_
