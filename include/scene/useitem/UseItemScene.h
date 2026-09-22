#if !defined(USEITEMSCENE_H_)
#define USEITEMSCENE_H_

#include "scene/GameScene.h"

class UseItemScene final : public GameScene
{
public:
	UseItemScene();

protected:
	void onCreate(uint32_t tick) override;
	void onDestroy(uint32_t tick) override;
	void onEnter() override;
	void onLeave() override;
};

#endif // USEITEMSCENE_H_
