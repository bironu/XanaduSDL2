#include "pause.h"
#include "app/Application.h"
#include "scene/GameScene.h"

namespace {

GameScene *currentGameScene()
{
	return dynamic_cast<GameScene *>(Application::instance().getCurrentScene().get());
}

} // namespace

void begin_wait(std::function<bool()> isDone, std::function<void()> onComplete, uint32_t maxWaitMs)
{
	if (auto *scene = currentGameScene()) {
		scene->wait(std::move(isDone), std::move(onComplete), maxWaitMs);
	}
}

void begin_pause(int clearkey, std::function<void()> onComplete)
{
	if (auto *scene = currentGameScene()) {
		scene->pauseFor(clearkey, std::move(onComplete));
	}
}
