#include "scene/ending/EndingScene.h"
#include "xanadu.h"
#include "fade.h"
#include "context.h"
#include "sdl/LegacyPlatform.h"

#include <SDL3/SDL_events.h>
#include <cstdio>

EndingScene *EndingScene::active_ = nullptr;

EndingScene::EndingScene()
	: initialized_(false)
	, waitingForKey_(false)
	, kanjiCodeTop_(nullptr)
	, kanjiCodeEnd_(nullptr)
	, msgY_(0)
	, msgRestRows_(0)
{
}

void EndingScene::dispatch(const SDL_Event &event)
{
	if (event.type != SDL_EVENT_KEY_DOWN || event.key.repeat != 0) {
		return;
	}

	if (waitingForKey_) {
		// スクロール終了後は何を押してもメニューへ戻る
		restoreContext();
		return;
	}

	// スクロール中断(旧ending_loopのget_keystate(VK_CONTROL) && get_keystate('Q')に
	// 対応)。keystate_vectorはGameScene::dispatch()経由でしか更新されず、
	// GameSceneを継承しないこのクラスでは常に古いままになるため、ここで
	// SDL_Eventから直接Ctrl+Qを判定する。
	if ((event.key.mod & SDL_KMOD_CTRL) != 0 && event.key.key == SDLK_Q) {
		restoreContext();
	}
}

void EndingScene::onCreate(uint32_t /*tick*/)
{
	initialized_ = false;
	waitingForKey_ = false;
	kanjiBase_ = nullptr;
	msg_ = nullptr;
	kanjiCode_.clear();
	kanjiCodeTop_ = nullptr;
	kanjiCodeEnd_ = nullptr;
	msgY_ = 0;
	msgRestRows_ = 0;

	active_ = this;

	// clip_main等(メニュー画面の文字列など)がclip_overallへ合成されると、
	// このシーンが画面全体やclip_endingrollへ直接描く演出の上に被さってしまうため
	// 止めておく。resume_context()で本当にこのシーンが終わる時(onSuspend())に戻す。
	setLegacyPanelCompositingEnabled(false);
	// clip_endingroll自身はスクロール開始直前(onEnter()の2回目)にtrueにする
	setLegacyEndingRollCompositingEnabled(false);
}

void EndingScene::onDestroy(uint32_t /*tick*/)
{
	active_ = nullptr;
}

void EndingScene::onResume(uint32_t /*tick*/)
{
	onEnter();
}

void EndingScene::onSuspend()
{
	kill_timer();

	// FadeSceneへ一時的に処理を譲るだけ(演出がまだ続く)ならfalseのままにし、
	// このシーン自体が本当に終わる(isFinished())時にだけ合成を元へ戻す
	if (isFinished()) {
		setLegacyPanelCompositingEnabled(true);
	}
}

void EndingScene::onEnter()
{
	if (!initialized_) {
		if (!init()) {
			beep();
			restoreContext();
			return;
		}
		initialized_ = true;

		if (!in_scenario2()) {
			// スクリーンのコピーを作成
			visual_image = create_image(clip_overall->getWidth(), clip_overall->getHeight());
			if (!visual_image) {
				restoreContext();
				return;
			}
			draw_image(visual_image, 0, 0, clip_overall);
			fill_image(clip_overall, 0, 0, clip_overall->getWidth(), clip_overall->getHeight(),
			           SDL_::Color::BLACK);

			// セピア色?
			extend_context(init_fade(clip_overall, 0, 0, visual_image, 0xff7f00));
		} else {
			visual_image = load_image(IMAGE_DIR "/xa2/ending/background.bmp");
			if (!visual_image) {
				restoreContext();
				return;
			}
			extend_context(init_fade(clip_overall, 0, 0, visual_image, 0xffffff));
		}
	} else {
		// もう一度スクリーンをコピー
		visual_image = create_image(clip_overall->getWidth(), clip_overall->getHeight());
		if (!visual_image) {
			restoreContext();
			return;
		}
		draw_image(visual_image, 0, 0, clip_overall);

		bgm_play(bgm_data.theme[user.environment.scenario].main);
		setLegacyEndingRollCompositingEnabled(true);
		set_timer(kEndingInterval, timerProc);
	}
}

