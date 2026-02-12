# Vulkan Documentation Index & Actionable Analysis

## 1. 1-Vulkan-Tutorial_English.pdf
**Core Focus**: Standard boilerplate and initial bootstrap.
- **Key Insight**: Emphasizes the `VkPipeline` creation flow and the requirement for `VkDescriptorSetLayout` to match shader `layout(set=X, binding=Y)` exactly.
- **Actionable for Uvkornio**: Our current [vulkan_app.cpp](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/src/vulkan_app.cpp) has a hardcoded descriptor set layout that needs expansion for MVP matrices.

## 2. api-without-secrets-introduction-to-vulkan-part-3-623695.pdf
**Core Focus**: Memory management and Buffer usage.
- **Key Insight**: Highlights the difference between `HOST_VISIBLE` and `DEVICE_LOCAL` memory. Staging buffers are recommended for performance.
- **Actionable for Uvkornio**: The `WaterfallBuffer` is currently `HOST_VISIBLE`. We should implement a staging path in [waterfall_renderer.cpp](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/src/waterfall_renderer.cpp) to move it to `DEVICE_LOCAL` for faster vertex shader reads.

## 3. CS380_fall2024_lecture_17_vulkan_tutorial.pdf
**Core Focus**: Transformations and Coordinate Systems.
- **Key Insight**: Detailed mapping of Model space -> World space -> View space -> Clip space. Specifically mentions the Vulkan-specific Y-flip (NDC Y is down, unlike OpenGL).
- **Actionable for Uvkornio**: Fix the vertex shader projection in [waterfall.vert.glsl](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/shaders/waterfall.vert.glsl) to handle the Vulkan Y-axis correctly during the 3D transition.

## 4. vulkanised-2024-helmut-hlavacs.pdf
**Core Focus**: Advanced Rendering & Performance (Push Constants vs UBOs).
- **Key Insight**: Push constants are optimal for frequently changing data (like frame indices or simple matrices) but limited in size (usually 128-256 bytes).
- **Actionable for Uvkornio**: Use Push Constants for the MVP matrix instead of a full UBO to minimize driver overhead during the real-time audio loop.