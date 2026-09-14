#include "scene/boss/BossScene.h"
#include "boss.h"
#include "context.h"

BossScene::BossScene()
	: GameScene(CONTEXT_BOSS)
{
}

void BossScene::onEnter()
{
	boss_enter();
}

void BossScene::onLeave()
{
	boss_leave();
}
