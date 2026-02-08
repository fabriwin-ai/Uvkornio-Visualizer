#include "visualizer.h"

#include <algorithm>
#include <iostream>

namespace uvk {

Visualizer::~Visualizer() {
  shutdown();
}

void Visualizer::initialize(VulkanContext& context, size_t binCount, size_t historyLength) {
  context_ = &context;
  waterfall_.initialize(context, binCount, historyLength);
}

void Visualizer::shutdown() {
  waterfall_.shutdown();
  context_ = nullptr;
}

void Visualizer::update(const SurroundAnalysis& analysis, const SpectrumFrame& spectrum) {
  state_.energy = analysis.energy;
  state_.azimuthDegrees = analysis.azimuthDegrees;
  state_.elevationDegrees = analysis.elevationDegrees;
  std::transform(analysis.rms.begin(), analysis.rms.end(), state_.meterLevels.begin(),
                 [](float value) { return std::min(value, 1.0f); });
  waterfall_.update(spectrum);
  waterfall_.uploadToGpu();
  state_.bounds = DifferentialMath::analyzeWaterfall(
      waterfall_.waterfall(), waterfall_.binCount(), waterfall_.historyLength(), true);
}

void Visualizer::renderFrame() {
  if (!context_) {
    return;
  }
  std::cout << "Energy: " << state_.energy << " | Azimuth: " << state_.azimuthDegrees
            << " | Elevation: " << state_.elevationDegrees << " | Bounds Y: ["
            << state_.bounds.bounds.min[1] << ", " << state_.bounds.bounds.max[1] << "]"
            << '\n';
}

}  // namespace uvk
