#include "scene/useitem/UseItemScene.h"
#include "use_item.h"
#include "context.h"

UseItemScene::UseItemScene()
	: GameScene(CONTEXT_USE)
{
}

void UseItemScene::onEnter()
{
	use_item_enter();
}

void UseItemScene::onLeave()
{
	use_item_leave();
}
