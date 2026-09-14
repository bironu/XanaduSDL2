#include "scene/field/FieldScene.h"
#include "field.h"
#include "context.h"

FieldScene::FieldScene()
	: GameScene(CONTEXT_FIELD)
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
