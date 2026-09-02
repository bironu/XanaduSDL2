#include "scene/pause/PauseScene.h"
#include "pause.h"
#include "context.h"

PauseScene::PauseScene()
	: GameScene(CONTEXT_PAUSE, []{ return std::make_shared<PauseScene>(); })
{
}

void PauseScene::onEnter()
{
	pause_enter();
}

void PauseScene::onLeave()
{
	pause_leave();
}
