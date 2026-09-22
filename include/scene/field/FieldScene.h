#if !defined(FIELDSCENE_H_)
#define FIELDSCENE_H_

#include "scene/GameScene.h"

class FieldScene final : public GameScene
{
public:
	FieldScene();

protected:
	void onCreate(uint32_t tick) override;
	void onDestroy(uint32_t tick) override;
	void onEnter() override;
	void onLeave() override;
};

#endif // FIELDSCENE_H_
