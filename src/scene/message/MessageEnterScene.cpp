#include "scene/message/MessageEnterScene.h"
#include "message.h"

MessageEnterScene::MessageEnterScene(int contextId)
	: GameScene(contextId, [contextId]{ return std::make_shared<MessageEnterScene>(contextId); })
{
}

void MessageEnterScene::onEnter()
{
	message_enter_enter();
}

void MessageEnterScene::onLeave()
{
	message_enter_leave();
}
