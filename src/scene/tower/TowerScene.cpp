#include "scene/tower/TowerScene.h"
#include "tower.h"
#include "context.h"

TowerScene::TowerScene()
	: GameScene(CONTEXT_TOWER, []{ return std::make_shared<TowerScene>(); })
{
}

void TowerScene::onEnter()
{
	tower_enter();
}

void TowerScene::onLeave()
{
	tower_leave();
}
