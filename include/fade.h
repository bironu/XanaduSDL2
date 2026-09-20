#ifndef fade_H
#define fade_H

#include "graphics.h"

// 市松状にfade_R/G/Bでマスクした色へ寄せていくフェード演出。
// XanaduFadeはタイマーも画面反映(swap等)も一切持たない。呼び出し側が
// 自分のタイマーからstep()を1フレームぶんずつ呼び出し、呼び出し側自身の
// 責任で描画結果を画面へ反映する。
class XanaduFade
{
public:
	XanaduFade();

	// フェードを開始する。dstの(x, y)にimgを合成しながら、
	// 市松状にrgb(0xRRGGBB)へ寄せていく準備をする。
	void start(std::shared_ptr<SDL_::Image> dst, int x, int y,
	           std::shared_ptr<SDL_::Image> img, unsigned rgb);

	// 呼び出し側のタイマーから1フレームぶんだけ呼ぶ。
	// まだ続きがあればtrue、この呼び出しで完了していればfalseを返す。
	bool step();

	bool isDone() const { return step_ >= kSteps; }

private:
	static constexpr int kSteps = 11;

	std::shared_ptr<SDL_::Image> dst_;
	int x_;
	int y_;
	// imgをdst_と同じピクセルフォーマットへ変換したもの。
	// 生ピクセル比較のためフォーマットを揃える必要がある
	std::shared_ptr<SDL_::Image> source_;
	unsigned r_;
	unsigned g_;
	unsigned b_;
	int step_;
};

#endif // fade_H
