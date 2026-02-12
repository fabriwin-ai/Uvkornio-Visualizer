#pragma once

#include "spectrum_analyzer.h"
#include "vulkan_context.h"

#include <vector>

namespace uvk {

class WaterfallRenderer {
 public:
  void initialize(VulkanContext& context, size_t binCount, size_t historyLength);
  void shutdown();
  void update(const SpectrumFrame& spectrum);
  void uploadToGpu();

  [[nodiscard]] size_t binCount() const noexcept { return binCount_; }
  [[nodiscard]] size_t historyLength() const noexcept { return historyLength_; }
  [[nodiscard]] const std::vector<float>& waterfall() const noexcept { return waterfall_; }
  [[nodiscard]] const VulkanBuffer& buffer() const noexcept { return buffer_; }

 private:
  VulkanContext* context_{nullptr};
  VulkanBuffer buffer_{};
  size_t binCount_{};
  size_t historyLength_{};
  std::vector<float> waterfall_;
};

}  // namespace uvk
