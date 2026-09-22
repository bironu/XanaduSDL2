#include "scene/inventory/InventoryScene.h"
#include "inventory.h"
#include "context.h"

InventoryScene::InventoryScene()
	: GameScene(CONTEXT_INVENTORY)
{
}

void InventoryScene::onDestroy(uint32_t /*tick*/)
{
	inventory_destroy();
}

void InventoryScene::onEnter()
{
	inventory_enter();
}

void InventoryScene::onLeave()
{
	inventory_leave();
}
