# Technical Debt & Level of Detail (LOD) Analysis

## Summary of Vulkan-Tutorial-English.pdf
*This summary is based on the standard Vulkan tutorial structure identified in the document "1-Vulkan-Tutorial_English.pdf".*

### Key Concepts
- **Vulkan Architecture**: Explicit control over GPU, memory management, and command buffers.
- **Pipeline Stages**: Vertex processing, rasterization, and fragment processing are explicitly defined via `VkPipeline`.
- **Memory Management**: Buffers (UBOs, SSBOs) must be manually allocated and bound.
- **Synchronization**: Fences and Semaphores are critical for CPU-GPU coordination.

## Technical Debt (LOD) Identified
- **Lack of Dynamic Projection**: The current implementation uses hardcoded NDC coordinates in shaders.
- **Manual Descriptor Management**: Descriptor sets are allocated but not dynamically updated for camera/view changes.
- **Vertex Input State**: Using `gl_VertexIndex` without vertex buffers is efficient but limits geometric complexity.
- **Error Handling**: Missing robust check for `VK_ERROR_DEVICE_LOST` or swapchain recreation triggers.

## Recommendations
1. **Implement MVP Matrix**: Add a Push Constant or UBO for Model-View-Projection matrices.
2. **Depth Buffering**: Enable depth testing in the pipeline to support 3D waterfall rendering.
3. **Descriptor Abstraction**: Create a wrapper for descriptor set updates to prevent race conditions.