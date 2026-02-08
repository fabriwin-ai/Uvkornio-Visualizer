#include "differential_math.h"

#include <algorithm>
#include <cmath>

namespace uvk {

DifferentialBounds DifferentialMath::analyzeWaterfall(const std::vector<float>& waterfall,
                                                      size_t binCount, size_t historyLength,
                                                      bool includeBatch) {
  DifferentialBounds result{};
  if (waterfall.empty() || binCount == 0 || historyLength == 0) {
    return result;
  }

  const size_t totalSize = binCount * historyLength;
  const size_t size = std::min(totalSize, waterfall.size());
  float minValue = waterfall.front();
  float maxValue = waterfall.front();

  for (size_t i = 0; i < size; ++i) {
    minValue = std::min(minValue, waterfall[i]);
    maxValue = std::max(maxValue, waterfall[i]);
  }

  result.bounds.min = {0.0f, minValue, 0.0f};
  result.bounds.max = {static_cast<float>(binCount - 1), maxValue,
                       static_cast<float>(historyLength - 1)};

  if (includeBatch) {
    float dx = 0.0f;
    float dz = 0.0f;
    float dy = 0.0f;
    for (size_t row = 1; row < historyLength; ++row) {
      for (size_t col = 1; col < binCount; ++col) {
        const size_t index = row * binCount + col;
        if (index >= size) {
          continue;
        }
        const float current = waterfall[index];
        const float left = waterfall[index - 1];
        const float prevRow = waterfall[index - binCount];
        dx += current - left;
        dz += current - prevRow;
        dy += std::abs(current);
      }
    }
    const float denom = static_cast<float>((historyLength - 1) * (binCount - 1));
    result.gradient = {dx / denom, dy / denom, dz / denom};
  }

  return result;
}

}  // namespace uvk
