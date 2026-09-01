#if !defined(SDLWINDOW_H_)
#define SDLWINDOW_H_

#include "misc/Uncopyable.h"
#include "sdl/SDLRenderer.h"
#include "geo/Vector2.h"
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_messagebox.h>
#include <map>
#include <memory>

namespace SDL_
{

class Window
{
public:
	UNCOPYABLE(Window);
	Window(const char* title, int x, int y, int w, int h, Uint32 flags);
	~Window();

	bool isWindow() const { return window_; }
	SDL_Window *get() const { return window_; }

	//float getBrightness() const { return ::SDL_GetWindowBrightness(window_); }
	void* getUsaerData(const char* name) const { return ::SDL_GetWindowData(window_, name); }
	int getDisplayIndex() const { return ::SDL_GetWindowDisplayIndex(window_); }
	//int getDisplayMode(SDL_DisplayMode* mode) const { return ::SDL_GetWindowDisplayMode(window_, mode); }
	Uint32 getWindowFlag() const { return ::SDL_GetWindowFlags(window_); }
	//SDL_Window* getWindowFromID(Uint32 id) const { return ::SDL_GetWindowFromID(id); }
	//int getGammaRamp(Uint16* red, Uint16* green, Uint16* blue) const { return ::SDL_GetWindowGammaRamp(window_, red, green, blue); }
	//bool getGrab() const { return ::SDL_GetWindowGrab(window_); }
	Uint32 getWindowId() const { return ::SDL_GetWindowID(window_); }
	void getMaximumSize(int* w, int* h) const { ::SDL_GetWindowMaximumSize(window_, w, h); }
	void getMinimumSize(int* w, int* h) const { ::SDL_GetWindowMinimumSize(window_, w, h); }
	Uint32 getPixelFormat() const { return ::SDL_GetWindowPixelFormat(window_); }
	const Point getPosition() const
	{
		int x, y;
		::SDL_GetWindowPosition(window_, &x, &y);
		return {x, y};
	}
	const geo::Sizei getSize() const
	{
		int w, h;
		::SDL_GetWindowSize(window_, &w, &h);
		return {w, h};
	}
	//SDL_Surface* SDL_GetWindowSurface(SDL_Window* window);
	const char *getTitle() const { return ::SDL_GetWindowTitle(window_); }
	//SDL_bool SDL_GetWindowWMInfo(SDL_Window* window, SDL_SysWMinfo* info);
	void hide() { ::SDL_HideWindow(window_); }
	void maximize() { ::SDL_MaximizeWindow(window_); }
	void minimize() { ::SDL_MinimizeWindow(window_); }
	void raise() { ::SDL_RaiseWindow(window_); }
	void restore() { ::SDL_RestoreWindow(window_); }
	void setBordered(bool bordered) { ::SDL_SetWindowBordered(window_, bordered?SDL_TRUE:SDL_FALSE); }
	//int SDL_SetWindowBrightness(SDL_Window* window, float brightness);
	void* setUserData(const char* name, void* userdata) { return ::SDL_SetWindowData(window_, name, userdata); }
	//int SDL_SetWindowDisplayMode(SDL_Window* window, const SDL_DisplayMode* mode);
	int setFullscreen(Uint32 flags) { return ::SDL_SetWindowFullscreen(window_, flags); }
	//int SDL_SetWindowGammaRamp(SDL_Window* window, const Uint16* red, const Uint16* green, const Uint16* blue);
	//void SDL_SetWindowGrab(SDL_Window* window, SDL_bool grabbed);
	//int SDL_SetWindowHitTest(SDL_Window* window, SDL_HitTest callback, void* callback_data);
	//void setIcon(Surface &surface) { ::SDL_SetWindowIcon(window_, surface.get()); }
	void setMaximumSize(int max_w, int max_h) { ::SDL_SetWindowMaximumSize(window_, max_w, max_h); }
	void setMinimumSize(int min_w, int min_h) { ::SDL_SetWindowMinimumSize(window_, min_w, min_h); }
	void setPosition(int x, int y) { ::SDL_SetWindowPosition(window_, x, y); }
	void setSize(int w, int h) { ::SDL_SetWindowSize(window_, w, h); }
	void setTitle(const char *title) { ::SDL_SetWindowTitle(window_, title); }
	//int SDL_ShowMessageBox(const SDL_MessageBoxData* messageboxdata, int* buttonid);
	int showSimpleMessageBox(Uint32 flags, const char* title, const char* message) { return ::SDL_ShowSimpleMessageBox(flags, title, message, window_); }
	void show() { ::SDL_ShowWindow(window_); }
	//int updateSurface() { ::SDL_UpdateWindowSurface(window_); }
	//int updateSurfaceRects(const SDL_Rect* rects, int numrects) { ::SDL_UpdateWindowSurfaceRects(window_, rects, numrects); }
	SDL_::Renderer &getRenderer() { return renderer_; }

	void swap();

private:
	SDL_Window * const window_;
	SDL_GLContext const context_;
	SDL_::Renderer renderer_;
};

} // SDL_

#endif // SDLWINDOW_H_
