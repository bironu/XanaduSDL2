#if !defined(SHOPSCENE_H_)
#define SHOPSCENE_H_

#include "scene/GameScene.h"

class ShopScene final : public GameScene
{
public:
	ShopScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // SHOPSCENE_H_
