#include "surround_analyzer.h"

#include <algorithm>
#include <cmath>

namespace uvk {

namespace {

constexpr std::array<const char*, 8> kChannelNames = {
    "Front Left", "Front Right", "Center", "LFE", "Surround Left",
    "Surround Right", "Rear Left", "Rear Right"};

constexpr std::array<std::array<float, 3>, 8> kChannelVectors = {
    std::array<float, 3>{-0.7f, 0.0f, 1.0f},  // Front Left
    std::array<float, 3>{0.7f, 0.0f, 1.0f},   // Front Right
    std::array<float, 3>{0.0f, 0.0f, 1.0f},   // Center
    std::array<float, 3>{0.0f, -0.4f, 0.6f},  // LFE
    std::array<float, 3>{-1.0f, 0.0f, 0.0f},  // Surround Left
    std::array<float, 3>{1.0f, 0.0f, 0.0f},   // Surround Right
    std::array<float, 3>{-0.8f, 0.0f, -1.0f}, // Rear Left
    std::array<float, 3>{0.8f, 0.0f, -1.0f}   // Rear Right
};

}  // namespace

SurroundAnalysis SurroundAnalyzer::analyze(const SurroundBlock& block) const {
  SurroundAnalysis analysis{};
  std::array<float, 8> sumSquares{};

  if (block.samples.empty()) {
    return analysis;
  }

  for (const auto& sample : block.samples) {
    for (size_t i = 0; i < sample.size(); ++i) {
      sumSquares[i] += sample[i] * sample[i];
    }
  }

  float energySum = 0.0f;
  std::array<float, 8> weights{};
  const float invCount = 1.0f / static_cast<float>(block.samples.size());
  for (size_t i = 0; i < sumSquares.size(); ++i) {
    const float rms = std::sqrt(sumSquares[i] * invCount);
    analysis.rms[i] = rms;
    energySum += rms;
    weights[i] = rms;
  }

  analysis.energy = energySum;

  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  for (size_t i = 0; i < weights.size(); ++i) {
    x += weights[i] * kChannelVectors[i][0];
    y += weights[i] * kChannelVectors[i][1];
    z += weights[i] * kChannelVectors[i][2];
  }

  const float horizontalLength = std::sqrt(x * x + z * z);
  analysis.azimuthDegrees = std::atan2(x, z) * 57.2957795f;
  analysis.elevationDegrees = std::atan2(y, horizontalLength) * 57.2957795f;

  const auto maxIt = std::max_element(weights.begin(), weights.end());
  const size_t dominantIndex = static_cast<size_t>(std::distance(weights.begin(), maxIt));
  analysis.dominantChannel = kChannelNames[dominantIndex];

  return analysis;
}

}  // namespace uvk
