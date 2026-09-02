#if !defined(ENDINGSCENE_H_)
#define ENDINGSCENE_H_

#include "scene/GameScene.h"

class EndingScene final : public GameScene
{
public:
	EndingScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // ENDINGSCENE_H_
