#include "scene/animation/AnimationScene.h"
#include "animation.h"
#include "context.h"

AnimationScene::AnimationScene()
	: GameScene(CONTEXT_ANIMATION)
{
}

void AnimationScene::onEnter()
{
	animation_enter();
}

void AnimationScene::onLeave()
{
	animation_leave();
}
