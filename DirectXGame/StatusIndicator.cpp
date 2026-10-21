#include "StatusIndicator.h"
#include "BaseEnemy.h"
#include "GlobalVariables.h"
#include "OfficerBuff.h"
#include "Player.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <numbers>
#include <string>

using namespace KamataEngine;

namespace {
// グループ名は旧EnemyHUDのまま(既存のjsonをそのまま使うため)
constexpr const char* kGroupName = "StatusIndicator";
// 画像の探す場所(上から順に試す)
constexpr const char* kStatusImageDirs[] = {"./Resources/UserInterface/", "./Resources/UserInterface/Status/"};

constexpr float kDeltaTime = 1.0f / 60.0f;

float EaseOutCubic(float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	const float u = 1.0f - t;
	return 1.0f - u * u * u;
}
// 状態ごとの色(テクスチャに乗算される。画像が無い時の仮色も兼ねる)
constexpr float kStandardTint[5][3] = {
    {1.0f, 1.0f,  1.0f}, // none
    {0.4f, 0.6f,  1.0f}, // slowed: 青
    {0.3f, 0.9f,  0.4f}, // buffed: 緑
    {1.0f, 0.6f,  0.2f}, // knockback: オレンジ
    {1.0f, 0.95f, 0.2f}, // stunned: 黄
};
constexpr float kReactiveTint[7][3] = {
    {0.3f, 0.9f,  0.4f }, // buff: 緑
    {1.0f, 0.6f,  0.2f }, // reload: オレンジ
    {0.4f, 0.8f,  1.0f }, // block: 水色
    {1.0f, 0.25f, 0.25f}, // alert: 赤
    {0.7f, 0.7f,  0.7f }, // missing: グレー
    {1.0f, 0.9f,  0.2f }, // checkpoint: 黄
    {1.0f, 0.5f,  0.7f }, // healing: ピンク
};
} // namespace

// =====================================================================
// 既存部分(EnemyHUDから名前だけ変更)
// =====================================================================
void StatusIndicator::RegisterGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	gv->CreateGroup(kGroupName);
	gv->AddItem(kGroupName, "EnemyCountThreshold", kEnemyCountThreshold);
	gv->AddItem(kGroupName, "IconSize", kIconSize);
	gv->AddItem(kGroupName, "ArrowSize", kArrowSize);
	gv->AddItem(kGroupName, "HeadOffsetY", kHeadOffsetY);
	gv->AddItem(kGroupName, "ScreenEdgeMargin", kScreenEdgeMargin);
	// ---- 状態アイコン ----
	gv->AddItem(kGroupName, "StatusIconSize", kStatusIconSize);
	gv->AddItem(kGroupName, "StatusFadeRate", kStatusFadeRate);
	gv->AddItem(kGroupName, "ReactiveIconSize", kReactiveIconSize);
	gv->AddItem(kGroupName, "ReactivePopScale", kReactivePopScale);
	gv->AddItem(kGroupName, "ReactivePopDuration", kReactivePopDuration);
	gv->AddItem(kGroupName, "ReactiveHoldDuration", kReactiveHoldDuration);
	gv->AddItem(kGroupName, "ReactiveFadeDuration", kReactiveFadeDuration);
	gv->AddItem(kGroupName, "ReactiveFlashFrequency", kReactiveFlashFrequency);
	gv->AddItem(kGroupName, "ReactiveOffsetY", kReactiveOffsetY);
	gv->AddItem(kGroupName, "ReactiveSpacing", kReactiveSpacing);
	gv->AddItem(kGroupName, "AlertRetriggerCooldown", kAlertRetriggerCooldown);
	gv->AddItem(kGroupName, "AlertOffsetY", kAlertOffsetY);
}

