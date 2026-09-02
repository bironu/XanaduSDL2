#include "scene/menu/MenuScene.h"
#include "menu.h"
#include "context.h"

MenuScene::MenuScene()
	: GameScene(CONTEXT_START_MENU, []{ return std::make_shared<MenuScene>(); })
{
}

void MenuScene::onEnter()
{
	menu_enter();
}

void MenuScene::onLeave()
{
	menu_leave();
}
