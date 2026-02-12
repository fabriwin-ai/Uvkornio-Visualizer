# Actionable Integration Plan: 3D Waterfall Evolution

## Phase 1: 3D Math & Projection (Short Term)
- [ ] **Implement CPU MVP Calculation**: Add simple `mat4` utilities to [vulkan_app.cpp](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/src/vulkan_app.cpp) for perspective and view matrices.
- [ ] **Expand Push Constants**: Update `VkPushConstantRange` in [vulkan_app.cpp](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/src/vulkan_app.cpp#L481) to accommodate a `mat4` (64 bytes).
- [ ] **Shader Projection**: Modify [waterfall.vert.glsl](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/shaders/waterfall.vert.glsl) to apply the MVP matrix to the vertex position.

## Phase 2: Pipeline Robustness (Medium Term)
- [ ] **Enable Depth Testing**: Create a depth image/view and update the `VkPipelineDepthStencilStateCreateInfo` in [vulkan_app.cpp](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/src/vulkan_app.cpp#L450).
- [ ] **Descriptor Abstraction**: Refactor `createDescriptorSet` to handle dynamic buffer updates without pool exhaustion.
- [ ] **Staging Buffer Integration**: Update [waterfall_renderer.cpp](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/src/waterfall_renderer.cpp) to use a staging buffer for `DEVICE_LOCAL` storage.

## Phase 3: Visual Polish (Long Term)
- [ ] **Color Mapping Optimization**: Move color calculation from [waterfall.vert.glsl](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/shaders/waterfall.vert.glsl#L35) to a lookup texture for better performance.
- [ ] **Surround Visualization**: Map the surround analysis metrics from [AnalysisBuffer](file:///c:/Users/53890197/Documents/Uvkornio-Visualizer-codex-create-vulkan-audio-visualizer-framework/shaders/waterfall.vert.glsl#L7) to 3D sphere positioning in the UI.