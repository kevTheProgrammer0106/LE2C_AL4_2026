#pragma once

#include "BaseEnemyBehavior.h"
#include "WeaponSabre.h"

class EnemyRifleman;

/// <summary>
/// EnemyRifleman専用のBehaviorモジュール。共通のIdle/Roam/Root/Deathは基底クラスに任せ、
/// ここでは「発見→75/25で射撃or銃剣突撃を選択（近距離なら必ず銃剣）→(射撃なら)狙う→撃つ→
/// 後退しながらリロード→追跡へ戻る」の一連の交戦サイクルを実装する。
///
/// 銃剣突撃(kMelee)のランジ攻撃は、WeaponSabreのコンボ3段目(index 2, 大振りの叩きつけ)が
/// 持つ間合い・速度・各フェーズの長さをそのまま流用する。専用の数値を新たに持たず、
/// WeaponSabreのstatic値を問い合わせるためだけのダミーインスタンス(sabreComboReference_)を使う。
/// </summary>
class RiflemanBehavior final : public BaseEnemyBehavior {
public:
	explicit RiflemanBehavior(EnemyRifleman& owner);

	// 射撃準備中・リロード中・銃剣突撃中はkRootへ割り込まれないようにする
	bool IsEngaged() const override {
		Behavior b = GetBehavior();
		return b == Behavior::kAiming || b == Behavior::kReloading || b == Behavior::kMelee;
	}

	void InitializeBehaviorState(Behavior behavior) override;
	void UpdateBehaviorState() override;


		bool IsInAttackAnticipation() const override { return GetBehavior() == Behavior::kMelee && meleePhase_ == MeleePhase::kAnticipate; }

protected:
	// プレイヤーを発見しての追跡中(kRoot)、交戦距離まで詰まったら
	// 射撃/銃剣のどちらで交戦するかをここで判定してリクエストする。
	void BehaviorRootInitialize() override;
	void BehaviorRootUpdate() override;

private:
	enum class MeleePhase { kAnticipate, kLunge, kRecovery };
	bool lungeFacingLocked_ = false;

	void BehaviorAimingInitialize();
	void BehaviorAimingUpdate();

	void BehaviorReloadingInitialize();
	void BehaviorReloadingUpdate();

	void BehaviorMeleeInitialize();
	void BehaviorMeleeUpdate();

	EnemyRifleman& riflemanOwner_;

	// --- kAiming: 狙いを定めてから発砲するまで ---
	float aimTimer_ = 0.0f;
	static inline float kAimDuration = 1.2f; // 狙いを定めてから発砲するまでの秒数

	// --- kReloading: 後退しながらリロード（実際の弾薬回復はBaseFirearm側が自動で行う） ---
	float retreatTimer_ = 0.0f;
	static inline float kRetreatSpeed = 0.03f;
	static inline float kMaxRetreatDuration = 4.0f; // リロードが終わらなくても諦めて追跡へ戻るまでの秒数

	// --- kMelee: 銃剣突撃(ランジ) ---
	MeleePhase meleePhase_ = MeleePhase::kAnticipate;
	uint32_t meleeParameter_ = 0;
	// WeaponSabreのコンボ間合い/速度/フェーズ長を問い合わせるためだけのダミー(Initialize/Drawはしない)
	WeaponSabre sabreComboReference_;
	static constexpr int kBayonetComboIndex = 2; // 「コンボ3段目」＝大振りの叩きつけをランジに流用

	// --- 交戦距離・射撃/銃剣の選択確率 ---
	static inline float kMeleeEngageRange = 2.0f;    // これより近ければ必ず銃剣突撃
	static inline float kFiringRange = 6.0f;         // これより近ければ交戦判定を行う
	static inline float kRangedAttackChance = 0.75f; // 交戦判定時に射撃を選ぶ確率(残りは銃剣)
};