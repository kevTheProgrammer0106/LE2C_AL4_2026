#pragma once

#define NOMINMAX

#include "Global.h"
#include <KamataEngine.h>
#include <cstdint>
#include <memory>
#include <vector>

class BaseEnemy;

/// <summary>
/// アリーナ戦闘中のみ表示する敵位置インジケーターHUD。
/// - アリーナが非アクティブなら常に非表示。
/// - アリーナ内の残り敵数が少なくなった時(既定: 5体未満)は頭上アイコン/オフスクリーン矢印を表示。
/// [FIX] ウェーブ間の残り撃破数アイコン列(いわゆる「アリーナUI」)は削除。
/// 通常の敵位置インジケーターのみを保持する。
/// </summary>
class EnemyHUD {
public:
	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void Initialize(uint32_t arrowTextureHandle, uint32_t fallbackIconTextureHandle);

	// arenaEnemies: 現在アクティブなアリーナに属する敵のみ(GameScene側でarenaIDでフィルタして渡す)。
	// アリーナが非アクティブなら空のvectorを渡すことでHUD自体が完全に消える。
	void Update(const std::vector<BaseEnemy*>& arenaEnemies, const KamataEngine::Camera& camera, int screenWidth, int screenHeight);
	void Draw();

private:
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
	static inline float kHeadOffsetY = 1.2f;
	static inline float kScreenEdgeMargin = 32.0f;

	static inline int kEnemyCountThreshold = 5;
};