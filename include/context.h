#ifndef context_H
#define context_H

#define CONTEXT_NULL		0
#define CONTEXT_START_MENU	1	// スタートメニュー
#define CONTEXT_FIELD		2	// フィールド
#define CONTEXT_TOWER		3	// タワー内部
#define CONTEXT_BATTLE		4	// 戦闘
#define CONTEXT_BOSS		5	// ボスステージ
#define CONTEXT_SHOP		6	// ショップ
#define CONTEXT_EQUIPMENT	9	// 装備
#define CONTEXT_INVENTORY	10	// 在庫表示画面
#define CONTEXT_ENTER_CHARACTER	13	// 文字入力
#define CONTEXT_ENTER_NUMBER	14	// 数値入力
#define CONTEXT_ENTER_STRING	15	// 文字列入力
#define CONTEXT_ENDING		19	// エンディング
#define MAX_CONTEXT		20	// コンテキストの数

#define CONTEXT_RESUME		-1	// 直前のコンテキストを復帰

extern void switch_context(int context_id);
extern void extend_context(int context_id);
extern void resume_context(void);
extern void reset_context(void);
extern int current_context_id(void);
extern void finish_current_context(void); // 現在のSceneをfinish()する(XanaduPauseの完了コールバック用)

#endif // context_H