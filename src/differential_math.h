#pragma once

#include <array>
#include <vector>

namespace uvk {

struct AxisBounds {
  std::array<float, 3> min{};
  std::array<float, 3> max{};
};

struct DifferentialBounds {
  AxisBounds bounds;
  std::array<float, 3> gradient{};
};

class DifferentialMath {
 public:
  static DifferentialBounds analyzeWaterfall(const std::vector<float>& waterfall,
                                             size_t binCount, size_t historyLength,
                                             bool includeBatch);
};

}  // namespace uvk
