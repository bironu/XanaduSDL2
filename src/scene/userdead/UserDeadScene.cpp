#include "scene/userdead/UserDeadScene.h"
#include "user_dead.h"
#include "context.h"

UserDeadScene::UserDeadScene()
	: GameScene(CONTEXT_USER_DEAD, []{ return std::make_shared<UserDeadScene>(); })
{
}

void UserDeadScene::onEnter()
{
	user_dead_enter();
}

void UserDeadScene::onLeave()
{
	user_dead_leave();
}
