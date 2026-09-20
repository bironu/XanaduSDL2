#if !defined(ENDINGSCENE_H_)
#define ENDINGSCENE_H_

#include "scene/Scene.h"
#include "sdl/SDLImage.h"
#include "fade.h"
#include <memory>
#include <vector>

// 旧C実装(src/ending.cpp)のstatic変数・自由関数群をクラスのメンバに移設したもの。
// スクロール終了後のキー入力確認はthunk_key_eventではなくこのクラス自身のdispatch()で
// 完結する。フェード演出はXanaduFade(fade.h参照)を使い、このScene自身のタイマーで
// 1ステップぶんずつ進める(フェード中もdispatch()/onIdle()が止まらないようにするため)。
// MenuScene/OpeningSceneと同様にGameScene(旧C実装の共通コンテキスト基底)は継承せず、
// Sceneを直接継承してonCreate/onDestroy/onResume/onSuspendを自前で実装する。
class EndingScene final : public Scene
{
public:
	EndingScene();

	void dispatch(const SDL_Event &event) override;
	void onSuspend() override;
	void onCreate(uint32_t tick) override;
	void onDestroy(uint32_t tick) override;
	void onResume(uint32_t tick) override;

private:
	// 旧ending_enter/restore_context/ending_loop/wait_foreverに対応
	void onEnter();
	void restoreContext();
	void loop();
	void waitForever();

	// フェード演出(XanaduFade)をこのScene自身のタイマーで駆動する
	void startFade(std::shared_ptr<SDL_::Image> dst, int x, int y,
	               std::shared_ptr<SDL_::Image> img, unsigned rgb);
	void onFadeTimer();

	bool init();
	int loadKanjiCode(const char *filename);
	int drawKanjiText(std::shared_ptr<SDL_::Image> img, int x, int y, const int *code) const;

	// 旧C実装のset_timer()はvoid(*)(void)という素の関数ポインタしか受け付けず、
	// メンバ関数を直接渡すことができない。そのため、実行中のインスタンスへ
	// 転送するだけの静的ブリッジを用意する(同時にアクティブなEndingSceneは
	// 常に高々1つという前提に依る)。
	static void timerProc();
	static EndingScene *active_;

	static constexpr int kEndingInterval = 80;
	// フェードの1ステップと同じ間隔(旧FADE_INTERVAL)
	static constexpr uint32_t kFadeInterval = 50;
	static const uint32_t SDL_ENDING_FADE_TIMER_EVENT;

	XanaduFade fade_;

	static constexpr int kKanjiWidth = 23;    // 漢字フォントの幅
	static constexpr int kKanjiHeight = 23;   // 漢字フォントの高さ
	static constexpr int kKanjiPageCol = 16;  // 漢字フォントの列数
	static constexpr int kKanjiPageRow = 24;  // 漢字フォントの行数

	static constexpr int kMessageBufferSize = 8024;

	// 旧kanji_baseの非nullptr判定(初回のフェード演出が済んだかどうか)に対応
	bool initialized_;
	// wait_forever()到達後、キー入力を待っている間だけtrue
	// (旧実装のthunk_key_event = ending_key_eventに対応)
	bool waitingForKey_;

	SDL_::SubImage kanji_[kKanjiPageRow][kKanjiPageCol];
	std::shared_ptr<SDL_::Image> kanjiBase_;
	std::shared_ptr<SDL_::Image> msg_;

	std::vector<int> kanjiCode_;
	int *kanjiCodeTop_;
	int *kanjiCodeEnd_;

	int msgY_;
	int msgRestRows_;
};

#endif // ENDINGSCENE_H_