void StatusIndicator::ApplyGlobalVariables() {
	GlobalVariables* gv = GlobalVariables::GetInstance();
	kEnemyCountThreshold = gv->GetIntValue(kGroupName, "EnemyCountThreshold");
	kIconSize = gv->GetIntValue(kGroupName, "IconSize");
	kArrowSize = gv->GetIntValue(kGroupName, "ArrowSize");
	kHeadOffsetY = gv->GetFloatValue(kGroupName, "HeadOffsetY");
	kScreenEdgeMargin = gv->GetFloatValue(kGroupName, "ScreenEdgeMargin");
	// ---- 状態アイコン ----
	kStatusIconSize = gv->GetIntValue(kGroupName, "StatusIconSize");
	kStatusFadeRate = std::clamp(gv->GetFloatValue(kGroupName, "StatusFadeRate"), 0.01f, 1.0f);
	kReactiveIconSize = gv->GetIntValue(kGroupName, "ReactiveIconSize");
	kReactivePopScale = gv->GetFloatValue(kGroupName, "ReactivePopScale");
	kReactivePopDuration = std::max(gv->GetFloatValue(kGroupName, "ReactivePopDuration"), 0.01f);
	kReactiveHoldDuration = std::max(gv->GetFloatValue(kGroupName, "ReactiveHoldDuration"), 0.0f);
	kReactiveFadeDuration = std::max(gv->GetFloatValue(kGroupName, "ReactiveFadeDuration"), 0.01f);
	kReactiveFlashFrequency = gv->GetFloatValue(kGroupName, "ReactiveFlashFrequency");
	kReactiveOffsetY = gv->GetFloatValue(kGroupName, "ReactiveOffsetY");
	kReactiveSpacing = gv->GetFloatValue(kGroupName, "ReactiveSpacing");
	kAlertRetriggerCooldown = std::max(gv->GetFloatValue(kGroupName, "AlertRetriggerCooldown"), 0.0f);
	kAlertOffsetY = gv->GetFloatValue(kGroupName, "AlertOffsetY");
}

void StatusIndicator::Initialize(uint32_t arrowTextureHandle, uint32_t fallbackIconTextureHandle) {
	arrowTextureHandle_ = arrowTextureHandle;
	fallbackIconTextureHandle_ = fallbackIconTextureHandle;
}

StatusIndicator::IndicatorVisual& StatusIndicator::AcquireVisual(size_t index, uint32_t iconTexture) {
	if (index >= visuals_.size()) {
		visuals_.resize(index + 1);
	}
	IndicatorVisual& visual = visuals_[index];
	if (!visual.icon) {
		visual.icon.reset(Sprite::Create(iconTexture, {0.0f, 0.0f}));
		visual.icon->SetSize({static_cast<float>(kIconSize), static_cast<float>(kIconSize)});
		visual.icon->SetAnchorPoint({0.5f, 0.5f});
	}
	if (!visual.arrow) {
		visual.arrow.reset(Sprite::Create(arrowTextureHandle_, {0.0f, 0.0f}));
		visual.arrow->SetSize({static_cast<float>(kArrowSize), static_cast<float>(kArrowSize)});
		visual.arrow->SetAnchorPoint({0.5f, 0.5f});
	}
	visual.inUse = true;
	return visual;
}

void StatusIndicator::ReleaseUnusedVisuals(size_t activeCount) {
	for (size_t i = activeCount; i < visuals_.size(); ++i) {
		visuals_[i].inUse = false;
	}
}

