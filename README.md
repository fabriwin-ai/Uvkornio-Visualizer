# Uvkornio-Visualizer

Native C++ Vulkan framework prototype for real-time surround audio reporting and visualization.

## What this provides
- A lightweight Vulkan bootstrap (instance + device) to host native rendering pipelines.
- A streaming audio simulator that produces 7.1 surround frames.
- A surround analyzer that converts per-channel energy into azimuth/elevation cues.
- A visualizer loop that can be replaced with Vulkan render passes, shaders, and GPU buffers.

## Building
Requires the Vulkan SDK (headers + loader) and a C++20 toolchain.

```bash
cmake -S . -B build
cmake --build build
./build/uvkornio_visualizer
```

## Next steps
- Attach a Vulkan swapchain + render pass for a 3D spectrum waterfall.
- Replace the audio simulator with a real streaming input (ASIO/CoreAudio/ALSA).
- Upload surround analysis to GPU storage buffers for shader-driven visuals.
