#if !defined(MENUSCENE_H_)
#define MENUSCENE_H_

#include "scene/GameScene.h"

class MenuScene final : public GameScene
{
public:
	MenuScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // MENUSCENE_H_