void StatusIndicator::Update(const std::vector<BaseEnemy*>& arenaEnemies, const Camera& camera, int screenWidth, int screenHeight) {
	std::vector<BaseEnemy*> alive;
	alive.reserve(arenaEnemies.size());
	for (BaseEnemy* enemy : arenaEnemies) {
		if (enemy && !enemy->IsDead()) {
			alive.push_back(enemy);
		}
	}

	bool showIndicators = static_cast<int>(alive.size()) < kEnemyCountThreshold && !alive.empty();
	isActive_ = showIndicators;

	if (!showIndicators) {
		ReleaseUnusedVisuals(0);
		return;
	}

	Matrix4x4 matViewport = MakeViewportMatrix(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, 1.0f);
	Matrix4x4 matVPV = Multiply(camera.matView, Multiply(camera.matProjection, matViewport));

	float halfW = static_cast<float>(screenWidth) * 0.5f;
	float halfH = static_cast<float>(screenHeight) * 0.5f;
	float edge = kScreenEdgeMargin;

	for (size_t i = 0; i < alive.size(); ++i) {
		BaseEnemy* enemy = alive[i];
		uint32_t iconTexture = enemy->GetHUDIconTexture() != 0 ? enemy->GetHUDIconTexture() : fallbackIconTextureHandle_;

		IndicatorVisual& visual = AcquireVisual(i, iconTexture);
		visual.icon->SetTextureHandle(iconTexture);

		Vector3 headWorldPos = enemy->GetWorldPosition() + Vector3(0.0f, kHeadOffsetY, 0.0f);
		Vector3 screenPos = TransformCoordinate(headWorldPos, matVPV);

		bool onScreen = screenPos.x >= 0.0f && screenPos.x <= static_cast<float>(screenWidth) && screenPos.y >= 0.0f && screenPos.y <= static_cast<float>(screenHeight);

		bool spotted = enemy->IsPlayerSpotted();
		Vector4 iconColor = spotted ? Vector4(1.0f, 0.35f, 0.35f, 1.0f) : Vector4(1.0f, 1.0f, 1.0f, 1.0f);

		if (onScreen) {
			visual.arrow->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
			visual.icon->SetPosition({screenPos.x, screenPos.y});
			visual.icon->SetRotation(0.0f);
			visual.icon->SetColor(iconColor);
		} else {
			Vector2 dir = {screenPos.x - halfW, screenPos.y - halfH};
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len < 0.0001f) {
				dir = {1.0f, 0.0f};
				len = 1.0f;
			}
			dir.x /= len;
			dir.y /= len;

			float scaleX = (dir.x != 0.0f) ? (halfW - edge) / std::abs(dir.x) : std::numeric_limits<float>::max();
			float scaleY = (dir.y != 0.0f) ? (halfH - edge) / std::abs(dir.y) : std::numeric_limits<float>::max();
			float scale = std::min(scaleX, scaleY);

			Vector2 clampedPos = {halfW + dir.x * scale, halfH + dir.y * scale};
			float angle = std::atan2(dir.y, dir.x);

			visual.arrow->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
			visual.arrow->SetPosition(clampedPos);
			visual.arrow->SetRotation(angle);

			visual.icon->SetPosition({clampedPos.x, clampedPos.y});
			visual.icon->SetRotation(0.0f);
			visual.icon->SetColor(iconColor);
		}
	}

	ReleaseUnusedVisuals(alive.size());
}

void StatusIndicator::Draw() {
	if (!isActive_) {
		return;
	}
	Sprite::PreDraw();
	for (IndicatorVisual& visual : visuals_) {
		if (!visual.inUse) {
			continue;
		}
		if (visual.arrow) {
			visual.arrow->Draw();
		}
		if (visual.icon) {
			visual.icon->Draw();
		}
	}
	Sprite::PostDraw();
}

// =====================================================================
// 追加: 状態アイコン
// =====================================================================
bool StatusIndicator::LoadIfExists(const char* fileName, uint32_t& outTex) {
	for (const char* dir : kStatusImageDirs) {
		const std::string path = std::string(dir) + fileName;
		std::ifstream f(path);
		if (!f.good())
			continue;
		outTex = TextureManager::Load(path);
		return true;
	}
	return false; // どこにも無い -> 色付きの白で代用
}

