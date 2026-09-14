#include "scene/ending/EndingScene.h"
#include "ending.h"
#include "context.h"

EndingScene::EndingScene()
	: GameScene(CONTEXT_ENDING)
{
}

void EndingScene::onEnter()
{
	ending_enter();
}

void EndingScene::onLeave()
{
	ending_leave();
}
