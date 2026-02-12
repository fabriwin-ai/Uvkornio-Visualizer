#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <string>
#include <vector>

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
  void initializeWithSurface(const std::string& appName,
                             const std::vector<const char*>& instanceExtensions,
                             const std::vector<const char*>& deviceExtensions,
                             VkSurfaceKHR surface);
  void initializeInstance(const std::string& appName,
                          const std::vector<const char*>& instanceExtensions);
  void initializeDeviceWithSurface(VkSurfaceKHR surface,
                                   const std::vector<const char*>& deviceExtensions);
  void shutdown();

  [[nodiscard]] VkInstance instance() const noexcept { return instance_; }
  [[nodiscard]] VkPhysicalDevice physicalDevice() const noexcept { return physicalDevice_; }
  [[nodiscard]] VkDevice device() const noexcept { return device_; }
  [[nodiscard]] VkQueue graphicsQueue() const noexcept { return graphicsQueue_; }
  [[nodiscard]] VkQueue presentQueue() const noexcept { return presentQueue_; }
  [[nodiscard]] uint32_t graphicsFamilyIndex() const noexcept { return graphicsFamilyIndex_; }
  [[nodiscard]] uint32_t presentFamilyIndex() const noexcept { return presentFamilyIndex_; }
  [[nodiscard]] const VkPhysicalDeviceMemoryProperties& memoryProperties() const noexcept {
    return memoryProperties_;
  }

  VulkanBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties);
  void destroyBuffer(VulkanBuffer& buffer);

  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

 private:
  void createInstance(const std::string& appName,
                      const std::vector<const char*>& instanceExtensions);
  void pickPhysicalDevice(const std::vector<const char*>& deviceExtensions);
  void createDevice(const std::vector<const char*>& deviceExtensions);
  bool isDeviceSuitable(VkPhysicalDevice device,
                        const std::vector<const char*>& deviceExtensions);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device,
                                   const std::vector<const char*>& deviceExtensions) const;
  void findQueueFamilies(VkPhysicalDevice device);

  VkInstance instance_{VK_NULL_HANDLE};
  VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
  VkDevice device_{VK_NULL_HANDLE};
  VkQueue graphicsQueue_{VK_NULL_HANDLE};
  VkQueue presentQueue_{VK_NULL_HANDLE};
  uint32_t graphicsFamilyIndex_{0};
  uint32_t presentFamilyIndex_{0};
  VkSurfaceKHR surface_{VK_NULL_HANDLE};
  VkPhysicalDeviceMemoryProperties memoryProperties_{};
};

}  // namespace uvk
