#if !defined(BATTLESCENE_H_)
#define BATTLESCENE_H_

#include "scene/GameScene.h"
#include "battle.h"

// 旧src/battle.cppのロジック本体。戦闘の生データ(部屋/マップ/モンスター/魔法)は
// BattleScene生成前後にも直接触られるためBattleState(include/battle/BattleState.h)
// に置き、ここには「戦闘中の入力処理・メインループ・演出」だけを持たせる
class BattleScene final : public GameScene
{
public:
	BattleScene();

protected:
	void onCreate(uint32_t tick) override;
	void onDestroy(uint32_t tick) override;
	void onEnter() override;
	void onLeave() override;

private:
	// ---- 定数(旧#define) ----
	static constexpr int kBattleInterval = 70;       // 戦闘時インターバル
	static constexpr int kStatusMonsterHpLine = 8;
	static constexpr int kStepUser = 8;
	static constexpr int kStepMonster = 8;
	static constexpr int kStepMagic = 16;
	static constexpr int kUpdateAbort = -1;
	static constexpr int kPhantomTime = 20;
	static constexpr int kDeadTime = 1;
	static constexpr int kSkillMax[5] = { 255, 255, 200, 200, 255 }; // 熟練度の最大値

	// モンスターの状態(MONSTER_*)はbattle.hで定義(BattleStateも使うため)

	// Deg系魔法の状態(旧DEG_PHASE_*)
	static constexpr int kDegPhaseInactive  = 0;
	static constexpr int kDegPhaseHit       = 1;
	static constexpr int kDegPhaseDead      = 2;
	static constexpr int kDegPhaseDisappear = 3;

	// ---- ヘルパー(旧マクロ関数) ----
	bool userGetSkillfull() const;             // 熟練度が上がる？
	void userSkillUp(int type, int n);          // 熟練度アップ
	static int userError(int p);                // 誤差: 30/32..38/32
	static int monsterError(int p);              // 誤差: 12/16..20/16
	int monsterTime() const;                     // モンスター行動間隔

	// ---- モンスター行動パターンテーブル ----
	// BattleState::activityPatternと同じインデックスを共有する
	// (sensitive/isCasterはBattleState側が持つ)
	struct ActivityProc {
		int (BattleScene::*moveProc)();
		int (BattleScene::*changeDir)(const monster_t *);
	};
	static const ActivityProc kActivityTable[13];

	// ---- 戦場の情報 ----
	member_t memberWall_ = { MEMBER_UNUSED, 0, 0, 0, 0 };
	member_t memberLock_ = { MEMBER_UNUSED, 0, 0, 0, 0 };

	// ---- その他のループで使う状態 ----
	member_t *userAttack_ = nullptr; // ユーザー「が」攻撃した何か
	void *userDamage_ = nullptr;     // ユーザー「を」攻撃した何か
	int userDegPhase_ = kDegPhaseInactive; // ユーザーがDeg系魔法を唱えた？
	int killedUserMagic_ = 0;        // ユーザーの魔法がモンスターを倒した？
	int keystateShift_ = 0;          // Shiftキーの状態
	bool needEncountSound_ = false;  // onCreate()で立てる。遭遇音の待機を一度だけ行う
	int mirrorWait_ = 0;             // battleLoop()の呼び出し間で保持する状態
	int magicLoopTimer_ = 0;         // battleLoopMagic()の呼び出し間で保持する状態

	// ---- メインループ ----
	void battleLoop();
	void battleLoopAttack();
	void battleLoopMagic();
	void updateBackground();
	void battleEnterContinue();

	// ---- ユーザー移動・入力 ----
	int battleMoveUser(int dir);
	int battleControlUserMagic(int dir);
	int userAttemptEscape(int dir);
	int userAttemptAttack(member_t *um);
	member_t *battleCanThrough(int x1, int y1, int x2, int y2);
	member_t *battleHitTest(int x, int y);

	// ---- モンスターAI ----
	int monstersMoveWalker();
	int monstersMoveRooted();
	int monstersMoveVision();
	member_t *monsterCanThrough(int x1, int y1, int dir);
	int monsterCanAppear(member_t *mm);
	int monsterCaptureUser(monster_t *mo);
	int changeDirChaser(const monster_t *mo);
	int changeDirDancer(const monster_t *mo);
	int changeDirNeutral(const monster_t *mo);
	int changeDirEscapee(const monster_t *mo);

	// ---- 魔法 ----
	int battleMoveMagic();
	member_t *moveMagic(magic_t *ma);
	int magicAttackMonster(member_t *mm, magic_t *ma, int deg);
	void magicAttackUser(magic_t *ma);
	void battleCastSpell(magic_t *ma, int scrollId, int INT, int x, int y, int dir);

	// ---- 攻撃 ----
	static int whichDirection(int x1, int y1, int x2, int y2, int d2);
	int decrementMonsterHP(member_t *mm, int howMany, int byMagic);
	int decrementUserHP(int howMany, int noEcho);
	void battleMonstersDestroyed();
	void battleAttackMonster(member_t *mm);
	void battleAttackUser(member_t *mm);

	// ---- 宝箱・お宝 ----
	void battleGetGoods(member_t *gm);
	void battleOpenBox(member_t *um);
};

#endif // BATTLESCENE_H_