void StatusIndicator::InitializeStatus(uint32_t whiteTextureHandle) {
	whiteTex_ = whiteTextureHandle;
	entityVisuals_.clear();

	// standard(slowed / buffed / knockback / stunned)
	const char* standardFiles[kStatusCount] = {nullptr, "statusSlowed.png", "statusBuffed.png", "statusKnockback.png", "statusStunned.png"};
	for (int i = kStatusSlowed; i < kStatusCount; ++i) {
		standardTex_[i] = whiteTex_;
		standardHasTex_[i] = LoadIfExists(standardFiles[i], standardTex_[i]);
	}
	// reactive(バフ / リロード / ブロック)
	const char* reactiveFiles[kReactiveCount] = {"statusBuffed.png", "statusReload.png", "statusBlocked.png", "statusAlert.png", "statusMissing.png", "statusCheckpoint.png", "statusHealed.png"};
	pendingCheckpoint_ = false;
	lastPlayerHp_ = -1;
	for (int i = 0; i < kReactiveCount; ++i) {
		reactiveTex_[i] = whiteTex_;
		reactiveHasTex_[i] = LoadIfExists(reactiveFiles[i], reactiveTex_[i]);
	}
}

StatusIndicator::Flags StatusIndicator::GatherFlags(const BaseEnemy& enemy) {
	Flags f;
	const BaseEnemyBehavior& b = enemy.GetBehaviorModule();
	f.knockback = b.IsKnockedBack();
	f.stunned = b.IsStunned();
	f.buffed = OfficerBuff::gActive; // 公式バフは全敵共通
	f.slowed = false;                // 敵のスロー状態が実装されたらここに繋ぐ(現状 BaseEnemy には無い)

	f.reactive[kReactiveBuff] = OfficerBuff::gActive;
	f.reactive[kReactiveReload] = enemy.GetFirearmMode() == BaseEnemy::FirearmMode::kReloading || enemy.GetThrowableMode() == BaseEnemy::ThrowableMode::kReloading;
	f.reactive[kReactiveBlock] = b.IsBlocking();
	// 発見 = 見え始めた瞬間 / 見失い = 見えなくなった瞬間(どちらも立ち上がりで再生)
	f.reactive[kReactiveAlert] = enemy.IsPlayerSpotted();
	f.reactive[kReactiveMissing] = !enemy.IsPlayerSpotted();
	return f;
}

StatusIndicator::Flags StatusIndicator::GatherFlags(Player& player) {
	Flags f;
	const Player::DebuffStatus d = player.GetDebuffStatus();
	f.knockback = d.knockedBack;
	f.stunned = d.stunned;
	f.slowed = d.slowedByTerrain || d.slowedByEffect;
	f.buffed = false; // プレイヤーはバフを受けない
	f.reactive[kReactiveBlock] = player.IsBlocking();
	// リロードは敵のみ(プレイヤーは武器UIがあるので出さない)

	// 回復: HPが前フレームより増えたら(どんな回復手段でも)。最初のフレームは基準を取るだけ
	const int hp = player.GetHP();
	if (lastPlayerHp_ >= 0 && hp > lastPlayerHp_)
		f.trigger[kReactiveHealing] = true;
	lastPlayerHp_ = hp;

	// チェックポイント到達(GameSceneからの通知を1回だけ消費する)
	if (pendingCheckpoint_) {
		f.trigger[kReactiveCheckpoint] = true;
		pendingCheckpoint_ = false;
	}
	return f;
}

