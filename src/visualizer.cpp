#include "visualizer.h"

#include <algorithm>
#include <iostream>

namespace uvk {

void Visualizer::initialize(VulkanContext& context) {
  context_ = &context;
}

void Visualizer::update(const SurroundAnalysis& analysis) {
  state_.energy = analysis.energy;
  state_.azimuthDegrees = analysis.azimuthDegrees;
  state_.elevationDegrees = analysis.elevationDegrees;
  std::transform(analysis.rms.begin(), analysis.rms.end(), state_.meterLevels.begin(),
                 [](float value) { return std::min(value, 1.0f); });
}

void Visualizer::renderFrame() {
  if (!context_) {
    return;
  }
  std::cout << "Energy: " << state_.energy << " | Azimuth: " << state_.azimuthDegrees
            << " | Elevation: " << state_.elevationDegrees << '\n';
}

}  // namespace uvk
