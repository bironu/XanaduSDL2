#include "app/Application.h"
#include "sdl/SDLWindow.h"
#include "sdl/SDLMixMixer.h"
#include "resources/Resources.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL_log.h>
#include <iostream>

Application *Application::instance_ = nullptr;

Application::Application(Uint32 flags)
	: is_application_(::SDL_Init(flags))
	, is_ttf_(::TTF_Init())
	, is_image_(true)
	, is_mixer_(MIX_Init())
	, mixer_(std::make_unique<SDL_::Mix_::Mixer>())
	, currentScene_()
	, stackResumeScene_()
	, listWindow_()
	, mainWindow_()
	, nextScene_()
	, return_code_(0)
{
	instance_ = this;
}

Application::~Application()
{
	instance_ = nullptr;
	listWindow_.clear();
	mixer_.reset();
    if (isMixer()){
        ::MIX_Quit();
    }
	if (isTtf()){
		::TTF_Quit();
	}
	if (isApplication()){
		::SDL_Quit();
	}
}

void Application::registerMainWindow(std::shared_ptr<SDL_::Window> mainWindow)
{
	mainWindow_ = mainWindow;
	registerWindow(mainWindow);
}

std::shared_ptr<SDL_::Window> Application::getMainWindow()
{
	return mainWindow_;
}

void Application::registerWindow(std::shared_ptr<SDL_::Window> window)
{
	listWindow_.push_back(window);
}

void Application::unregisterWindow(std::shared_ptr<SDL_::Window> window)
{
	listWindow_.erase(std::remove(std::begin(listWindow_), std::end(listWindow_), window), std::end(listWindow_));
}

std::shared_ptr<SDL_::Window> Application::getWindow(int id)
{
	std::shared_ptr<SDL_::Window> result;
	for(auto window : listWindow_) {
		if(window->getWindowId() == id) {
			result = window;
			break;
		}
	}
	return result;
}

int Application::run(Resources &res, TaskManager &manager)
{
	currentScene_ = nextScene_;
	currentScene_->prepare(this, &res, &manager);
	currentScene_->onCreate(getTickCount());
    currentScene_->onResume(getTickCount());
	nextScene_.reset();

	SDL_Event event;
	bool idle(false);
	while(currentScene_) {
		const auto tick = getTickCount();
		if(::SDL_PollEvent(&event)){
			if (!handlePreEvent(res, manager, event)) {
				currentScene_->dispatch(event);
			}
			idle = true;
		}
		else if (idle) {
			idle = currentScene_->onIdle(tick);
		}
		else {
			if (!::SDL_WaitEvent(nullptr)){
				::SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_WaitEvent Error!! %s", ::SDL_GetError());
				quit(1);
			}
		}
		if (currentScene_ && currentScene_->isFinished()) {
            currentScene_->onSuspend();
			currentScene_->onDestroy(tick);
			currentScene_.reset();
		}

		if (nextScene_) {
			if (currentScene_) {
                currentScene_->onSuspend();
				stackResumeScene_.push(currentScene_);
			}
			currentScene_ = nextScene_;
			nextScene_.reset();
        	currentScene_->prepare(this, &res, &manager);
			currentScene_->onCreate(tick);
            currentScene_->onResume(tick);
		}
		else if (!currentScene_) {
			if(!stackResumeScene_.empty()){
				currentScene_ = stackResumeScene_.top();
				stackResumeScene_.pop();
				currentScene_->onResume(tick);
			}
            else {
                break;
            }
		}
	}
	clearResumeStack();
	return return_code_;
}

void Application::clearResumeStack()
{
	while(!stackResumeScene_.empty()){stackResumeScene_.pop();}
}

void Application::quit(const int val)
{
	SDL_Event event = {SDL_EVENT_QUIT};
	return_code_ = val;
	::SDL_PushEvent(&event);
}

void Application::updateWindow(Uint32 id)
{
	SDL_Event event = {SDL_EVENT_WINDOW_EXPOSED};
	event.window.windowID = id;
	::SDL_PushEvent(&event);
}

bool Application::handlePreEvent(Resources &res, TaskManager &manager, SDL_Event &event)
{
	bool result = false;

	switch(event.type)
	{
	case SDL_EVENT_QUIT:
		currentScene_.reset();
		clearResumeStack();
		nextScene_ = nullptr;
		result = true;
		break;

	case SDL_EVENT_JOYSTICK_ADDED:
		res.addJoyDevice(event.jdevice);
		//currentScene_->onAddJoystick(event.jdevice.which);
		break;

	case SDL_EVENT_JOYSTICK_REMOVED:
		res.removeJoyDevice(event.jdevice);
		break;

	// SDL3ではSDL_WINDOWEVENT+ネストしたevent.window.eventによる分岐は廃止され、
	// 個々のウィンドウイベントがトップレベルのevent.typeとして独立している
	case SDL_EVENT_WINDOW_SHOWN:
		SDL_Log("Window %d shown", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_HIDDEN:
		SDL_Log("Window %d hidden", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_EXPOSED:
		SDL_Log("Window %d exposed", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_MOVED:
		SDL_Log("Window %d moved to %d,%d",
				event.window.windowID, event.window.data1, event.window.data2);
		break;
	case SDL_EVENT_WINDOW_RESIZED:
		res.setWindowWidth(event.window.data1);
		res.setWindowHeight(event.window.data2);
		SDL_Log("Window %d resized to %dx%d",
				event.window.windowID, event.window.data1, event.window.data2);
		break;
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		SDL_Log("Window %d pixel size changed to %dx%d",
				event.window.windowID, event.window.data1, event.window.data2);
		break;
	case SDL_EVENT_WINDOW_MINIMIZED:
		SDL_Log("Window %d minimized", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_MAXIMIZED:
		SDL_Log("Window %d maximized", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_RESTORED:
		SDL_Log("Window %d restored", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_MOUSE_ENTER:
		SDL_Log("Mouse entered window %d", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_MOUSE_LEAVE:
		SDL_Log("Mouse left window %d", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
		SDL_Log("Window %d gained keyboard focus", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_FOCUS_LOST:
		SDL_Log("Window %d lost keyboard focus", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		SDL_Log("Window %d closed", event.window.windowID);
		break;
	case SDL_EVENT_WINDOW_HIT_TEST:
		SDL_Log("Window %d has a special hit test", event.window.windowID);
		break;

	default:
		break;
	}

	return result;
}

//void Application::waitFrame()
//{
//	static const Uint32 wait((1000<<16)/60);
//	static Uint32 lasttime(0); // last time
//	static Uint32 passage(0);
//
//	const Uint32 t(::SDL_GetTicks()); // now time
//	const Uint32 progress(t - lasttime);
//	passage = (passage & 0xffff) + wait;
//	const Uint32 twait(passage >> 16);
//	if(progress >= twait){
//		lasttime = t;
//	}
//	else{
//		::SDL_Delay(twait-progress);
//		while((::SDL_GetTicks()-lasttime) < twait);
//		lasttime += twait;
//	}
//}
