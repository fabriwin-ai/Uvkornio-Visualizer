#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace uvk {

class VulkanContext {
 public:
  VulkanContext() = default;
  ~VulkanContext();

  VulkanContext(const VulkanContext&) = delete;
  VulkanContext& operator=(const VulkanContext&) = delete;

  void initialize(const std::string& appName);
  void shutdown();

  [[nodiscard]] VkInstance instance() const noexcept { return instance_; }
  [[nodiscard]] VkPhysicalDevice physicalDevice() const noexcept { return physicalDevice_; }
  [[nodiscard]] VkDevice device() const noexcept { return device_; }

 private:
  void createInstance(const std::string& appName);
  void pickPhysicalDevice();
  void createDevice();

  VkInstance instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};
};

}  // namespace uvk
