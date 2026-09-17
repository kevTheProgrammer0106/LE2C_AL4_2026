#pragma once
#include <cstdint>
#include <iostream>
#include <random>

class RandomGenerator {
public:
	// 乱数生成エンジン
	std::random_device seedGenerator;
	// メルセンヌ・ツイスターエンジン(64bit版)
	std::mt19937_64 randomEngine;

	void Initialize();

	// 乱数を1つ生成して返す（0.0f〜3.1fの固定範囲、既存の呼び出し元との互換用）
	float GetRandValue() { return rotationDistribution_(randomEngine); }

	// 指定範囲の乱数を1つ生成して返す（min〜max）
	float GetRandValue(float min, float max) {
		std::uniform_real_distribution<float> dist(min, max);
		return dist(randomEngine);
	}

private:
	std::uniform_real_distribution<float> rotationDistribution_{0.0f, 3.1f};
};