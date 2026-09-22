#include "scene/userdead/UserDeadScene.h"
#include "user_dead.h"
#include "context.h"

UserDeadScene::UserDeadScene()
	: GameScene(CONTEXT_USER_DEAD)
{
}

void UserDeadScene::onCreate(uint32_t /*tick*/)
{
	user_dead_create();
}

void UserDeadScene::onDestroy(uint32_t /*tick*/)
{
	user_dead_destroy();
}

void UserDeadScene::onEnter()
{
	user_dead_enter();
}

void UserDeadScene::onLeave()
{
	user_dead_leave();
}
