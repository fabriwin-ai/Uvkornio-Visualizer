# Uvkornio-Visualizer

Native C++ Vulkan framework prototype for real-time surround audio reporting and visualization.

## What this provides
- A lightweight Vulkan bootstrap (instance + device) to host native rendering pipelines.
- A streaming audio simulator that produces 7.1 surround blocks with real Hz sample timing.
- A surround analyzer that converts per-channel energy into azimuth/elevation cues.
- A spectrum analyzer and waterfall ring buffer ready for Vulkan-driven 3D visuals.
- EnkiTS-style async task scheduling for parallel Hz-band analysis.
- A visualizer loop that uploads waterfall data into a Vulkan storage buffer.
- A microphone input abstraction (currently backed by the simulator).
- Differential math utilities to bound X/Y/Z vectors for streaming waterfall volumes.
- A GLFW-backed Vulkan swapchain and triangle baseline render pass.

## Building
Requires the Vulkan SDK (headers + loader), GLFW, and a C++20 toolchain.

```bash
cmake -S . -B build
cmake --build build
./build/uvkornio_visualizer
```

Compile the baseline shaders (from the Vulkan SDK tutorial) before running:

```bash
glslc shaders/triangle.vert.glsl -o shaders/triangle.vert.spv
glslc shaders/triangle.frag.glsl -o shaders/triangle.frag.spv
glslc shaders/waterfall.vert.glsl -o shaders/waterfall.vert.spv
glslc shaders/waterfall.frag.glsl -o shaders/waterfall.frag.spv
```

## Next steps
- Attach a Vulkan swapchain + render pass for a 3D spectrum waterfall mesh.
- Use a triangle-setup tutorial from the Vulkan SDK docs as a baseline for the swapchain render path.
- Replace the audio simulator with a real streaming input (ASIO/CoreAudio/ALSA).
- Swap the microphone input stub with a platform capture backend.
- Upload surround analysis to GPU storage buffers for shader-driven visuals.
