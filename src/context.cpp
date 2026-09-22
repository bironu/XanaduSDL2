#include "context.h"
#include "user.h"

#include "app/Application.h"
#include "scene/GameScene.h"

#include "scene/menu/MenuScene.h"
#include "scene/field/FieldScene.h"
#include "scene/tower/TowerScene.h"
#include "scene/battle/BattleScene.h"
#include "scene/boss/BossScene.h"
#include "scene/shop/ShopScene.h"
#include "scene/equip/EquipScene.h"
#include "scene/inventory/InventoryScene.h"
#include "scene/message/MessageEnterScene.h"
#include "scene/opening/OpeningScene.h"
#include "scene/ending/EndingScene.h"

#include <memory>

/*
 * 旧C実装ではコンテキストごとにenter_guard/leave_guardの関数ポインタを
 * 保持するテーブル(contexts[])を切り替えることで画面遷移を実現していたが、
 * 現在はCONTEXT_IDごとに対応するGameScene派生クラス(*Scene)をApplicationに
 * 生成させることで同じ役割を果たす。
 *
 *   switch_context(id)  : 現在のSceneをfinish()し、新しいSceneをそのまま置き換える
 *                          (旧: leave_guard() -> current差し替え -> enter_guard())
 *   extend_context(id)  : 現在のSceneを終了させずに新しいSceneへ進む
 *                          (Application::run()がonSuspend()でスタックへ積む)
 *   resume_context()    : 現在のSceneをfinish()するだけ
 *                          (Application::run()がスタックから前のSceneを復元しonResume()を呼ぶ)
 */

namespace {

std::shared_ptr<Scene> create_scene(int context_id)
{
  switch (context_id) {
  case CONTEXT_START_MENU:
    return std::make_shared<MenuScene>();
  case CONTEXT_FIELD:
    return std::make_shared<FieldScene>();
  case CONTEXT_TOWER:
    return std::make_shared<TowerScene>();
  case CONTEXT_BATTLE:
    return std::make_shared<BattleScene>();
  case CONTEXT_BOSS:
    return std::make_shared<BossScene>();
  case CONTEXT_SHOP:
    return std::make_shared<ShopScene>();
  case CONTEXT_EQUIPMENT:
    return std::make_shared<EquipScene>();
  case CONTEXT_INVENTORY:
    return std::make_shared<InventoryScene>();
  case CONTEXT_ENTER_CHARACTER:
  case CONTEXT_ENTER_NUMBER:
  case CONTEXT_ENTER_STRING:
    return std::make_shared<MessageEnterScene>(context_id);
  case CONTEXT_ENDING:
    return std::make_shared<EndingScene>();
  default:
    return nullptr;
  }
}

} // namespace

void switch_context(int context_id)
{
  Application &app = Application::instance();
  auto current = app.getCurrentScene();

  if (context_id == CONTEXT_RESUME) {
    // Application::run()がスタックの一つ前のSceneを自動的に復元する
    if (current) {
      current->finish();
    }
    return;
  }

  if (current) {
    current->finish();
  }
  app.registerNextScene(create_scene(context_id));
}

void extend_context(int context_id)
{
  // finish()しないことで、Application::run()が現在のSceneをonSuspend()経由で
  // スタックへ積んでから新しいSceneへ進む
  Application::instance().registerNextScene(create_scene(context_id));
}

void resume_context(void)
{
  switch_context(CONTEXT_RESUME);
}

// 初期状態に戻す
void reset_context(void)
{
  user_hidden = 0;
  // extend_context()で積まれたまま(死亡時のFieldScene/BattleScene等)の
  // 中断中Sceneが残っていると、後で無関係なresume_context()によって
  // 誤って復元されてしまう(スタック上のScene以下の状態は既に破棄・変化
  // している)。ここでメニューへ戻し切るので、スタックも空にしておく。
  Application::instance().clearResumeStack();
  switch_context(CONTEXT_START_MENU);
}

int current_context_id(void)
{
  auto current = Application::instance().getCurrentScene();
  auto game = std::dynamic_pointer_cast<GameScene>(current);
  return game ? game->contextId() : CONTEXT_NULL;
}

// XanaduPause(pause.cpp)の完了コールバックから呼ばれる。
// 旧switch_context(init_pause(...))が担っていた「ポーズ後に現在の
// Sceneを終了し、前のSceneへ戻る」効果をfinish()で直接再現する
void finish_current_context(void)
{
  auto current = Application::instance().getCurrentScene();
  if (current) {
    current->finish();
  }
}
