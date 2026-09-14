#include "scene/equip/EquipScene.h"
#include "equip.h"
#include "context.h"

EquipScene::EquipScene()
	: GameScene(CONTEXT_EQUIPMENT)
{
}

void EquipScene::onEnter()
{
	equip_enter();
}

void EquipScene::onLeave()
{
	equip_leave();
}
