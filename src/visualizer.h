#pragma once

#include "surround_analyzer.h"
#include "vulkan_context.h"

#include <array>

namespace uvk {

struct VisualizerState {
  std::array<float, 8> meterLevels{};
  float energy{};
  float azimuthDegrees{};
  float elevationDegrees{};
};

class Visualizer {
 public:
  void initialize(VulkanContext& context);
  void update(const SurroundAnalysis& analysis);
  void renderFrame();

  [[nodiscard]] const VisualizerState& state() const noexcept { return state_; }

 private:
  VulkanContext* context_{nullptr};
  VisualizerState state_{};
};

}  // namespace uvk
