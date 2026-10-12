#pragma once

#include <KamataEngine.h>
#include "Global.h"
#include <memory>

/// <summary>
/// 画面全体の色フラッシュ(仮: Fadeと同じ white1x1 の全画面スプライト)。
/// 一瞬のフラッシュ(被弾=赤 / 回復=緑 / ブロック成功=白)と、
/// 状態に応じて出続ける色(低HP=赤 / スタン=黄)を重ねて表示する。
/// </summary>
class ScreenFlash {
public:
	enum class Type { kDamage, kHeal, kBlock };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
	static int GetLowHpThreshold() { return kLowHpThreshold; }

	void Initialize(uint32_t whiteTextureHandle);
	void Update();
	void Draw();

	// 一瞬のフラッシュを出す(最大の不透明度から薄れていく)。直前のフラッシュは上書きする
	void Trigger(Type type);
	// 状態に応じて出続ける色(毎フレーム GameScene から渡す)
	void SetStunned(bool stunned) { stunned_ = stunned; }
	void SetLowHealth(bool lowHealth) { lowHealth_ = lowHealth; }

private:
	std::unique_ptr<KamataEngine::Sprite> sprite_;

	// ---- 一瞬のフラッシュ ----
	Type flashType_ = Type::kDamage;
	float flashTimer_ = 0.0f; // 残り秒数(0で無し)

	// ---- 出続ける色 ----
	bool stunned_ = false;
	bool lowHealth_ = false;
	float persistentAlpha_ = 0.0f;
	KamataEngine::Vector3 persistentColor_ = {1.0f, 0.0f, 0.0f};

	// ---- 調整値 ----
	static inline float kMaxAlpha = 0.8f;       // フラッシュの最大の不透明度
	static inline float kFlashDuration = 0.35f; // フラッシュが消えるまでの秒数
	static inline float kLowHpAlpha = 0.3f;     // 低HP中に出続ける赤の不透明度
	static inline float kStunAlpha = 0.4f;      // スタン中に出続ける黄色の不透明度
	static inline float kPersistentFade = 0.1f; // 出続ける色の出入りの速さ(毎フレームの補間率)
	static inline int kLowHpThreshold = 30;     // このHP以下で赤が出続ける
	static inline KamataEngine::Vector3 kDamageColor = {1.0f, 0.0f, 0.0f};
	static inline KamataEngine::Vector3 kHealColor = {0.2f, 1.0f, 0.3f};
	static inline KamataEngine::Vector3 kBlockColor = {1.0f, 1.0f, 1.0f};
	static inline KamataEngine::Vector3 kStunColor = {1.0f, 0.9f, 0.1f};
};