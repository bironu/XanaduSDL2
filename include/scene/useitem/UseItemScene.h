#if !defined(USEITEMSCENE_H_)
#define USEITEMSCENE_H_

#include "scene/GameScene.h"

class UseItemScene final : public GameScene
{
public:
	UseItemScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // USEITEMSCENE_H_
