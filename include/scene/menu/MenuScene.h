#if !defined(MENUSCENE_H_)
#define MENUSCENE_H_

#include "scene/Scene.h"
#include <array>
#include <memory>

namespace SDL_
{
class Image;
class Color;
}

struct SDL_Keysym;

// 旧C実装(src/menu.cpp)のstatic変数・自由関数群をクラスのメンバに移設したもの。
// キー入力はthunk_key_event/get_keystate()のどちらも使わず、dispatch()が
// SDL_Eventから直接state_に応じたon*Keyメンバへ振り分ける。そのため、他の
// *Scene群と違い、GameScene(旧C実装の共通コンテキスト基底)は継承せず、
// Sceneを直接継承してonCreate/onDestroy/onResume/onSuspendを自前で実装する。
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

	enum class State { Generic, Version, Load, Debug, Boss, Game };

	struct UserEntry
	{
		char name[16];
	};

	static constexpr int kMaxUserEntry = 8;

	void drawText(int row, int col, const char *s, const SDL_::Color &pixel);
	void drawItem(int row, int col, int key, const char *s, const SDL_::Color &pixel);

	void onGenericKey(const SDL_Keysym &keysym);
	void onLoadKey(const SDL_Keysym &keysym);
	void onDebugKey(const SDL_Keysym &keysym);
	void onBossKey(const SDL_Keysym &keysym);
	void onVersionKey(const SDL_Keysym &keysym);

	void initDebug();
	int initLoadMenu();
	int loadGame(int index);

	// 旧menu.cppのstatic変数と同様、インスタンスをまたいで状態を保持する
	// 必要がある(例: 'N'で menu_state = MENU_GAME にした後、opening scene
	// 経由で一度破棄・再構築されたMenuSceneに戻ってきてもGAME状態を継続する)ため
	// static membersとする。
	static State state_;
	static std::array<UserEntry, kMaxUserEntry> userEntries_;
	std::shared_ptr<SDL_::Image> imageLogo_;
	std::shared_ptr<SDL_::Image> imageFrame_;
};

#endif // MENUSCENE_H_
