#if !defined(MENUSCENE_H_)
#define MENUSCENE_H_

#include "scene/Scene.h"
#include "geo/FRect.h"
#include <array>
#include <memory>

namespace SDL_
{
class Image;
class Color;
}

struct SDL_KeyboardEvent;

class MenuScene final : public Scene
{
public:
	MenuScene();

	void dispatch(const SDL_Event &event) override;
	void onSuspend() override;
	void onCreate(uint32_t tick) override;
	void onDestroy(uint32_t tick) override;
	void onResume(uint32_t tick) override;

private:
	// 旧*_enter関数、旧*_leave関数に対応
	void onEnter();
	void onLeave();

    void onKeyDown(const SDL_KeyboardEvent &key);
    void onWindowExpose(const SDL_WindowEvent &window);

	enum class State { Generic, Version, Load, Debug, Boss, Game };

	void drawText(int row, int col, const char *s, const SDL_::Color &pixel);
	void drawItem(int row, int col, int key, const char *s, const SDL_::Color &pixel);

	void onGenericKey(const SDL_KeyboardEvent &key);
	void onLoadKey(const SDL_KeyboardEvent &key);
	void onDebugKey(const SDL_KeyboardEvent &key);
	void onBossKey(const SDL_KeyboardEvent &key);
	void onVersionKey(const SDL_KeyboardEvent &key);

	void initDebug();
	int initLoadMenu();
	int loadGame(int index);
	bool loadUser();

    std::shared_ptr<SDL_::Image> clip_main_;
    std::shared_ptr<SDL_::Image> clip_overall_;
    static const FRect rect_overall_;
    static const FRect rect_main_;

	// 旧menu.cppのstatic変数と同様、インスタンスをまたいで状態を保持する
	// 必要がある(例: 'N'で menu_state = MENU_GAME にした後、opening scene
	// 経由で一度破棄・再構築されたMenuSceneに戻ってきてもGAME状態を継続する)ため
	// static membersとする。
	static constexpr int kMaxUserEntry = 8;
	static State state_;
	static std::array<std::string, kMaxUserEntry> userEntries_;
};

#endif // MENUSCENE_H_
