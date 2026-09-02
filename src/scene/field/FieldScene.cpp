#include "scene/field/FieldScene.h"
#include "field.h"
#include "context.h"

FieldScene::FieldScene()
	: GameScene(CONTEXT_FIELD, []{ return std::make_shared<FieldScene>(); })
{
}

void FieldScene::onEnter()
{
	field_enter();
}

void FieldScene::onLeave()
{
	field_leave();
}
