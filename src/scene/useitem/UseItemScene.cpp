#include "scene/useitem/UseItemScene.h"
#include "use_item.h"
#include "context.h"

UseItemScene::UseItemScene()
	: GameScene(CONTEXT_USE)
{
}

void UseItemScene::onCreate(uint32_t /*tick*/)
{
	use_item_create();
}

void UseItemScene::onDestroy(uint32_t /*tick*/)
{
	use_item_destroy();
}

void UseItemScene::onEnter()
{
	use_item_enter();
}

void UseItemScene::onLeave()
{
	use_item_leave();
}
