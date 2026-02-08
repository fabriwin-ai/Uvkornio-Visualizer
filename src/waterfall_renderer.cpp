#include "waterfall_renderer.h"

#include <algorithm>
#include <cstring>

namespace uvk {

void WaterfallRenderer::initialize(VulkanContext& context, size_t binCount, size_t historyLength) {
  if (context_) {
    shutdown();
  }
  context_ = &context;
  binCount_ = binCount;
  historyLength_ = historyLength;
  waterfall_.assign(binCount_ * historyLength_, 0.0f);

  if (context_) {
    const VkDeviceSize size = sizeof(float) * waterfall_.size();
    buffer_ = context_->createBuffer(
        size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  }
}

void WaterfallRenderer::shutdown() {
  if (context_) {
    context_->destroyBuffer(buffer_);
  }
  context_ = nullptr;
  waterfall_.clear();
  binCount_ = 0;
  historyLength_ = 0;
}

void WaterfallRenderer::update(const SpectrumFrame& spectrum) {
  if (binCount_ == 0 || historyLength_ == 0) {
    return;
  }

  const size_t rowSize = binCount_;
  const size_t dataSize = rowSize * historyLength_;
  if (waterfall_.size() != dataSize) {
    waterfall_.assign(dataSize, 0.0f);
  }

  for (size_t row = historyLength_ - 1; row > 0; --row) {
    std::copy_n(&waterfall_[(row - 1) * rowSize], rowSize, &waterfall_[row * rowSize]);
  }

  const size_t copyCount = std::min(rowSize, spectrum.magnitudes.size());
  std::fill_n(waterfall_.begin(), rowSize, 0.0f);
  std::copy_n(spectrum.magnitudes.begin(), copyCount, waterfall_.begin());
}

void WaterfallRenderer::uploadToGpu() {
  if (!context_ || buffer_.buffer == VK_NULL_HANDLE) {
    return;
  }
  void* mapped = nullptr;
  vkMapMemory(context_->device(), buffer_.memory, 0, buffer_.size, 0, &mapped);
  std::memcpy(mapped, waterfall_.data(), buffer_.size);
  vkUnmapMemory(context_->device(), buffer_.memory);
}

}  // namespace uvk
