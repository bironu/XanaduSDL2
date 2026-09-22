#include "scene/field/FieldScene.h"
#include "field.h"
#include "context.h"

FieldScene::FieldScene()
	: GameScene(CONTEXT_FIELD)
{
}

void FieldScene::onCreate(uint32_t /*tick*/)
{
	field_create();
}

void FieldScene::onDestroy(uint32_t /*tick*/)
{
	field_destroy();
}

void FieldScene::onEnter()
{
	field_enter();
}

void FieldScene::onLeave()
{
	field_leave();
}
