#include "vulkan_app.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace uvk {

namespace {

VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
  for (const auto& format : formats) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  return formats.front();
}

VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes) {
  for (const auto& mode : modes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
  if (capabilities.currentExtent.width != UINT32_MAX) {
    return capabilities.currentExtent;
  }
  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  VkExtent2D extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width);
  extent.height = std::clamp(extent.height, capabilities.minImageExtent.height,
                             capabilities.maxImageExtent.height);
  return extent;
}

}  // namespace

VulkanApp::~VulkanApp() {
  shutdown();
}

void VulkanApp::initialize(const std::string& title, int width, int height) {
  initWindow(title, width, height);

  std::vector<const char*> instanceExtensions;
  uint32_t extensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
  instanceExtensions.assign(glfwExtensions, glfwExtensions + extensionCount);
  std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  context_.initializeInstance(title, instanceExtensions);
  createSurface();
  context_.initializeDeviceWithSurface(surface_, deviceExtensions);
  createSwapchain();
  createImageViews();
  createRenderPass();
  createDescriptorSetLayout();
  createPipeline();
  createFramebuffers();
  createCommandPool();
  createCommandBuffers();
  createSyncObjects();
  initialized_ = true;
}

void VulkanApp::setWaterfallSource(const VulkanBuffer& buffer, size_t binCount,
                                   size_t historyLength) {
  waterfallBuffer_ = buffer;
  waterfallBinCount_ = binCount;
  waterfallHistoryLength_ = historyLength;
  if (descriptorSetLayout_ == VK_NULL_HANDLE) {
    createDescriptorSetLayout();
  }
  if (descriptorPool_ == VK_NULL_HANDLE) {
    createDescriptorPool();
  }
  createDescriptorSet();
}

void VulkanApp::run(const std::function<void()>& perFrame) {
  while (window_ && !glfwWindowShouldClose(window_)) {
    glfwPollEvents();
    if (perFrame) {
      perFrame();
    }
    drawFrame();
  }
  if (context_.device() != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(context_.device());
  }
}

void VulkanApp::shutdown() {
  if (!initialized_) {
    return;
  }
  VkDevice device = context_.device();
  if (device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(device);
  }
  if (imageAvailableSemaphore_ != VK_NULL_HANDLE) {
    vkDestroySemaphore(device, imageAvailableSemaphore_, nullptr);
  }
  if (renderFinishedSemaphore_ != VK_NULL_HANDLE) {
    vkDestroySemaphore(device, renderFinishedSemaphore_, nullptr);
  }
  if (inFlightFence_ != VK_NULL_HANDLE) {
    vkDestroyFence(device, inFlightFence_, nullptr);
  }
  if (commandPool_ != VK_NULL_HANDLE) {
    vkDestroyCommandPool(device, commandPool_, nullptr);
  }
  if (descriptorPool_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, descriptorPool_, nullptr);
    descriptorPool_ = VK_NULL_HANDLE;
  }
  if (descriptorSetLayout_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout_, nullptr);
    descriptorSetLayout_ = VK_NULL_HANDLE;
  }
  cleanupSwapchain();
  if (surface_ != VK_NULL_HANDLE && context_.instance() != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(context_.instance(), surface_, nullptr);
    surface_ = VK_NULL_HANDLE;
  }
  context_.shutdown();
  if (window_) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
    glfwTerminate();
  }
  initialized_ = false;
}

void VulkanApp::initWindow(const std::string& title, int width, int height) {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW.");
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!window_) {
    throw std::runtime_error("Failed to create GLFW window.");
  }
}

