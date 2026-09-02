#if !defined(OPENINGSCENE_H_)
#define OPENINGSCENE_H_

#include "scene/GameScene.h"

class OpeningScene final : public GameScene
{
public:
	OpeningScene();

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // OPENINGSCENE_H_
