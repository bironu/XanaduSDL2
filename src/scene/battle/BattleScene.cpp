#include "scene/battle/BattleScene.h"
#include "battle.h"
#include "context.h"

BattleScene::BattleScene()
	: GameScene(CONTEXT_BATTLE)
{
}

void BattleScene::onCreate(uint32_t /*tick*/)
{
	battle_create();
}

void BattleScene::onDestroy(uint32_t /*tick*/)
{
	battle_destroy();
}

void BattleScene::onEnter()
{
	battle_enter();
}

void BattleScene::onLeave()
{
	battle_leave();
}
