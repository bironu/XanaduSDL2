#include "scene/opening/OpeningScene.h"
#include "opening.h"
#include "context.h"

OpeningScene::OpeningScene()
	: GameScene(CONTEXT_OPENING, []{ return std::make_shared<OpeningScene>(); })
{
}

void OpeningScene::onEnter()
{
	opening_enter();
}

void OpeningScene::onLeave()
{
	opening_leave();
}