void StatusIndicator::UpdateEntity(const void* key, const Flags& flags, const AABB& aabb, const Matrix4x4& matVPV, int screenWidth, int screenHeight) {
	// 初めて見るキャラは「今の状態」を基準にする(スポーン時にいきなり光らせない)
	bool isNew = (entityVisuals_.find(key) == entityVisuals_.end());
	EntityVisual& v = entityVisuals_[key];
	v.seen = true;
	if (isNew) {
		for (int i = 0; i < kReactiveCount; ++i)
			v.reactive[i].prevCondition = flags.reactive[i];
	}

	// ---- 体の中心 -> スクリーン座標 ----
	const Vector3 center = {(aabb.min.x + aabb.max.x) * 0.5f, (aabb.min.y + aabb.max.y) * 0.5f, (aabb.min.z + aabb.max.z) * 0.5f};
	const Vector3 screenPos = TransformCoordinate(center, matVPV);
	v.onScreen = screenPos.x >= 0.0f && screenPos.x <= static_cast<float>(screenWidth) && screenPos.y >= 0.0f && screenPos.y <= static_cast<float>(screenHeight);

	// ================= standard =================
	// 一番強い状態を選ぶ(slowed < buffed < knockback < stunned)
	int desired = kStatusNone;
	if (flags.slowed)
		desired = kStatusSlowed;
	if (flags.buffed)
		desired = kStatusBuffed;
	if (flags.knockback)
		desired = kStatusKnockback;
	if (flags.stunned)
		desired = kStatusStunned;

	if (v.shown == kStatusNone) {
		// 何も出ていない -> 新しい状態を0からフェードイン
		v.shown = desired;
		v.alpha = 0.0f;
	}
	if (desired != v.shown) {
		// 別の状態に変わった -> いまのを消してから切り替える
		v.alpha += (0.0f - v.alpha) * kStatusFadeRate;
		if (v.alpha <= 0.02f) {
			v.alpha = 0.0f;
			v.shown = desired;
		}
	} else {
		const float target = (v.shown == kStatusNone) ? 0.0f : 1.0f;
		v.alpha += (target - v.alpha) * kStatusFadeRate;
	}

	if (v.shown != kStatusNone) {
		// 状態ごとに専用スプライトを、正しいテクスチャで生成する(後からSetTextureHandleで差し替えない)
		std::unique_ptr<Sprite>& s = v.standardSprites[v.shown];
		if (!s) {
			s.reset(Sprite::Create(standardTex_[v.shown], {0.0f, 0.0f}));
			s->SetAnchorPoint({0.5f, 0.5f});
		}
		s->SetSize({static_cast<float>(kStatusIconSize), static_cast<float>(kStatusIconSize)});
		s->SetPosition({screenPos.x, screenPos.y});
		// 状態の色を常に乗算する(画像があっても仮の画像でも同じ)
		s->SetColor({kStandardTint[v.shown][0], kStandardTint[v.shown][1], kStandardTint[v.shown][2], v.alpha});
	}

	// ================= reactive =================
	const float totalDuration = kReactivePopDuration + kReactiveHoldDuration + kReactiveFadeDuration;
	for (int i = 0; i < kReactiveCount; ++i) {
		ReactiveState& r = v.reactive[i];
		r.cooldown = std::max(0.0f, r.cooldown - kDeltaTime);

		// 条件が false -> true になった瞬間、または一回限りのトリガーで演出を最初から始める
		const bool fire = (flags.reactive[i] && !r.prevCondition) || flags.trigger[i];
		if (fire && r.cooldown <= 0.0f) {
			r.active = true;
			r.timer = 0.0f;
			const bool isSight = (i == kReactiveAlert || i == kReactiveMissing);
			r.cooldown = isSight ? kAlertRetriggerCooldown : 0.0f;
			// 発見と見失いは同時に出さない(新しい方に置き換える)
			if (i == kReactiveAlert)
				v.reactive[kReactiveMissing].active = false;
			if (i == kReactiveMissing)
				v.reactive[kReactiveAlert].active = false;
		}
		r.prevCondition = flags.reactive[i];

		if (r.active) {
			r.timer += kDeltaTime;
			if (r.timer >= totalDuration)
				r.active = false;
		}
	}
	// 横並びの対象は Alert/Missing 以外(あの2つは上の段に1つだけ出す)
	auto isTopRow = [](int k) { return k == kReactiveAlert || k == kReactiveMissing; };
	int visibleCount = 0;
	for (int i = 0; i < kReactiveCount; ++i) {
		v.reactiveVisible[i] = v.reactive[i].active;
		if (v.reactive[i].active && !isTopRow(i))
			++visibleCount;
	}

	int slot = 0;
	for (int i = 0; i < kReactiveCount; ++i) {
		if (!v.reactiveVisible[i])
			continue;
		const ReactiveState& r = v.reactive[i];

		float scale = 1.0f;
		float alpha = 1.0f;
		if (r.timer < kReactivePopDuration) {
			// 光る + 拡大 -> 元のサイズへイージング
			const float t = r.timer / kReactivePopDuration;
			scale = 1.0f + (kReactivePopScale - 1.0f) * (1.0f - EaseOutCubic(t));
			const float wave = 0.5f + 0.5f * std::sin(2.0f * std::numbers::pi_v<float> * kReactiveFlashFrequency * r.timer);
			const float flash = 1.0f - t; // 光り方は時間とともに落ち着く
			alpha = 1.0f - flash * 0.65f * (1.0f - wave);
		} else if (r.timer < kReactivePopDuration + kReactiveHoldDuration) {
			// 元のサイズのまま表示
		} else {
			// 消える
			alpha = 1.0f - (r.timer - kReactivePopDuration - kReactiveHoldDuration) / kReactiveFadeDuration;
		}
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		if (!v.reactiveSprites[i]) {
			v.reactiveSprites[i].reset(Sprite::Create(reactiveTex_[i], {0.0f, 0.0f})); // 正しいテクスチャで生成
			v.reactiveSprites[i]->SetAnchorPoint({0.5f, 0.5f});
		}
		Sprite* s = v.reactiveSprites[i].get();
		const float size = static_cast<float>(kReactiveIconSize) * scale;
		s->SetSize({size, size});
		float x = screenPos.x;
		float y = screenPos.y + kReactiveOffsetY;
		if (isTopRow(i)) {
			y = screenPos.y + kAlertOffsetY; // 発見/見失いは上の段(中央に1つ)
		} else {
			// その他は横に並べる
			x += (static_cast<float>(slot) - static_cast<float>(visibleCount - 1) * 0.5f) * kReactiveSpacing;
			++slot;
		}
		s->SetPosition({x, y});
		s->SetColor({kReactiveTint[i][0], kReactiveTint[i][1], kReactiveTint[i][2], alpha});
		++slot;
	}
}

