#if !defined(MESSAGE_ENTER_SCENE_H_)
#define MESSAGE_ENTER_SCENE_H_

#include "scene/GameScene.h"

// CONTEXT_ENTER_CHARACTER / CONTEXT_ENTER_NUMBER / CONTEXT_ENTER_STRING の3つは
// いずれも旧message_enter_enter/message_enter_leaveを共有しており、
// 実際の入力モードの違いはcurrent_context_id()を見て message_key() 側で
// 判定される(src/message.cpp参照)。そのため個別のクラスに分けず、
// contextIdをコンストラクタで受け取る1クラスとして実装する。
class MessageEnterScene final : public GameScene
{
public:
	explicit MessageEnterScene(int contextId);

protected:
	void onEnter() override;
	void onLeave() override;
};

#endif // MESSAGE_ENTER_SCENE_H_
