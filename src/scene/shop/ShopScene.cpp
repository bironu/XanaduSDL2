#include "scene/shop/ShopScene.h"
#include "shop.h"
#include "context.h"

ShopScene::ShopScene()
	: GameScene(CONTEXT_SHOP)
{
}

void ShopScene::onCreate(uint32_t /*tick*/)
{
	shop_create();
}

void ShopScene::onDestroy(uint32_t /*tick*/)
{
	shop_destroy();
}

void ShopScene::onEnter()
{
	shop_enter();
}

void ShopScene::onLeave()
{
	shop_leave();
}
