#include "enki_ts.h"
#include "microphone_input.h"
#include "spectrum_analyzer.h"
#include "surround_analyzer.h"
#include "vulkan_app.h"
#include "visualizer.h"
#include "visualizer_presets.h"
#include <iostream>

namespace uvk {

class VisualizerApp {
 public:
  void run() {
    app_.initialize("Uvkornio Visualizer", 1280, 720);
    const auto preset = makeWidebandPreset();
    const size_t fftSize = static_cast<size_t>(preset.fftSize);
    constexpr size_t kHistoryLength = 120;
    visualizer_.initialize(app_.context(), fftSize / 2, kHistoryLength);

    MicrophoneInput microphone(48000.0f, 1024);
    SurroundAnalyzer analyzer;
    SpectrumAnalyzer spectrumAnalyzer;
    EnkiTaskScheduler scheduler;
    scheduler.initialize();
    app_.run([&]() {
      const auto block = microphone.captureBlock();
      const auto analysis = analyzer.analyze(block);
      const auto spectrum = spectrumAnalyzer.analyze(
          block, static_cast<int>(fftSize), preset.bandEdgesHz, &scheduler);
      visualizer_.update(analysis, spectrum);
    });

    visualizer_.shutdown();
    app_.shutdown();
  }

 private:
  VulkanApp app_;
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
