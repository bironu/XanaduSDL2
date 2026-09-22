#ifndef cave_H
#define cave_H

#include <functional>

// 洞窟(レベル間の縦穴)に入っていく演出を再生し、指定レベルへ移動する。
// 画面遷移を伴わないため専用Sceneには依存しない。完了後、呼び出し元の
// ゲームループを再開するためonResume(field_enter等)を呼ぶ
void play_cave(int to_level, std::function<void()> onResume);

#endif // cave_H
