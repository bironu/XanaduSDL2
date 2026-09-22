#if !defined(APPLICATION_H_)
#define APPLICATION_H_

#include "misc/Uncopyable.h"
#include "scene/Scene.h"
#include "sdl/SDLTimer.h"
#include <SDL3/SDL.h>
#include <memory>
#include <stack>

namespace SDL_
{
class Window;
namespace Mix_
{
class Mixer;
}
}

class Resources;
class TaskManager;
class Application final
{
public:
	UNCOPYABLE(Application);
	explicit Application(Uint32 flags);
	~Application();

	void registerMainWindow(std::shared_ptr<SDL_::Window> mainWindow);
	std::shared_ptr<SDL_::Window> getMainWindow();
	void registerWindow(std::shared_ptr<SDL_::Window> window);
	void unregisterWindow(std::shared_ptr<SDL_::Window> window);
	std::shared_ptr<SDL_::Window> getWindow(int id);

	bool initSubSystem(Uint32 flags) { return ::SDL_InitSubSystem(flags); }
	void quitSubSystem(Uint32 flags) { ::SDL_QuitSubSystem(flags); }
	bool isApplication() const { return is_application_; }
	bool isTtf() const { return is_ttf_; }
	bool isMixer() const { return is_mixer_; }
	SDL_::Mix_::Mixer &getMixer() { return *mixer_; }
	int run(Resources &, TaskManager &manager);
	void clearResumeStack();

	void registerNextScene(std::shared_ptr<Scene> nextScene)
	{
		nextScene_ = nextScene;
	}

	std::shared_ptr<Scene> getCurrentScene() const { return currentScene_; }

	// 旧C時代の自由関数(context.cpp の switch_context 等)からアプリ本体に
	// アクセスするための参照。インスタンスは常に高々1つしか生成されない前提。
	static Application &instance() { return *instance_; }

	//static void waitFrame();
	void quit(const int val = 0);

	static uint32_t getTickCount() { return ::SDL_GetTicks(); }
    void setTimer(int interval, SDL_::Timer::Callback timer_proc);
    void killTimer(void);

    static const bool *getKeybordState() { return keybordState_; }

private:
	bool handlePreEvent(Resources &res, TaskManager &manager, SDL_Event &);

	const bool is_application_;
	const bool is_ttf_;
	const bool is_mixer_;
    std::unique_ptr<SDL_::Timer> timer_;
	std::unique_ptr<SDL_::Mix_::Mixer> mixer_;
	std::shared_ptr<Scene> currentScene_;
	std::stack<std::shared_ptr<Scene>> stackResumeScene_;
	std::vector<std::shared_ptr<SDL_::Window>> listWindow_;
	std::shared_ptr<SDL_::Window> mainWindow_;
	std::shared_ptr<Scene> nextScene_;
	int return_code_;

    static const bool *keybordState_;
	static Application *instance_; // TODO: singletonはやめる
};

#endif // APPLICATION_H_