void StatusIndicator::UpdateStatus(Player* player, const std::vector<std::unique_ptr<BaseEnemy>>& enemies, const Camera& camera, int screenWidth, int screenHeight) {
	Matrix4x4 matViewport = MakeViewportMatrix(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, 1.0f);
	Matrix4x4 matVPV = Multiply(camera.matView, Multiply(camera.matProjection, matViewport));

	for (auto& [key, visual] : entityVisuals_)
		visual.seen = false;

	if (player && !player->IsDefeated())
		UpdateEntity(player, GatherFlags(*player), player->GetAABB(), matVPV, screenWidth, screenHeight);

	for (const std::unique_ptr<BaseEnemy>& enemy : enemies) {
		if (!enemy || enemy->IsDead() || enemy->GetBehaviorModule().IsDeathBehavior())
			continue;
		UpdateEntity(enemy.get(), GatherFlags(*enemy), enemy->GetAABB(), matVPV, screenWidth, screenHeight);
	}

	// いなくなったキャラ(死亡 / 画面外に削除)の分を片付ける
	for (auto it = entityVisuals_.begin(); it != entityVisuals_.end();) {
		it = it->second.seen ? std::next(it) : entityVisuals_.erase(it);
	}
}

void StatusIndicator::DrawStatus() {
	Sprite::PreDraw();
	for (auto& [key, v] : entityVisuals_) {
		if (!v.onScreen)
			continue;
		if (v.shown != kStatusNone && v.alpha > 0.01f && v.standardSprites[v.shown])
			v.standardSprites[v.shown]->Draw();
		for (int i = 0; i < kReactiveCount; ++i) {
			if (v.reactiveVisible[i] && v.reactiveSprites[i])
				v.reactiveSprites[i]->Draw();
		}
	}
	Sprite::PostDraw();
}