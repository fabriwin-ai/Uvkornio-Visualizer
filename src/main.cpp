#include "enki_ts.h"
#include "microphone_input.h"
#include "spectrum_analyzer.h"
#include "surround_analyzer.h"
#include "visualizer.h"
#include "visualizer_presets.h"
#include "vulkan_context.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace uvk {

class VisualizerApp {
 public:
  void run() {
    context_.initialize("Uvkornio Visualizer");
    const auto preset = makeWidebandPreset();
    const size_t fftSize = static_cast<size_t>(preset.fftSize);
    constexpr size_t kHistoryLength = 120;
    visualizer_.initialize(context_, fftSize / 2, kHistoryLength);

    MicrophoneInput microphone(48000.0f, 1024);
    SurroundAnalyzer analyzer;
    SpectrumAnalyzer spectrumAnalyzer;
    EnkiTaskScheduler scheduler;
    scheduler.initialize();

    constexpr int kFrameCount = 120;
    for (int frame = 0; frame < kFrameCount; ++frame) {
      const auto block = microphone.captureBlock();
      const auto analysis = analyzer.analyze(block);
      const auto spectrum = spectrumAnalyzer.analyze(
          block, static_cast<int>(fftSize), preset.bandEdgesHz, &scheduler);
      visualizer_.update(analysis, spectrum);
      visualizer_.renderFrame();

      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    visualizer_.shutdown();
  }

 private:
  VulkanContext context_;
  Visualizer visualizer_;
};

}  // namespace uvk

int main() {
  try {
    uvk::VisualizerApp app;
    app.run();
  } catch (const std::exception& ex) {
    std::cerr << "Visualizer failed: " << ex.what() << '\n';
    return 1;
  }
  return 0;
}
