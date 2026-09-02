#include "scene/inventory/InventoryScene.h"
#include "inventory.h"
#include "context.h"

InventoryScene::InventoryScene()
	: GameScene(CONTEXT_INVENTORY, []{ return std::make_shared<InventoryScene>(); })
{
}

void InventoryScene::onEnter()
{
	inventory_enter();
}

void InventoryScene::onLeave()
{
	inventory_leave();
}
