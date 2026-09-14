#include "scene/battle/BattleScene.h"
#include "battle.h"
#include "context.h"

BattleScene::BattleScene()
	: GameScene(CONTEXT_BATTLE)
{
}

void BattleScene::onEnter()
{
	battle_enter();
}

void BattleScene::onLeave()
{
	battle_leave();
}
