#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace uvk {

struct VulkanBuffer {
  VkBuffer buffer{VK_NULL_HANDLE};
  VkDeviceMemory memory{VK_NULL_HANDLE};
  VkDeviceSize size{};
};

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
  [[nodiscard]] const VkPhysicalDeviceMemoryProperties& memoryProperties() const noexcept {
    return memoryProperties_;
  }

  VulkanBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties);
  void destroyBuffer(VulkanBuffer& buffer);

 private:
  void createInstance(const std::string& appName);
  void pickPhysicalDevice();
  void createDevice();
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

  VkInstance instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};
  VkPhysicalDeviceMemoryProperties memoryProperties_{};
};

}  // namespace uvk
