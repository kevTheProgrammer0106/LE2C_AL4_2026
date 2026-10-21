#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>
#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

class BaseEnemy;
class Player;

/// <summary>
/// 敵位置インジケーター(旧EnemyHUD)+ 各キャラの状態アイコン。
/// [既存] Update/Draw: アリーナ戦闘中、残り敵が少ない時の頭上アイコン/オフスクリーン矢印。
/// [追加] UpdateStatus/DrawStatus: 体の中心に出す状態アイコン。
///   - standard: slowed < buffed < knockback < stunned のうち一番強い1つだけを表示。変わる時はアルファをイージングで切り替える。
///   - reactive: 公式バフ / リロード(敵のみ) / ブロックが「始まった瞬間」に光って拡大 -> 元のサイズへイージング(1.5秒)
///               -> そのまま2.5秒表示 -> 消える。
/// </summary>
class StatusIndicator {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(uint32_t arrowTextureHandle, uint32_t fallbackIconTextureHandle);

	// arenaEnemies: 現在アクティブなアリーナに属する敵のみ(GameScene側でarenaIDでフィルタして渡す)。
	void Update(const std::vector<BaseEnemy*>& arenaEnemies, const KamataEngine::Camera& camera, int screenWidth, int screenHeight);
	void Draw();

	// ---- 状態アイコン ----
	// 画像が無い時の代わりに使う白テクスチャを渡す(例: assets_.healthBarBackgroundTex)
	void InitializeStatus(uint32_t whiteTextureHandle);
	// player は nullptr 可。enemies は GameScene の全敵リスト
	void UpdateStatus(Player* player, const std::vector<std::unique_ptr<BaseEnemy>>& enemies, const KamataEngine::Camera& camera, int screenWidth, int screenHeight);
	void DrawStatus();
	// プレイヤーがチェックポイントに到達した時にGameSceneから呼ぶ(次のUpdateStatusで演出が始まる)
	void NotifyCheckpoint() { pendingCheckpoint_ = true; }


private:
	// ================= 既存: 位置インジケーター =================
	struct IndicatorVisual {
		std::unique_ptr<KamataEngine::Sprite> icon;
		std::unique_ptr<KamataEngine::Sprite> arrow;
		bool inUse = false;
	};

	IndicatorVisual& AcquireVisual(size_t index, uint32_t iconTexture);
	void ReleaseUnusedVisuals(size_t activeCount);

	std::vector<IndicatorVisual> visuals_;

	uint32_t arrowTextureHandle_ = 0;
	uint32_t fallbackIconTextureHandle_ = 0;
	bool isActive_ = false;

	static inline int kIconSize = 28;
	static inline int kArrowSize = 24;
	static inline float kHeadOffsetY = -1.2f;
	static inline float kScreenEdgeMargin = 32.0f;

	static inline int kEnemyCountThreshold = 5;

	// ================= 追加: 状態アイコン =================
	// 数字が大きいほど優先(slowed < buffed < knockback < stunned)
	enum StandardStatus : int { kStatusNone = 0, kStatusSlowed, kStatusBuffed, kStatusKnockback, kStatusStunned, kStatusCount };
	enum ReactiveKind : int {
		kReactiveBuff = 0,
		kReactiveReload,
		kReactiveBlock,
		kReactiveAlert,      // 敵: プレイヤーを発見した
		kReactiveMissing,    // 敵: プレイヤーを見失った
		kReactiveCheckpoint, // プレイヤー: チェックポイントに到達した
		kReactiveHealing,    // プレイヤー: 回復した
		kReactiveCount
	};

	// 1フレーム分の状態のスナップショット
	struct Flags {
		bool slowed = false;
		bool buffed = false;
		bool knockback = false;
		bool stunned = false;
		std::array<bool, kReactiveCount> reactive{};
		std::array<bool, kReactiveCount> trigger{}; // 条件ではなく「今フレームに起きた出来事」(立ち上がり検出なしで即再生)
	};

	struct ReactiveState {
		bool active = false;
		bool prevCondition = false; // 立ち上がり検出用
		float timer = 0.0f;
		float cooldown = 0.0f; // 連続再生を防ぐ待ち時間(発見/見失いのちらつき対策)
	};

	struct EntityVisual {
		// standard
		int shown = kStatusNone; // いま見えている(フェード中を含む)状態
		float alpha = 0.0f;
		std::array<std::unique_ptr<KamataEngine::Sprite>, kStatusCount> standardSprites; // 状態ごとに専用スプライト(テクスチャは生成時に固定)
		// reactive
		std::array<ReactiveState, kReactiveCount> reactive;
		std::array<std::unique_ptr<KamataEngine::Sprite>, kReactiveCount> reactiveSprites;
		std::array<bool, kReactiveCount> reactiveVisible{};
		// 共通
		bool onScreen = false;
		bool seen = false;
	};

	static Flags GatherFlags(const BaseEnemy& enemy);
	Flags GatherFlags(Player& player);
	void UpdateEntity(const void* key, const Flags& flags, const AABB& aabb, const KamataEngine::Matrix4x4& matVPV, int screenWidth, int screenHeight);
	bool LoadIfExists(const char* fileName, uint32_t& outTex);

	std::unordered_map<const void*, EntityVisual> entityVisuals_;

	uint32_t whiteTex_ = 0;
	bool pendingCheckpoint_ = false;
	int lastPlayerHp_ = -1; // -1=未取得。HPが増えたら回復演出
	std::array<uint32_t, kStatusCount> standardTex_{};
	std::array<bool, kStatusCount> standardHasTex_{};
	std::array<uint32_t, kReactiveCount> reactiveTex_{};
	std::array<bool, kReactiveCount> reactiveHasTex_{};

	// ---- 調整値(GlobalVariables "EnemyHUD" グループ) ----
	static inline int kStatusIconSize = 36;             // standardアイコンのサイズ(px)
	static inline float kStatusFadeRate = 0.2f;         // 毎フレームのアルファ追従率(イージング)
	static inline int kReactiveIconSize = 40;           // reactiveアイコンの基本サイズ(px)
	static inline float kReactivePopScale = 5.0f;       // 出た瞬間の拡大率
	static inline float kReactivePopDuration = 1.0f;    // 拡大 -> 元サイズまでの時間(秒)
	static inline float kReactiveHoldDuration = 0.1f;   // 元サイズのまま表示する時間(秒)
	static inline float kReactiveFadeDuration = 0.3f;   // 消えるまでの時間(秒)
	static inline float kReactiveFlashFrequency = 8.0f; // 光る(点滅)速さ
	static inline float kReactiveOffsetY = -15.0f;      // 体の中心からの縦オフセット(px、負=上)
	static inline float kReactiveSpacing = 44.0f;       // 複数同時に出る時の横間隔(px)
	static inline float kAlertRetriggerCooldown = 1.0f; // 発見/見失いの再発火までの最短間隔(秒)
	static inline float kAlertOffsetY = -76.0f;         // Alert/Missing は他のreactiveより上の段に出す(px、負=上)
};