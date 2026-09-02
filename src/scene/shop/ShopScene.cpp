#include "scene/shop/ShopScene.h"
#include "shop.h"
#include "context.h"

ShopScene::ShopScene()
	: GameScene(CONTEXT_SHOP, []{ return std::make_shared<ShopScene>(); })
{
}

void ShopScene::onEnter()
{
	shop_enter();
}

void ShopScene::onLeave()
{
	shop_leave();
}
