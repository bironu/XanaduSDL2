#if !defined(OPENINGSCENE_H_)
#define OPENINGSCENE_H_

#include "scene/Scene.h"

// 旧C実装(src/opening.cpp)のstatic変数・自由関数群をクラスのメンバに移設したもの。
// フェード演出中の連射キー確認はget_keystate()による共有状態の参照だけで足り、
// 最終ステップの「キー待ち」もこのクラス自身のdispatch()で完結するため、
// MenuSceneと同様にGameScene(旧C実装の共通コンテキスト基底)は継承せず、
// Sceneを直接継承してonCreate/onSuspend/onResumeを自前で実装する。
class OpeningScene final : public Scene
{
public:
	OpeningScene();

	void dispatch(const SDL_Event &event) override;
	void onSuspend() override;
	void onCreate(uint32_t tick) override;
	void onResume(uint32_t tick) override;

private:
	// 旧opening_enter/wait_forever/restore_contextに対応
	void onEnter();
	void waitForever();
	void restoreContext();

	int step_;
	unsigned fadeMask_;
	// wait_forever()到達後、キー入力を待っている間だけtrueになる
	// (旧実装のthunk_key_event = opening_key_eventに対応)
	bool waitingForKey_;
};

#endif // OPENINGSCENE_H_
