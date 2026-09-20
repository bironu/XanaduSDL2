#if !defined(ENDINGSCENE_H_)
#define ENDINGSCENE_H_

#include "scene/Scene.h"
#include "sdl/SDLImage.h"
#include "geo/Rect.h"
#include "fade.h"
#include <memory>
#include <vector>

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
	// フェード(scenario2)/即黒背景(scenario1)のどちらの後でも呼ばれる、
	// クレジットロール開始処理
	void startScroll();

	// フェード演出(XanaduFade)をこのScene自身のタイマーで駆動する
	void startFade(std::shared_ptr<SDL_::Image> dst, int x, int y,
	               std::shared_ptr<SDL_::Image> img, unsigned rgb);
	// フェード中/スクロール中共通のタイマー処理(fade_の状態でどちらかへ振り分ける)
	void onTimer();
	// Application::setTimer()を起動し、以後SDL_ENDING_TIMER_EVENT経由でonTimer()を呼ぶ
	void startTimer(uint32_t interval);

	// clipOverall_(+スクロール中はclipEndingRoll_)をバックバッファへ反映する
	void onDraw();

	bool init();
	int loadKanjiCode(const char *filename);
	int drawKanjiText(std::shared_ptr<SDL_::Image> img, int x, int y, const int *code) const;

	static constexpr int kEndingInterval = 80;
	// フェードの1ステップと同じ間隔(旧FADE_INTERVAL)
	static constexpr uint32_t kFadeInterval = 50;
	static const uint32_t SDL_ENDING_TIMER_EVENT;

	XanaduFade fade_;

	static constexpr int kKanjiWidth = 23;    // 漢字フォントの幅
	static constexpr int kKanjiHeight = 23;   // 漢字フォントの高さ
	static constexpr int kKanjiPageCol = 16;  // 漢字フォントの列数
	static constexpr int kKanjiPageRow = 24;  // 漢字フォントの行数

	static constexpr int kMessageBufferSize = 8024;

	static const Rect kRectOverall;
	static const Rect kRectEndingRoll;

	// 旧kanji_baseの非nullptr判定(初回のフェード演出が済んだかどうか)に対応
	bool initialized_;
	// wait_forever()到達後、キー入力を待っている間だけtrue
	// (旧実装のthunk_key_event = ending_key_eventに対応)
	bool waitingForKey_;
	// クレジットロール(clipEndingRoll_)をclipOverall_へ合成するかどうか。
	// 旧setLegacyEndingRollCompositingEnabled(グローバルフラグ)に対応するが、
	// このScene自身の状態として持つ。
	bool rollActive_;

	// 旧clip_overall/clip_endingroll/visual_image(いずれもLegacyPlatform.cppの
	// グローバル)をこのScene自身のメンバとして持つ。onCreate()で確保し、
	// onDestroy()で破棄する。画面反映は自前のonDraw()(backBuffer_+swap())で行い、
	// presentLegacyFrame()には依存しない。
	std::shared_ptr<SDL_::Image> clipOverall_;
	std::shared_ptr<SDL_::Image> clipEndingRoll_;
	std::shared_ptr<SDL_::Image> visualImage_;

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
