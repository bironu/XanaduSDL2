#ifndef user_dead_H
#define user_dead_H

#include <functional>

// ユーザー死亡演出(昏倒→墓標を表示してキー入力待ち→復活/リセット)を
// 即座に再生する。画面遷移を伴わないため専用Sceneには依存しない。
// 復活した場合、呼び出し元のゲームループを再開するためonRevive
// (battle_enter/field_enter等)を呼ぶ。リセットの場合はメニューへ
// 戻るため呼ばない
void play_user_dead(int x, int y, std::function<void()> update_background,
                    std::function<void()> onRevive);

#endif // user_dead_H
