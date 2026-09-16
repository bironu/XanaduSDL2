#include "app/Application.h"
#include "sdl/SDLWindow.h"
#include "resources/Resources.h"
#include "task/TaskManager.h"
#include "scene/menu/MenuScene.h"
#include "sdl/LegacyPlatform.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <memory>

int main(int argc, char *argv[])
{
	Application app(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
	if(!app.isApplication()){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL Init error. %s", SDL_GetError());
		return 1;
	}

	Resources res;
	res.reload();
	const int width = res.getWindowWidth();
	const int height = res.getWindowHeight();

	auto mainWindow = std::make_shared<SDL_::Window>("xanadu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	app.registerMainWindow(mainWindow);

	initLegacyGraphics(res);
	initLegacySound(res);

	app.registerNextScene(std::make_shared<MenuScene>());

	TaskManager manager;
	return app.run(res, manager);
}
