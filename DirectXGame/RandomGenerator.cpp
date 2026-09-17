#include "RandomGenerator.h"

void RandomGenerator::Initialize() {
	// メルセンヌ・ツイスターエンジンの初期化
	randomEngine.seed(seedGenerator());
	// 指定範囲の乱数生成器 (浮動小数点数用)
	std::uniform_real_distribution<float> rotationDistribution(0.0f, 6.283185f); // full circle
}