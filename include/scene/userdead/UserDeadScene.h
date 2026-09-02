#if !defined(USERDEADSCENE_H_)
#define USERDEADSCENE_H_

#include "scene/GameScene.h"

class UserDeadScene final : public GameScene
{
public:
	UserDeadScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // USERDEADSCENE_H_
