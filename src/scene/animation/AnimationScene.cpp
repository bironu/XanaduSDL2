#include "scene/animation/AnimationScene.h"
#include "animation.h"
#include "context.h"

AnimationScene::AnimationScene()
	: GameScene(CONTEXT_ANIMATION, []{ return std::make_shared<AnimationScene>(); })
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
