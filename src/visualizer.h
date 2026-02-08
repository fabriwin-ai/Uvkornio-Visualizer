#pragma once

#include "differential_math.h"
#include "surround_analyzer.h"
#include "vulkan_context.h"
#include "waterfall_renderer.h"

#include <array>

namespace uvk {

struct VisualizerState {
  std::array<float, 8> meterLevels{};
  float energy{};
  float azimuthDegrees{};
  float elevationDegrees{};
  DifferentialBounds bounds{};
};

class Visualizer {
 public:
  ~Visualizer();
  void initialize(VulkanContext& context, size_t binCount, size_t historyLength);
  void shutdown();
  void update(const SurroundAnalysis& analysis, const SpectrumFrame& spectrum);
  void renderFrame();

  [[nodiscard]] const VisualizerState& state() const noexcept { return state_; }
  [[nodiscard]] const VulkanBuffer& waterfallBuffer() const noexcept {
    return waterfall_.buffer();
  }
  [[nodiscard]] size_t waterfallBinCount() const noexcept { return waterfall_.binCount(); }
  [[nodiscard]] size_t waterfallHistoryLength() const noexcept {
    return waterfall_.historyLength();
  }

 private:
  VulkanContext* context_{nullptr};
  VisualizerState state_{};
  WaterfallRenderer waterfall_;
};

}  // namespace uvk
