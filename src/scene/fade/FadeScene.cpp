#include "scene/fade/FadeScene.h"
#include "graphics.h"
#include "fade.h"
#include "context.h"

FadeScene::FadeScene()
	: GameScene(CONTEXT_FADE, []{ return std::make_shared<FadeScene>(); })
{
}

void FadeScene::onEnter()
{
	fade_enter();
}

void FadeScene::onLeave()
{
	fade_leave();
}
