#include "visualizer.h"

#include <algorithm>
#include <cstring>
#include <iostream>

namespace uvk {

Visualizer::~Visualizer() {
  shutdown();
}

void Visualizer::initialize(VulkanContext& context, size_t binCount, size_t historyLength) {
  context_ = &context;
  waterfall_.initialize(context, binCount, historyLength);
  analysisBuffer_ = context_->createBuffer(sizeof(float) * 4,
                                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void Visualizer::shutdown() {
  waterfall_.shutdown();
  if (context_) {
    context_->destroyBuffer(analysisBuffer_);
  }
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
  if (context_ && analysisBuffer_.buffer != VK_NULL_HANDLE) {
    void* mapped = nullptr;
    vkMapMemory(context_->device(), analysisBuffer_.memory, 0, analysisBuffer_.size, 0, &mapped);
    const float metrics[4] = {state_.energy, state_.azimuthDegrees, state_.elevationDegrees, 0.0f};
    std::memcpy(mapped, metrics, sizeof(metrics));
    vkUnmapMemory(context_->device(), analysisBuffer_.memory);
  }
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
