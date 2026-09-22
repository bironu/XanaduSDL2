#ifndef use_item_H
#define use_item_H

#include "dungeon.h"
#include <functional>

// 現在装備している魔法アイテムを即座に使用する。画面遷移を伴わないため
// 専用Sceneには依存しない。処理完了後、呼び出し元のゲームループを
// 再開するためonResume(battle_enter/field_enter等)を呼ぶ
void play_use_item(std::function<void()> update_background, room_t *room,
                   std::function<void()> onResume);

#endif // use_item_H
