#include "scene/Scene.h"
#include "sdl/SDLWindow.h"
#include "sdl/LegacyPlatform.h"
#include "app/Application.h"
#include "resources/Resources.h"
#include "task/TaskManager.h"
#include <SDL3/SDL_log.h>

Scene::Scene()
	: app_(nullptr)
	, res_(nullptr)
	, manager_(nullptr)
	, isFinished_(false)
{
}

void Scene::prepare(Application *app, Resources *res, TaskManager *manager)
{
	app_ = app;
	res_ = res;
	manager_ = manager;
}

// void Scene::clear()
// {
// 	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
// }

void Scene::swap()
{
	presentLegacyFrame();
	app_->getMainWindow()->swap();
}

bool Scene::onIdle(uint32_t tick)
{
	const bool stillRunning = !manager_->compute(tick);
	/* レンダラはSDL_SetRenderVSync()でVSync有効にして生成されているため、
	   ここで毎回present()してもリフレッシュレートで自然にペーシングされる */
	swap();
	return stillRunning;
}

void Scene::onCreate(uint32_t tick)
{
}

void Scene::registerTask(int id, std::shared_ptr<Task> task)
{
	manager_->registerTask(id, std::move(task));
}

void Scene::unregisterTask(int id, bool isFinishAction)
{
	manager_->unregisterTask(id, isFinishAction);
}

void Scene::onDestroy(uint32_t tick)
{
}

void Scene::onResume(uint32_t tick)
{
}

void Scene::quit()
{
	app_->quit();
}
