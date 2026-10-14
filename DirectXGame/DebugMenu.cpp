#define NOMINMAX

#include "DebugMenu.h"

#include "GameScene.h"
#include "StageManager.h"
#include "EnemyCarabinier.h"

#include <array>
#include <cstdio>
#include <unordered_map>

using namespace KamataEngine;

// Player debuff section needs Player::DebuffStatus / GetDebuffStatus / HasAnyDebuff / ClearAllDebuffs /
// ClearStun / ClearKnockback / ApplySlow / ClearSlow. Set to 0 if your Player doesn't have them.
#ifndef DEBUGMENU_PLAYER_DEBUFFS
#define DEBUGMENU_PLAYER_DEBUFFS 1
#endif

#ifdef _DEBUG

namespace {

// First-seen value of every edited SRTOffset, so "Reset" works.
std::unordered_map<const void*, SRTOffset> sOriginals;

bool DragF(const char* label, float& v, float speed) { return ImGui::DragFloat(label, &v, speed); }
bool DragF3Arr(const char* label, std::array<float, 3>& a, float speed) { return ImGui::DragFloat3(label, a.data(), speed); }

bool EditSRT(const char* label, SRTOffset& v) {
	bool changed = false;
	auto it = sOriginals.try_emplace(&v, v).first;

	ImGui::PushID(&v);
	if (ImGui::TreeNode(label)) {
		changed |= ImGui::DragFloat3("Scale", &v.scale.x, 0.01f);
		changed |= ImGui::DragFloat3("Rotation (rad)", &v.rotation.x, 0.01f);
		changed |= ImGui::DragFloat3("Translation", &v.translation.x, 0.005f);
		if (ImGui::SmallButton("Reset")) {
			v = it->second;
			changed = true;
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
	return changed;
}

bool EditComboPatterns(const char* id, std::array<ComboAttackPattern, 3>& patterns) {
	static const char* kPhaseNames[4] = {"anticipateStart", "anticipateEnd", "attackEnd", "recoveryEnd"};
	bool changed = false;

	ImGui::PushID(id);
	for (int i = 0; i < 3; ++i) {
		char name[32];
		std::snprintf(name, sizeof(name), "Combo %d", i);
		if (ImGui::TreeNode(name)) {
			ComboAttackPattern& p = patterns[i];
			SRTOffset* poses[4] = {&p.anticipateStart, &p.anticipateEnd, &p.attackEnd, &p.recoveryEnd};
			for (int k = 0; k < 4; ++k)
				changed |= EditSRT(kPhaseNames[k], *poses[k]);
			ImGui::TreePop();
		}
	}
	ImGui::PopID();
	return changed;
}

const char* EnemyTypeName(BaseEnemy* e) {
	if (dynamic_cast<EnemySapper*>(e))
		return "Sapper";
	if (dynamic_cast<EnemyGrenadier*>(e))
		return "Grenadier";
	if (dynamic_cast<EnemyOfficer*>(e))
		return "Officer";
	if (dynamic_cast<EnemyCarabinier*>(e))
		return "Carabinier";
	if (dynamic_cast<EnemyShotgunner*>(e))
		return "Shotgunner";
	if (dynamic_cast<EnemyFighter*>(e))
		return "Fighter";
	if (dynamic_cast<EnemyCuirassier*>(e))
		return "Cuirassier";
	if (dynamic_cast<EnemyRifleman*>(e))
		return "Rifleman";
	return "Enemy";
}

const char* PhaseName(GameScene::Phase p) {
	switch (p) {
	case GameScene::Phase::kFadeIn:
		return "FadeIn";
	case GameScene::Phase::kPlay:
		return "Play";
	case GameScene::Phase::kPause:
		return "Pause";
	case GameScene::Phase::kDeath:
		return "Death";
	case GameScene::Phase::kRetry:
		return "Retry";
	case GameScene::Phase::kFadeOut:
		return "FadeOut";
	case GameScene::Phase::kVictory:
		return "Victory";
	case GameScene::Phase::kGoalSequence:
		return "GoalSequence";
	}
	return "?";
}

const char* ArenaStateName(ArenaManager::State s) {
	switch (s) {
	case ArenaManager::State::kDormant:
		return "Dormant";
	case ArenaManager::State::kActive:
		return "Active";
	case ArenaManager::State::kCleared:
		return "Cleared";
	}
	return "?";
}

constexpr ImVec4 kWarn = {1.0f, 0.75f, 0.2f, 1.0f};

} // namespace

// ------------------------------------------------------------------ Main window
void DebugMenu::Draw(GameScene* gs) {
	if (!gs)
		return;

	gs->reloadRequested_ = false;

	Input* input = Input::GetInstance();
	if (input->TriggerKey(DIK_F3))
		isOpen_ = !isOpen_;
	if (input->TriggerKey(DIK_F2)) {
		bool both = BaseEnemyBehavior::IsDebugDecisionFreeze() && BaseEnemyBehavior::IsDebugMovementFreeze();
		BaseEnemyBehavior::SetDebugDecisionFreeze(!both);
		BaseEnemyBehavior::SetDebugMovementFreeze(!both);
	}

	// Runs even while the window is closed so newly spawned enemies keep your head/hat edits.
	PushBodyOffsetsToEnemies(gs);

	if (!isOpen_)
		return;

	if (!ImGui::Begin("Debug Menu (F3)", &isOpen_)) {
		ImGui::End();
		return;
	}

	ImGui::TextDisabled("F1 Reload field | F2 freeze enemies | F3 this window | F4 collision boxes | P debug camera");
	ImGui::TextDisabled("Values registered in GlobalVariables are edited in the Global Variables window, not here.");

	if (ImGui::BeginTabBar("DebugMenuTabs")) {
		if (ImGui::BeginTabItem("Scene")) {
			DrawSceneTab(gs);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Player")) {
			DrawPlayerTab(gs);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Spawn")) {
			DrawSpawnTab(gs);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Enemies")) {
			DrawEnemiesTab(gs);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Sabre (Fighter)")) {
			DrawSabreTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Heavy Sabre (Cuirassier)")) {
			DrawHeavySabreTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Musket (Rifleman)")) {
			DrawMusketTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Body / Anim")) {
			DrawBodyTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Combat")) {
			DrawCombatTab();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

// ------------------------------------------------------------------ Scene
void DebugMenu::DrawSceneTab(GameScene* gs) {
	StageManager* sm = gs->stageManager_;

	ImGui::Text("Phase: %s", PhaseName(gs->phase_));
	if (sm)
		ImGui::Text("Stage: %s (index %d)", sm->GetCurrentStageData().name.c_str(), sm->GetCurrentStageIndex());
	ImGui::Text("Debug camera (P): %s", gs->isDebugCameraActive_ ? "ON" : "off");

	if (ImGui::Button("Reload Scene") || Input::GetInstance()->TriggerKey(DIK_F1))
		gs->reloadRequested_ = true;

	if (sm) {
		ImGui::Text("Load stage:");
		int count = sm->GetStageCount();
		for (int i = 0; i < count; ++i) {
			if (i > 0)
				ImGui::SameLine();
			char label[96];
			std::snprintf(label, sizeof(label), "%s##stage%d", sm->GetStageData(i).name.c_str(), i);
			if (ImGui::Button(label)) {
				sm->SetCurrentStageIndex(i);
				gs->reloadRequested_ = true;
			}
		}
	}

	ImGui::Separator();
	ImGui::Checkbox("Show collision boxes (F1)", &gs->showCollisionDebug_);

	if (gs->player_) {
		Vector3 p = gs->player_->GetWorldPosition();
		if (ImGui::Button("Spawn Guard Effect"))
			gs->CreateGuardEffect(p);
		ImGui::SameLine();
		if (ImGui::Button("Spawn Hit Effect"))
			gs->CreateHitEffect(p);
	}

	if (ImGui::CollapsingHeader("Arenas")) {
		if (gs->arenas_.empty())
			ImGui::TextDisabled("No arenas registered");
		for (const auto& arena : gs->arenas_) {
			if (!arena)
				continue;
			ImGui::Text(
			    "Arena %d: %s  remaining %d %s", static_cast<int>(arena->GetArenaID()), ArenaStateName(arena->GetState()), arena->GetRemainingEnemyCount(gs),
			    arena->IsWaitingForNextWave() ? "[waiting for next wave]" : "");
		}
	}

	if (ImGui::CollapsingHeader("Camera") && gs->cameraController_)
		gs->cameraController_->DrawDebugControls();
}

// ------------------------------------------------------------------ Player
void DebugMenu::DrawPlayerTab(GameScene* gs) {
	Player* player = gs->player_.get();
	if (!player) {
		ImGui::Text("No player");
		return;
	}

	int hp = player->GetHP();
	if (ImGui::SliderInt("HP", &hp, 0, player->GetMaxHP()))
		player->SetHP(hp);

	if (ImGui::Button("Reset HP (100)")) {
		player->SetMaxHP(100);
		player->SetHP(100);
	}
	ImGui::SameLine();
	if (ImGui::Button("Debug HP (9999)")) {
		player->SetMaxHP(9999);
		player->SetHP(9999);
	}
	ImGui::SameLine();
	if (ImGui::Button("Kill player"))
		player->KillInstant();

	ImGui::Separator();
	bool firearm = player->GetWeaponMode() == Player::WeaponMode::kFirearm;
	if (ImGui::RadioButton("Melee", !firearm))
		player->SetWeaponMode(Player::WeaponMode::kMelee);
	ImGui::SameLine();
	if (ImGui::RadioButton("Firearm", firearm))
		player->SetWeaponMode(Player::WeaponMode::kFirearm);

	bool locked = player->IsControlLocked();
	if (ImGui::Checkbox("Control Locked", &locked))
		player->SetControlLocked(locked);

#if DEBUGMENU_PLAYER_DEBUFFS
	if (ImGui::CollapsingHeader("Debuffs", ImGuiTreeNodeFlags_DefaultOpen)) {
		const ImVec4 kOn = {1.0f, 0.45f, 0.25f, 1.0f};
		const ImVec4 kOff = {0.55f, 0.55f, 0.55f, 1.0f};
		Player::DebuffStatus s = player->GetDebuffStatus();

		ImGui::Text("Active: %s", player->HasAnyDebuff() ? "YES" : "none");
		ImGui::Separator();

		if (s.stunned)
			ImGui::TextColored(kOn, "Stun: %.2fs left", s.stunRemaining);
		else
			ImGui::TextColored(kOff, "Stun: -");

		ImGui::TextColored(s.knockedBack ? kOn : kOff, "Knockback: %s", s.knockedBack ? "ACTIVE" : "-");
		ImGui::TextColored(s.slowedByTerrain ? kOn : kOff, "Slow (terrain): %s (x%.2f)", s.slowedByTerrain ? "ACTIVE" : "-", MapChipField::kSlowSpeedFactor);

		if (s.slowedByEffect) {
			if (s.slowUntilCleared)
				ImGui::TextColored(kOn, "Slow (effect): x%.2f until cleared", s.slowFactor);
			else
				ImGui::TextColored(kOn, "Slow (effect): x%.2f, %.2fs left", s.slowFactor, s.slowRemaining);
		} else {
			ImGui::TextColored(kOff, "Slow (effect): -");
		}

		ImGui::TextColored(s.controlLocked ? kOn : kOff, "Control Locked: %s", s.controlLocked ? "ACTIVE" : "-");

		if (s.beingPushed)
			ImGui::TextColored(kOn, "Scroll Push: ACTIVE (vx %.3f, squish %u)", s.pushVelocityX, s.squishTimer);
		else
			ImGui::TextColored(kOff, "Scroll Push: -");

		ImGui::Separator();
		ImGui::Text("Disable");
		if (ImGui::Button("Clear ALL Debuffs"))
			player->ClearAllDebuffs();
		if (ImGui::Button("Clear Stun"))
			player->ClearStun();
		ImGui::SameLine();
		if (ImGui::Button("Clear Knockback"))
			player->ClearKnockback();
		ImGui::SameLine();
		if (ImGui::Button("Clear Slow"))
			player->ClearSlow();

		ImGui::Separator();
		ImGui::Text("Apply");

		static float stunDuration = 1.5f;
		ImGui::SliderFloat("Stun Duration", &stunDuration, 0.1f, 10.0f, "%.2f s");
		if (ImGui::Button("Apply Stun"))
			player->SetStunned(stunDuration);

		static float knockbackPower = 5.0f;
		ImGui::SliderFloat("Knockback Power", &knockbackPower, 0.5f, 20.0f);
		if (ImGui::Button("Knockback Left"))
			player->SetKnockback({-1.0f, 0.4f, 0.0f}, knockbackPower);
		ImGui::SameLine();
		if (ImGui::Button("Knockback Right"))
			player->SetKnockback({1.0f, 0.4f, 0.0f}, knockbackPower);

		static float slowFactor = 0.45f;
		static float slowDuration = 5.0f;
		static bool slowUntilCleared = false;
		ImGui::SliderFloat("Slow Factor", &slowFactor, 0.05f, 1.0f);
		ImGui::Checkbox("Slow until cleared", &slowUntilCleared);
		if (!slowUntilCleared)
			ImGui::SliderFloat("Slow Duration", &slowDuration, 0.1f, 30.0f, "%.1f s");
		if (ImGui::Button("Apply Slow"))
			player->ApplySlow(slowFactor, slowUntilCleared ? 0.0f : slowDuration);
	}
#endif
}

// ------------------------------------------------------------------ Spawn
void DebugMenu::DrawSpawnTab(GameScene* gs) {
	Player* player = gs->player_.get();
	if (!player) {
		ImGui::Text("No player");
		return;
	}
	Vector3 p = player->GetWorldPosition();

	ImGui::DragFloat("Spawn distance from player", &spawnOffsetX_, 0.1f, 0.0f, 30.0f);

	// enemy type ids: 0 = Rifleman, 1 = Cuirassier, 2 = Fighter
	Vector3 right = {p.x + spawnOffsetX_, p.y, p.z};
	Vector3 left = {p.x - spawnOffsetX_, p.y, p.z};

	if (ImGui::Button("Spawn Rifleman (right)"))
		gs->SpawnEnemyAt(right, 0);
	ImGui::SameLine();
	if (ImGui::Button("Spawn Cuirassier (left)"))
		gs->SpawnEnemyAt(left, 1);
	ImGui::SameLine();
	if (ImGui::Button("Spawn Fighter (left)"))
		gs->SpawnEnemyAt(left, 2);
	ImGui::SameLine();
	if (ImGui::Button("Spawn Carabiner (right)"))
		gs->SpawnEnemyAt(right, 3);
	ImGui::SameLine();
	if (ImGui::Button("Spawn Shotgunner (right)"))
		gs->SpawnEnemyAt(right, 4);

	if (ImGui::Button("Spawn Grenadier (left)"))
		gs->SpawnEnemyAt(left, 5);
	ImGui::SameLine();
	if (ImGui::Button("Spawn Officer (left)"))
		gs->SpawnEnemyAt(left, 6);
	if (ImGui::Button("Spawn Sapper (left)"))
		gs->SpawnEnemyAt(left, 7);

	if (ImGui::Button("Spawn one of each")) {
		gs->SpawnEnemyAt(right, 0);
		gs->SpawnEnemyAt(left, 2);
		Vector3 farthest = {p.x - spawnOffsetX_ * 2.0f, p.y, p.z};
		gs->SpawnEnemyAt(farthest, 1);
		gs->SpawnEnemyAt(farthest, 3);
		gs->SpawnEnemyAt(right, 4);
		gs->SpawnEnemyAt(left, 5);
		gs->SpawnEnemyAt(farthest, 6);
		gs->SpawnEnemyAt(left, 7);
	}

	ImGui::Separator();
	ImGui::Text("Spawners at player position");
	if (ImGui::Button("Limited Rifleman (x3, every 4s)"))
		gs->AddEnemySpawner(p, 0, 4.0f, EnemySpawner::SpawnMode::kLimited, 3, false);
	if (ImGui::Button("Endless Cuirassier (every 8s, omniscient)"))
		gs->AddEnemySpawner(p, 1, 8.0f, EnemySpawner::SpawnMode::kEndless, -1, true);
	ImGui::Text("Active spawners: %d", static_cast<int>(gs->enemySpawners_.size()));

		// ---- アイテムスポーナー(武器ドロップ) ----
	ImGui::Separator();
	ImGui::Text("Item spawner");

	// 武器の種類と状態の選択(static: 選択を保持する)
	static int itemKindIndex = 0;
	static int itemStateIndex = 0;
	static float itemSpawnOffsetX = 1.5f;
	static float itemSpawnOffsetY = 1.0f;
	static bool itemSpawnFront = true;
	static int itemSpawnCount = 1;

	struct ItemEntry {
		const char* label;
		DropItemCategory category;
		DropWeaponKind kind;
	};
	static const ItemEntry kItemTable[] = {
	    {"Sabre (light)",    DropItemCategory::kMelee,   DropWeaponKind::kSabreLight   },
	    {"Sabre (heavy)",    DropItemCategory::kMelee,   DropWeaponKind::kSabreHeavy   },
	    {"Axe",              DropItemCategory::kMelee,   DropWeaponKind::kLongAxe      },
	    {"Spontoon", DropItemCategory::kMelee,   DropWeaponKind::kSpear},
	    {"Musket (long)",    DropItemCategory::kFirearm, DropWeaponKind::kMusketLong   },
	    {"Musket (bayonet)", DropItemCategory::kFirearm, DropWeaponKind::kMusketBayonet},
	    {"Shotgun",  DropItemCategory::kFirearm, DropWeaponKind::kShotgun},
	    {"Dual Artillery Pistols", DropItemCategory::kFirearm, DropWeaponKind::kDualPistols  },
	};
	constexpr int kItemCount = static_cast<int>(sizeof(kItemTable) / sizeof(kItemTable[0]));

	if (ImGui::BeginCombo("Weapon", kItemTable[itemKindIndex].label)) {
		for (int i = 0; i < kItemCount; ++i) {
			if (ImGui::Selectable(kItemTable[i].label, itemKindIndex == i))
				itemKindIndex = i;
		}
		ImGui::EndCombo();
	}

	// 銃: Ready=装填済み / OnCooldown=未装填、近接: Ready=通常 / OnCooldown=クールダウン中(拾い直しでリセットされる)
	static const char* kStateLabels[] = {"Ready (loaded / ready)", "OnCooldown (unloaded / cooling)"};
	ImGui::Combo("State", &itemStateIndex, kStateLabels, 2);

	ImGui::DragFloat("Offset X", &itemSpawnOffsetX, 0.05f, 0.0f, 20.0f);
	ImGui::DragFloat("Offset Y", &itemSpawnOffsetY, 0.05f, -5.0f, 10.0f);
	ImGui::Checkbox("In front of player (facing)", &itemSpawnFront);
	ImGui::SliderInt("Count", &itemSpawnCount, 1, 10);

	// プレイヤーの位置(向きに応じて前方 or 常に右)からスポーン位置を決める
	auto itemSpawnPosition = [&]() {
		float dir = 1.0f;
		if (itemSpawnFront && player->isLeft())
			dir = -1.0f;
		return Vector3{p.x + itemSpawnOffsetX * dir, p.y + itemSpawnOffsetY, p.z};
	};
	const DropItemState itemState = (itemStateIndex == 0) ? DropItemState::kReady : DropItemState::kOnCooldown;

	if (ImGui::Button("Spawn item")) {
		const ItemEntry& e = kItemTable[itemKindIndex];
		for (int i = 0; i < itemSpawnCount; ++i)
			gs->CreateDroppableItem(itemSpawnPosition(), e.category, e.kind, itemState);
	}
	ImGui::SameLine();
	if (ImGui::Button("Spawn one of each")) {
		for (int i = 0; i < kItemCount; ++i)
			gs->CreateDroppableItem(itemSpawnPosition(), kItemTable[i].category, kItemTable[i].kind, itemState);
	}
	ImGui::Text("Droppable items: %d", static_cast<int>(gs->droppableItems_.size()));
	ImGui::SameLine();
	if (ImGui::SmallButton("Clear items"))
		gs->droppableItems_.clear();
}

// ------------------------------------------------------------------ Enemies
void DebugMenu::DrawEnemiesTab(GameScene* gs) {
	bool freezeDecisions = BaseEnemyBehavior::IsDebugDecisionFreeze();
	if (ImGui::Checkbox("Freeze Enemy Decisions", &freezeDecisions))
		BaseEnemyBehavior::SetDebugDecisionFreeze(freezeDecisions);
	bool freezeMovement = BaseEnemyBehavior::IsDebugMovementFreeze();
	if (ImGui::Checkbox("Freeze Enemy Movement", &freezeMovement))
		BaseEnemyBehavior::SetDebugMovementFreeze(freezeMovement);
	ImGui::TextDisabled("F2 toggles both. Frozen enemies still animate their current attack.");

	ImGui::Separator();
	const auto& enemies = gs->GetEnemies();
	ImGui::Text("Live enemies: %d", static_cast<int>(enemies.size()));

	if (ImGui::Button("Heal all")) {
		for (const auto& e : enemies)
			if (e && !e->IsDead())
				e->SetHP(e->GetMaxHP());
	}
	ImGui::SameLine();
	if (ImGui::Button("Kill all")) {
		for (const auto& e : enemies)
			if (e && !e->IsDead())
				e->KillInstant();
	}

	ImGui::Separator();
	ImGui::Text("Debuffs (all live enemies)");
	static float stunDuration = 1.5f;
	static float knockbackPower = 5.0f;
	ImGui::SliderFloat("Stun Duration", &stunDuration, 0.1f, 10.0f, "%.2f s");
	ImGui::SliderFloat("Knockback Power", &knockbackPower, 0.5f, 20.0f);

	if (ImGui::Button("Stun all")) {
		for (const auto& e : enemies)
			if (e && !e->IsDead())
				e->RequestStun(stunDuration);
	}
	ImGui::SameLine();
	if (ImGui::Button("Knockback all left")) {
		for (const auto& e : enemies)
			if (e && !e->IsDead())
				e->RequestKnockback({-1.0f, 0.4f, 0.0f}, knockbackPower);
	}
	ImGui::SameLine();
	if (ImGui::Button("Knockback all right")) {
		for (const auto& e : enemies)
			if (e && !e->IsDead())
				e->RequestKnockback({1.0f, 0.4f, 0.0f}, knockbackPower);
	}
	ImGui::SameLine();
	if (ImGui::Button("Clear debuffs")) {
		for (const auto& e : enemies)
			if (e && !e->IsDead())
				e->ClearDebuffs();
	}

	ImGui::Separator();
	int index = 0;
	for (const auto& e : enemies) {
		if (!e)
			continue;
		Vector3 p = e->GetWorldPosition();
		const BaseEnemyBehavior& b = e->GetBehaviorModule();
		ImGui::Text(
		    "#%d %-10s HP %d/%d  pos(%.1f, %.1f) arena %d %s%s%s", index++, EnemyTypeName(e.get()), e->GetHP(), e->GetMaxHP(), p.x, p.y, static_cast<int>(e->GetArenaID()),
		    e->IsPlayerSpotted() ? "[SPOTTED] " : "", b.IsStunned() ? "[STUN] " : "", b.IsKnockedBack() ? "[KNOCKBACK]" : "");
	}
}

// ------------------------------------------------------------------ Sabre (EnemyFighter)
void DebugMenu::DrawSabreTab() {
	ImGui::TextColored(kWarn, "WeaponSabre statics are shared with the PLAYER's sabre.");
	EditSRT("readyStanceOffset_", WeaponSabre::readyStanceOffset_);

	if (ImGui::CollapsingHeader("Combo poses (visible while attacking)", ImGuiTreeNodeFlags_DefaultOpen))
		EditComboPatterns("sabreCombos", WeaponSabre::comboPatterns_);

	if (ImGui::CollapsingHeader("Reach / height (enemy hitbox)")) {
		DragF3Arr("kComboReach", WeaponSabre::kComboReach, 0.02f);
		DragF3Arr("kComboSwingHeight", WeaponSabre::kComboSwingHeight, 0.02f);
	}

	if (ImGui::CollapsingHeader("BaseMelee timing (shared by ALL melee weapons)")) {
		ImGui::TextDisabled("Player");
		DragF3Arr("P Attack speed", WeaponSabre::kComboAttackSpeedPlayer, 0.01f);
		DragF3Arr("P Anticipate (frames)", WeaponSabre::kComboAnticipateDurationPlayer, 0.2f);
		DragF3Arr("P Attack (frames)", WeaponSabre::kComboAttackDurationPlayer, 0.2f);
		DragF3Arr("P Recovery (frames)", WeaponSabre::kComboRecoveryDurationPlayer, 0.2f);
		ImGui::TextDisabled("Enemy");
		DragF3Arr("E Attack speed", WeaponSabre::kComboAttackSpeedEnemy, 0.01f);
		DragF3Arr("E Anticipate (frames)", WeaponSabre::kComboAnticipateDurationEnemy, 0.2f);
		DragF3Arr("E Attack (frames)", WeaponSabre::kComboAttackDurationEnemy, 0.2f);
		DragF3Arr("E Recovery (frames)", WeaponSabre::kComboRecoveryDurationEnemy, 0.2f);
	}
}

// ------------------------------------------------------------------ Heavy sabre (EnemyCuirassier)
void DebugMenu::DrawHeavySabreTab() {
	EditSRT("readyStanceOffset_", WeaponHeavySabre::readyStanceOffset_);

	if (ImGui::CollapsingHeader("Lunge poses", ImGuiTreeNodeFlags_DefaultOpen)) {
		EditSRT("kLungeAnticipate", WeaponHeavySabre::kLungeAnticipate);
		EditSRT("kLungeExtend", WeaponHeavySabre::kLungeExtend);
	}

	if (ImGui::CollapsingHeader("Combo poses (visible while attacking)"))
		EditComboPatterns("heavyCombos", WeaponHeavySabre::comboPatterns_);

	if (ImGui::CollapsingHeader("Reach / height (enemy hitbox)")) {
		DragF3Arr("kComboReach", WeaponHeavySabre::kComboReach, 0.02f);
		DragF3Arr("kComboSwingHeight", WeaponHeavySabre::kComboSwingHeight, 0.02f);
	}
}

// ------------------------------------------------------------------ Musket (EnemyRifleman)
void DebugMenu::DrawMusketTab() {
	ImGui::TextDisabled("Musket offsets, rifleman poses and lunge timings (frames) are in the Global Variables window.");
	EditSRT("firearmOffset_ (extra, applied via SetOffset)", EnemyRifleman::firearmOffset_);

	if (ImGui::CollapsingHeader("Bayonet lunge poses", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::TextDisabled("rotation.y must stay -1.5708. translation.z = forward, .x = camera side, rotation.x = barrel pitch.");
		EditSRT("windUp", FirearmMusketBayonet::kLungePoses.windUp);
		EditSRT("thrust", FirearmMusketBayonet::kLungePoses.thrust);
	}
}

// ------------------------------------------------------------------ Body / Anim
void DebugMenu::PushBodyOffsetsToEnemies(GameScene* gs) {
	const auto& enemies = gs->GetEnemies();

	if (!bodyInitialized_) {
		for (const auto& e : enemies) {
			if (!e)
				continue;
			bodyHead_ = e->GetAnimationManager().headOffset_;
			bodyHat_ = e->GetAnimationManager().hatOffset_;
			bodyInitialized_ = true;
			break;
		}
		return; // nothing to push until real values are captured
	}

	if (!pushBodyOffsets_)
		return;
	for (const auto& e : enemies) {
		if (!e)
			continue;
		e->GetAnimationManager().headOffset_ = bodyHead_;
		e->GetAnimationManager().hatOffset_ = bodyHat_;
	}
}

void DebugMenu::DrawBodyTab() {
	ImGui::TextColored(kWarn, "AnimationManager statics are shared with the PLAYER too.");

	ImGui::Checkbox("Push head/hat offsets to all enemies every frame", &pushBodyOffsets_);
	if (!bodyInitialized_) {
		ImGui::TextDisabled("Waiting for an enemy to exist to capture head/hat offsets...");
	} else {
		EditSRT("headOffset_ (per-instance -> all enemies)", bodyHead_);
		EditSRT("hatOffset_ (per-instance -> all enemies)", bodyHat_);
	}

	if (ImGui::CollapsingHeader("Movement poses", ImGuiTreeNodeFlags_DefaultOpen)) {
		EditSRT("kIdlePoseLow", AnimationManager::kIdlePoseLow);
		EditSRT("kIdlePoseHigh", AnimationManager::kIdlePoseHigh);
		EditSRT("kRunPoseLow", AnimationManager::kRunPoseLow);
		EditSRT("kRunPoseHigh", AnimationManager::kRunPoseHigh);
		EditSRT("kJumpPose", AnimationManager::kJumpPose);
		EditSRT("kFallPose", AnimationManager::kFallPose);

	}

	DragF("FootstepInterval", AnimationManager::kFootstepInterval, 0.005f);
}

// ------------------------------------------------------------------ Combat / movement
void DebugMenu::DrawCombatTab() {
	DragF("ChaseSpeed (all enemies)", BaseEnemy::kChaseSpeed, 0.002f);

	if (ImGui::CollapsingHeader("Cuirassier AI")) {
		DragF("Lunge anticipate (s)", CuirassierBehavior::kLungeAnticipateDuration, 0.01f);
		DragF("Lunge attack (s)", CuirassierBehavior::kLungeAttackDuration, 0.01f);
		DragF("Lunge recovery (s)", CuirassierBehavior::kLungeRecoveryDuration, 0.01f);
		DragF("Lunge speed", CuirassierBehavior::kLungeSpeed, 0.005f);
		DragF("Melee engage range", CuirassierBehavior::kMeleeEngageRange, 0.05f);
	}
	if (ImGui::CollapsingHeader("Fighter AI")) {
		DragF("Melee engage range", FighterBehavior::kMeleeEngageRange, 0.05f);
		DragF("Cooldown (s)", FighterBehavior::kCooldownDuration, 0.01f);
		DragF("Combo reset (s)", FighterBehavior::kComboResetDuration, 0.05f);
		DragF("Block chance when hit", FighterBehavior::kBlockChanceWhenHit, 0.01f);
	}
	if (ImGui::CollapsingHeader("Rifleman AI")) {
		DragF("Aim duration (s)", RiflemanBehavior::kAimDuration, 0.01f);
		DragF("Retreat speed", RiflemanBehavior::kRetreatSpeed, 0.002f);
		DragF("Retreat duration (s)", RiflemanBehavior::kRetreatDuration, 0.01f);
		DragF("Retreat range", RiflemanBehavior::kRetreatRange, 0.05f);
		DragF("Bayonet range", RiflemanBehavior::kBayonetRange, 0.05f);
		DragF("Firing range", RiflemanBehavior::kFiringRange, 0.05f);
	}
}

#else // Release: compiles to nothing

void DebugMenu::Draw(GameScene*) {}

#endif