void VulkanApp::createSurface() {
  if (glfwCreateWindowSurface(context_.instance(), window_, nullptr, &surface_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan surface.");
  }
}

void VulkanApp::createSwapchain() {
  VkSurfaceCapabilitiesKHR capabilities{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context_.physicalDevice(), surface_, &capabilities);

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(context_.physicalDevice(), surface_, &formatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(context_.physicalDevice(), surface_, &formatCount,
                                       formats.data());

  uint32_t presentCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(context_.physicalDevice(), surface_, &presentCount,
                                            nullptr);
  std::vector<VkPresentModeKHR> presentModes(presentCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(context_.physicalDevice(), surface_, &presentCount,
                                            presentModes.data());

  const VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
  const VkPresentModeKHR presentMode = choosePresentMode(presentModes);
  const VkExtent2D extent = chooseExtent(capabilities, window_);

  uint32_t imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface_;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {context_.graphicsFamilyIndex(),
                                   context_.presentFamilyIndex()};
  if (context_.graphicsFamilyIndex() != context_.presentFamilyIndex()) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(context_.device(), &createInfo, nullptr, &swapchain_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create swapchain.");
  }

  vkGetSwapchainImagesKHR(context_.device(), swapchain_, &imageCount, nullptr);
  swapchainImages_.resize(imageCount);
  vkGetSwapchainImagesKHR(context_.device(), swapchain_, &imageCount, swapchainImages_.data());
  swapchainImageFormat_ = surfaceFormat.format;
  swapchainExtent_ = extent;
}

void VulkanApp::createImageViews() {
  swapchainImageViews_.resize(swapchainImages_.size());
  for (size_t i = 0; i < swapchainImages_.size(); ++i) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = swapchainImages_[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapchainImageFormat_;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(context_.device(), &viewInfo, nullptr, &swapchainImageViews_[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create image view.");
    }
  }
}

void VulkanApp::createRenderPass() {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapchainImageFormat_;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(context_.device(), &renderPassInfo, nullptr, &renderPass_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass.");
  }
}

void VulkanApp::createDescriptorSetLayout() {
  if (descriptorSetLayout_ != VK_NULL_HANDLE) {
    return;
  }
  VkDescriptorSetLayoutBinding storageBinding{};
  storageBinding.binding = 0;
  storageBinding.descriptorCount = 1;
  storageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  storageBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &storageBinding;

  if (vkCreateDescriptorSetLayout(context_.device(), &layoutInfo, nullptr,
                                  &descriptorSetLayout_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout.");
  }
}

void VulkanApp::createDescriptorPool() {
  if (descriptorPool_ != VK_NULL_HANDLE) {
    return;
  }
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSize.descriptorCount = 1;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.maxSets = 1;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;

  if (vkCreateDescriptorPool(context_.device(), &poolInfo, nullptr, &descriptorPool_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool.");
  }
}

void VulkanApp::createDescriptorSet() {
  if (descriptorPool_ == VK_NULL_HANDLE || descriptorSetLayout_ == VK_NULL_HANDLE ||
      waterfallBuffer_.buffer == VK_NULL_HANDLE) {
    return;
  }

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool_;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descriptorSetLayout_;

  if (vkAllocateDescriptorSets(context_.device(), &allocInfo, &descriptorSet_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor set.");
  }

  VkDescriptorBufferInfo bufferInfo{};
  bufferInfo.buffer = waterfallBuffer_.buffer;
  bufferInfo.offset = 0;
  bufferInfo.range = waterfallBuffer_.size;

  VkWriteDescriptorSet descriptorWrite{};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = descriptorSet_;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  descriptorWrite.pBufferInfo = &bufferInfo;

  vkUpdateDescriptorSets(context_.device(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanApp::createPipeline() {
  const auto vertShaderCode = readFile("shaders/waterfall.vert.spv");
  const auto fragShaderCode = readFile("shaders/waterfall.frag.spv");

  VkShaderModule vertModule = createShaderModule(vertShaderCode);
  VkShaderModule fragModule = createShaderModule(fragShaderCode);

  VkPipelineShaderStageCreateInfo vertStage{};
  vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertStage.module = vertModule;
  vertStage.pName = "main";

  VkPipelineShaderStageCreateInfo fragStage{};
  fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragStage.module = fragModule;
  fragStage.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertStage, fragStage};

  VkPipelineVertexInputStateCreateInfo vertexInput{};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(swapchainExtent_.width);
  viewport.height = static_cast<float>(swapchainExtent_.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = swapchainExtent_;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;

  VkPushConstantRange pushConstant{};
  pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  pushConstant.offset = 0;
  pushConstant.size = sizeof(float) * 2;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

  if (vkCreatePipelineLayout(context_.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout.");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInput;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = pipelineLayout_;
  pipelineInfo.renderPass = renderPass_;
  pipelineInfo.subpass = 0;

  if (vkCreateGraphicsPipelines(context_.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                &graphicsPipeline_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create graphics pipeline.");
  }

  vkDestroyShaderModule(context_.device(), fragModule, nullptr);
  vkDestroyShaderModule(context_.device(), vertModule, nullptr);
}

void VulkanApp::createFramebuffers() {
  swapchainFramebuffers_.resize(swapchainImageViews_.size());
  for (size_t i = 0; i < swapchainImageViews_.size(); ++i) {
    VkImageView attachments[] = {swapchainImageViews_[i]};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass_;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = swapchainExtent_.width;
    framebufferInfo.height = swapchainExtent_.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(context_.device(), &framebufferInfo, nullptr,
                            &swapchainFramebuffers_[i]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer.");
    }
  }
}

void VulkanApp::createCommandPool() {
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = context_.graphicsFamilyIndex();
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (vkCreateCommandPool(context_.device(), &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create command pool.");
  }
}

void VulkanApp::createCommandBuffers() {
  commandBuffers_.resize(swapchainFramebuffers_.size());
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool_;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

  if (vkAllocateCommandBuffers(context_.device(), &allocInfo, commandBuffers_.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers.");
  }
}

void VulkanApp::createSyncObjects() {
  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  if (vkCreateSemaphore(context_.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphore_) !=
      VK_SUCCESS ||
      vkCreateSemaphore(context_.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphore_) !=
          VK_SUCCESS ||
      vkCreateFence(context_.device(), &fenceInfo, nullptr, &inFlightFence_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create sync objects.");
  }
}

void VulkanApp::cleanupSwapchain() {
  VkDevice device = context_.device();
  for (auto framebuffer : swapchainFramebuffers_) {
    vkDestroyFramebuffer(device, framebuffer, nullptr);
  }
  swapchainFramebuffers_.clear();
  if (graphicsPipeline_ != VK_NULL_HANDLE) {
    vkDestroyPipeline(device, graphicsPipeline_, nullptr);
    graphicsPipeline_ = VK_NULL_HANDLE;
  }
  if (pipelineLayout_ != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device, pipelineLayout_, nullptr);
    pipelineLayout_ = VK_NULL_HANDLE;
  }
  if (renderPass_ != VK_NULL_HANDLE) {
    vkDestroyRenderPass(device, renderPass_, nullptr);
    renderPass_ = VK_NULL_HANDLE;
  }
  if (descriptorPool_ != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(device, descriptorPool_, nullptr);
    descriptorPool_ = VK_NULL_HANDLE;
    descriptorSet_ = VK_NULL_HANDLE;
  }
  for (auto imageView : swapchainImageViews_) {
    vkDestroyImageView(device, imageView, nullptr);
  }
  swapchainImageViews_.clear();
  if (swapchain_ != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(device, swapchain_, nullptr);
    swapchain_ = VK_NULL_HANDLE;
  }
}

void VulkanApp::recreateSwapchain() {
  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window_, &width, &height);
  if (width == 0 || height == 0) {
    return;
  }
  vkDeviceWaitIdle(context_.device());
  cleanupSwapchain();
  createSwapchain();
  createImageViews();
  createRenderPass();
  if (waterfallBuffer_.buffer != VK_NULL_HANDLE) {
    createDescriptorPool();
    createDescriptorSet();
  }
  createPipeline();
  createFramebuffers();
  createCommandBuffers();
}

void VulkanApp::drawFrame() {
  vkWaitForFences(context_.device(), 1, &inFlightFence_, VK_TRUE, UINT64_MAX);
  vkResetFences(context_.device(), 1, &inFlightFence_);

  uint32_t imageIndex = 0;
  VkResult result = vkAcquireNextImageKHR(context_.device(), swapchain_, UINT64_MAX,
                                          imageAvailableSemaphore_, VK_NULL_HANDLE, &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain();
    return;
  }

  VkCommandBuffer commandBuffer = commandBuffers_[imageIndex];
  vkResetCommandBuffer(commandBuffer, 0);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass_;
  renderPassInfo.framebuffer = swapchainFramebuffers_[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = swapchainExtent_;
  VkClearValue clearColor{{0.05f, 0.05f, 0.08f, 1.0f}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
  if (descriptorSet_ != VK_NULL_HANDLE) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1,
                            &descriptorSet_, 0, nullptr);
  }
  const float pushConstants[2] = {static_cast<float>(waterfallBinCount_),
                                  static_cast<float>(waterfallHistoryLength_)};
  vkCmdPushConstants(commandBuffer, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                     sizeof(pushConstants), pushConstants);
  const uint32_t vertexCount =
      static_cast<uint32_t>(waterfallBinCount_ * waterfallHistoryLength_);
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  vkCmdEndRenderPass(commandBuffer);

  vkEndCommandBuffer(commandBuffer);

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphore_};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSemaphore signalSemaphores[] = {renderFinishedSemaphore_};

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(context_.graphicsQueue(), 1, &submitInfo, inFlightFence_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to submit draw command buffer.");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;
  VkSwapchainKHR swapchains[] = {swapchain_};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapchains;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(context_.presentQueue(), &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
    recreateSwapchain();
  }
}

std::vector<char> VulkanApp::readFile(const std::string& path) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file) {
    throw std::runtime_error("Failed to open shader file: " + path);
  }
  const size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
  return buffer;
}

VkShaderModule VulkanApp::createShaderModule(const std::vector<char>& code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule = VK_NULL_HANDLE;
  if (vkCreateShaderModule(context_.device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create shader module.");
  }
  return shaderModule;
}

}  // namespace uvk
