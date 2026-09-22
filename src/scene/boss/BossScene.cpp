#include "scene/boss/BossScene.h"
#include "boss.h"
#include "context.h"

BossScene::BossScene()
	: GameScene(CONTEXT_BOSS)
{
}

void BossScene::onCreate(uint32_t /*tick*/)
{
	boss_create();
}

void BossScene::onDestroy(uint32_t /*tick*/)
{
	boss_destroy();
}

void BossScene::onEnter()
{
	boss_enter();
}

void BossScene::onLeave()
{
	boss_leave();
}
