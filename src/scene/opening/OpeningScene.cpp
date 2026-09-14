#include "scene/opening/OpeningScene.h"
#include "opening.h"
#include "context.h"

OpeningScene::OpeningScene()
	: GameScene(CONTEXT_OPENING)
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
