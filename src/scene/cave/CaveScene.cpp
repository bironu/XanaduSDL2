#include "scene/cave/CaveScene.h"
#include "cave.h"
#include "context.h"

CaveScene::CaveScene()
	: GameScene(CONTEXT_CAVE)
{
}

void CaveScene::onEnter()
{
	cave_enter();
}

void CaveScene::onLeave()
{
	cave_leave();
}