void EndingScene::restoreContext()
{
	kanjiBase_ = nullptr;
	msg_ = nullptr;
	kanjiCode_.clear();
	kanjiCodeTop_ = nullptr;
	kanjiCodeEnd_ = nullptr;

	visual_image = nullptr;

	// スクロール中断時にclip_endingrollの最後の描画内容が残り続けないように、
	// バックグラウンドイメージを復元する前に合成を止めておく
	setLegacyEndingRollCompositingEnabled(false);

	// バックグラウンドイメージを復元
	load_background(IMAGE_DIR "/user/frame.bmp");
	update(rect_overall);

	resume_context();
}

bool EndingScene::init()
{
	char path[BUFSIZ];
	sprintf(path, IMAGE_DIR "/%s", !in_scenario2() ?
	        "xa1/ending/message.txt" : "xa2/ending/message.txt");

	// エンディングメッセージの読み込み
	if (loadKanjiCode(path)) {
		return false;
	}

	// 漢字フォントの読み込み
	kanjiBase_ = load_image(IMAGE_DIR "/picture/kanji.bmp");
	if (!kanjiBase_) {
		return false;
	}
	for (int i = 0; i < kKanjiPageRow; i++) {
		for (int j = 0; j < kKanjiPageCol; j++) {
			kanji_[i][j] = SDL_::SubImage{
				kanjiBase_, Rect(j * kKanjiWidth, i * kKanjiHeight, kKanjiWidth, kKanjiHeight)};
		}
	}

	// メッセージ用のオフスクリーンバッファ
	msg_ = create_image(clip_endingroll->getWidth(),
	                     clip_endingroll->getHeight() + kKanjiHeight);
	if (!msg_) {
		return false;
	}
	msg_->setColorKey(kanjiBase_->getColorKey());
	fill_image(msg_, 0, 0, msg_->getWidth(), msg_->getHeight(), SDL_::Color::BLACK);

	return true;
}

void EndingScene::timerProc()
{
	if (active_) {
		active_->loop();
	}
}

void EndingScene::loop()
{
	// 中断(Ctrl+Q)はdispatch()がSDL_Eventから直接判定するため、ここでは
	// get_keystate()による確認は行わない(旧keystate_vectorはGameSceneを
	// 継承しないこのクラスでは更新されない)。

	msgY_ += 1;

	// 新しい行が完全に現れた?
	if (msgY_ > kKanjiHeight) {
		scroll_image(msg_, -msgY_);
		msgY_ = 0;
		if (kanjiCodeTop_ < kanjiCodeEnd_) {
			// 最下行に描画
			const int n = drawKanjiText(msg_, 0, msg_->getHeight() - kKanjiHeight, kanjiCodeTop_);
			kanjiCodeTop_ += n;
		} else {
			msgRestRows_++;

			// 最後の行がスクリーンから見えなくなった?
			if (msgRestRows_ > msg_->getHeight() / kKanjiHeight) {
				waitForever();
			}
		}
	}

	draw_image(clip_endingroll, 0, 0,
	           SDL_::SubImage{visual_image, Rect(rect_endingroll.x, rect_endingroll.y,
	                                              rect_endingroll.width, rect_endingroll.height)});
	draw_sprite(clip_endingroll, 0, -msgY_, msg_);

	update(rect_endingroll);
}

void EndingScene::waitForever()
{
	kill_timer();
	waitingForKey_ = true;
	bgm_play(bgm_data.theme[user.environment.scenario].ending);
}

int EndingScene::loadKanjiCode(const char *filename)
{
	kanjiCode_.assign(kMessageBufferSize, 0);
	int *code = kanjiCode_.data();
	kanjiCodeTop_ = code;

	FILE *fp = fopen(filename, "rt");
	if (!fp) {
		perror(filename);
		return 1;
	}

	int i = 0;
	for (; i < kMessageBufferSize - 1; i++, code++) {
		if (fscanf(fp, "%d ", code) != 1) {
			break;
		}
	}
	*code = -1; // 終わりを示す
	kanjiCodeEnd_ = code;

	fclose(fp);
	return 0;
}

int EndingScene::drawKanjiText(std::shared_ptr<SDL_::Image> img, int x, int y, const int *code) const
{
	int n = 0;
	for (; *code >= 0; code++, n++) {
		const int row = *code / kKanjiPageCol;
		const int col = *code % kKanjiPageCol;
		if (row < kKanjiPageRow) {
			draw_image(img, x, y, kanji_[row][col]);
		}
		x += kKanjiWidth;
	}
	return n + 1;
}
