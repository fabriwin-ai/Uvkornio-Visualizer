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

## Building
Requires the Vulkan SDK (headers + loader) and a C++20 toolchain.

```bash
cmake -S . -B build
cmake --build build
./build/uvkornio_visualizer
```

## Next steps
- Attach a Vulkan swapchain + render pass for a 3D spectrum waterfall mesh.
- Replace the audio simulator with a real streaming input (ASIO/CoreAudio/ALSA).
- Swap the microphone input stub with a platform capture backend.
- Upload surround analysis to GPU storage buffers for shader-driven visuals.
