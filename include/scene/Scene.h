#if !defined(SCENE_H_)
#define SCENE_H_

#include "misc/Uncopyable.h"
#include <memory>
#include <functional>

namespace SDL_
{
class Window;
}

class Scene;
class Application;
class Resources;
class TaskManager;
using FuncCreateScene = std::function<std::shared_ptr<Scene>()>;
union SDL_Event;
class Task;
class Scene
{
public:
	UNCOPYABLE(Scene);
	explicit Scene();
	virtual ~Scene() = default;
	void prepare(Application*, Resources*, TaskManager*);

	void finish() { isFinished_ = true; }
	bool isFinished() const { return isFinished_; }
	void registerTask(int, std::shared_ptr<Task>);
	void unregisterTask(int, bool);

	virtual void dispatch(const SDL_Event &) = 0;
	virtual FuncCreateScene onSuspend() = 0;
	virtual bool onIdle(uint32_t);
	virtual void onCreate(uint32_t);
	virtual void onDestroy(uint32_t);
	virtual void onResume(uint32_t);

	// void clear();
	void swap();
	void quit();

	Application &getApplication() { return *app_; }
	Resources &getResources() { return *res_; }
	TaskManager &getManager() { return *manager_; }

private:
	Application *app_;
	Resources *res_;
	TaskManager *manager_;
	bool isFinished_;
};

#endif // SCENE_H_
