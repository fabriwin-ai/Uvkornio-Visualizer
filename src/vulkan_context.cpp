#include "vulkan_context.h"

#include <stdexcept>
#include <vector>

namespace uvk {

namespace {

void checkVk(VkResult result, const char* message) {
  if (result != VK_SUCCESS) {
    throw std::runtime_error(message);
  }
}

}  // namespace

VulkanContext::~VulkanContext() {
  shutdown();
}

void VulkanContext::initialize(const std::string& appName) {
  createInstance(appName);
  pickPhysicalDevice();
  createDevice();
}

void VulkanContext::shutdown() {
  if (device_ != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device_);
    vkDestroyDevice(device_, nullptr);
    device_ = VK_NULL_HANDLE;
  }
  if (instance_ != VK_NULL_HANDLE) {
    vkDestroyInstance(instance_, nullptr);
    instance_ = VK_NULL_HANDLE;
  }
}

void VulkanContext::createInstance(const std::string& appName) {
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName.c_str();
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.pEngineName = "Uvkornio";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  checkVk(vkCreateInstance(&createInfo, nullptr, &instance_),
          "Failed to create Vulkan instance.");
}

void VulkanContext::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  checkVk(vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr),
          "Failed to enumerate Vulkan physical devices.");
  if (deviceCount == 0) {
    throw std::runtime_error("No Vulkan physical devices found.");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  checkVk(vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data()),
          "Failed to enumerate Vulkan physical devices.");

  physicalDevice_ = devices.front();
  vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
}

void VulkanContext::createDevice() {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, families.data());

  uint32_t graphicsIndex = 0;
  for (uint32_t i = 0; i < queueFamilyCount; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsIndex = i;
      break;
    }
  }

  const float priority = 1.0f;
  VkDeviceQueueCreateInfo queueInfo{};
  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueInfo.queueFamilyIndex = graphicsIndex;
  queueInfo.queueCount = 1;
  queueInfo.pQueuePriorities = &priority;

  VkDeviceCreateInfo deviceInfo{};
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.queueCreateInfoCount = 1;
  deviceInfo.pQueueCreateInfos = &queueInfo;

  checkVk(vkCreateDevice(physicalDevice_, &deviceInfo, nullptr, &device_),
          "Failed to create Vulkan device.");
}

uint32_t VulkanContext::findMemoryType(uint32_t typeFilter,
                                       VkMemoryPropertyFlags properties) const {
  for (uint32_t i = 0; i < memoryProperties_.memoryTypeCount; ++i) {
    if ((typeFilter & (1u << i)) &&
        (memoryProperties_.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw std::runtime_error("Failed to find suitable Vulkan memory type.");
}

VulkanBuffer VulkanContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                         VkMemoryPropertyFlags properties) {
  VulkanBuffer buffer{};
  buffer.size = size;

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  checkVk(vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer.buffer),
          "Failed to create Vulkan buffer.");

  VkMemoryRequirements requirements{};
  vkGetBufferMemoryRequirements(device_, buffer.buffer, &requirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = requirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, properties);

  checkVk(vkAllocateMemory(device_, &allocInfo, nullptr, &buffer.memory),
          "Failed to allocate Vulkan buffer memory.");
  checkVk(vkBindBufferMemory(device_, buffer.buffer, buffer.memory, 0),
          "Failed to bind Vulkan buffer memory.");

  return buffer;
}

void VulkanContext::destroyBuffer(VulkanBuffer& buffer) {
  if (buffer.buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(device_, buffer.buffer, nullptr);
    buffer.buffer = VK_NULL_HANDLE;
  }
  if (buffer.memory != VK_NULL_HANDLE) {
    vkFreeMemory(device_, buffer.memory, nullptr);
    buffer.memory = VK_NULL_HANDLE;
  }
  buffer.size = 0;
}

}  // namespace uvk
