#pragma once

#include "vulkan_context.h"

#include <GLFW/glfw3.h>

#include <functional>
#include <string>
#include <vector>

namespace uvk {

class VulkanApp {
 public:
  VulkanApp() = default;
  ~VulkanApp();

  VulkanApp(const VulkanApp&) = delete;
  VulkanApp& operator=(const VulkanApp&) = delete;

  void initialize(const std::string& title, int width, int height);
  void setWaterfallSource(const VulkanBuffer& buffer, size_t binCount, size_t historyLength);
  void setAnalysisSource(const VulkanBuffer& buffer);
  void run(const std::function<void()>& perFrame);
  void shutdown();

  [[nodiscard]] VulkanContext& context() noexcept { return context_; }

 private:
  void initWindow(const std::string& title, int width, int height);
  void createSurface();
  void createSwapchain();
  void createImageViews();
  void createRenderPass();
  void createPipeline();
  void createDescriptorSetLayout();
  void createDescriptorPool();
  void createDescriptorSet();
  void createFramebuffers();
  void createDepthResources();
  void createCommandPool();
  void createCommandBuffers();
  void createSyncObjects();
  void cleanupSwapchain();
  void recreateSwapchain();
  void drawFrame();
  void updateMVP();

  struct MVP {
    float projection[16];
    float view[16];
    float model[16];
  } mvp_{};

  static std::vector<char> readFile(const std::string& path);
  VkShaderModule createShaderModule(const std::vector<char>& code);

  GLFWwindow* window_{nullptr};
  VkSurfaceKHR surface_{VK_NULL_HANDLE};
  VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
  std::vector<VkImage> swapchainImages_;
  VkFormat swapchainImageFormat_{};
  VkExtent2D swapchainExtent_{};
  std::vector<VkImageView> swapchainImageViews_;
  VkRenderPass renderPass_{VK_NULL_HANDLE};
  VkDescriptorSetLayout descriptorSetLayout_{VK_NULL_HANDLE};
  VkDescriptorPool descriptorPool_{VK_NULL_HANDLE};
  VkDescriptorSet descriptorSet_{VK_NULL_HANDLE};
  VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE};
  VkPipeline graphicsPipeline_{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> swapchainFramebuffers_;
  VkCommandPool commandPool_{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> commandBuffers_;
  VkSemaphore imageAvailableSemaphore_{VK_NULL_HANDLE};
  VkSemaphore renderFinishedSemaphore_{VK_NULL_HANDLE};
  VkFence inFlightFence_{VK_NULL_HANDLE};
  VkImageView depthImageView_{VK_NULL_HANDLE};
  VkImage depthImage_{VK_NULL_HANDLE};
  VkDeviceMemory depthImageMemory_{VK_NULL_HANDLE};

  VulkanContext context_;
  VulkanBuffer waterfallBuffer_{};
  VulkanBuffer analysisBuffer_{};
  size_t waterfallBinCount_{0};
  size_t waterfallHistoryLength_{0};
  bool initialized_{false};
};

}  // namespace uvk
