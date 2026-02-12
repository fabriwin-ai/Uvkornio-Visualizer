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
  createInstance(appName, {});
  pickPhysicalDevice({});
  createDevice({});
}

void VulkanContext::initializeWithSurface(const std::string& appName,
                                          const std::vector<const char*>& instanceExtensions,
                                          const std::vector<const char*>& deviceExtensions,
                                          VkSurfaceKHR surface) {
  initializeInstance(appName, instanceExtensions);
  initializeDeviceWithSurface(surface, deviceExtensions);
}

void VulkanContext::initializeInstance(const std::string& appName,
                                       const std::vector<const char*>& instanceExtensions) {
  createInstance(appName, instanceExtensions);
}

void VulkanContext::initializeDeviceWithSurface(
    VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
  surface_ = surface;
  pickPhysicalDevice(deviceExtensions);
  createDevice(deviceExtensions);
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

void VulkanContext::createInstance(const std::string& appName,
                                   const std::vector<const char*>& instanceExtensions) {
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
  createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
  createInfo.ppEnabledExtensionNames =
      instanceExtensions.empty() ? nullptr : instanceExtensions.data();

  checkVk(vkCreateInstance(&createInfo, nullptr, &instance_),
          "Failed to create Vulkan instance.");
}

void VulkanContext::pickPhysicalDevice(const std::vector<const char*>& deviceExtensions) {
  uint32_t deviceCount = 0;
  checkVk(vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr),
          "Failed to enumerate Vulkan physical devices.");
  if (deviceCount == 0) {
    throw std::runtime_error("No Vulkan physical devices found.");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  checkVk(vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data()),
          "Failed to enumerate Vulkan physical devices.");

  for (const auto& device : devices) {
    if (isDeviceSuitable(device, deviceExtensions)) {
      physicalDevice_ = device;
      break;
    }
  }
  if (physicalDevice_ == VK_NULL_HANDLE) {
    throw std::runtime_error("No suitable Vulkan physical device found.");
  }
  vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memoryProperties_);
}

void VulkanContext::createDevice(const std::vector<const char*>& deviceExtensions) {
  findQueueFamilies(physicalDevice_);
  std::vector<uint32_t> uniqueFamilies = {graphicsFamilyIndex_};
  if (presentFamilyIndex_ != graphicsFamilyIndex_) {
    uniqueFamilies.push_back(presentFamilyIndex_);
  }

  std::vector<VkDeviceQueueCreateInfo> queueInfos;
  const float priority = 1.0f;
  queueInfos.reserve(uniqueFamilies.size());
  for (uint32_t family : uniqueFamilies) {
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = family;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;
    queueInfos.push_back(queueInfo);
  }

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, families.data());

  VkDeviceCreateInfo deviceInfo{};
  deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
  deviceInfo.pQueueCreateInfos = queueInfos.data();
  deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  deviceInfo.ppEnabledExtensionNames =
      deviceExtensions.empty() ? nullptr : deviceExtensions.data();

  checkVk(vkCreateDevice(physicalDevice_, &deviceInfo, nullptr, &device_),
          "Failed to create Vulkan device.");

  vkGetDeviceQueue(device_, graphicsFamilyIndex_, 0, &graphicsQueue_);
  vkGetDeviceQueue(device_, presentFamilyIndex_, 0, &presentQueue_);
}

bool VulkanContext::isDeviceSuitable(
    VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions) {
  if (!checkDeviceExtensionSupport(device, deviceExtensions)) {
    return false;
  }
  if (surface_ != VK_NULL_HANDLE) {
    VkBool32 presentSupport = VK_FALSE;
    findQueueFamilies(device);
    vkGetPhysicalDeviceSurfaceSupportKHR(device, presentFamilyIndex_, surface_, &presentSupport);
    if (!presentSupport) {
      return false;
    }
  }
  return true;
}

bool VulkanContext::checkDeviceExtensionSupport(
    VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions) const {
  if (deviceExtensions.empty()) {
    return true;
  }
  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> available(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, available.data());

  for (const char* required : deviceExtensions) {
    bool found = false;
    for (const auto& ext : available) {
      if (std::string(ext.extensionName) == required) {
        found = true;
        break;
      }
    }
    if (!found) {
      return false;
    }
  }
  return true;
}

void VulkanContext::findQueueFamilies(VkPhysicalDevice device) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> families(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, families.data());

  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  for (uint32_t i = 0; i < queueFamilyCount; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphicsFamily = i;
    }
    if (surface_ != VK_NULL_HANDLE) {
      VkBool32 presentSupport = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
      if (presentSupport) {
        presentFamily = i;
      }
    }
  }

  if (graphicsFamily.has_value()) {
    graphicsFamilyIndex_ = graphicsFamily.value();
  }
  if (presentFamily.has_value()) {
    presentFamilyIndex_ = presentFamily.value();
  } else {
    presentFamilyIndex_ = graphicsFamilyIndex_;
  }
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
