#include "scene/ending/EndingScene.h"
#include "ending.h"
#include "context.h"

EndingScene::EndingScene()
	: GameScene(CONTEXT_ENDING, []{ return std::make_shared<EndingScene>(); })
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
