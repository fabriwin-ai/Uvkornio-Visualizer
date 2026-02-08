#include "audio_stream.h"
#include "spectrum_analyzer.h"
#include "surround_analyzer.h"
#include "visualizer.h"
#include "vulkan_context.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace uvk {

class VisualizerApp {
 public:
  void run() {
    context_.initialize("Uvkornio Visualizer");
    constexpr size_t kFftSize = 256;
    constexpr size_t kHistoryLength = 120;
    visualizer_.initialize(context_, kFftSize / 2, kHistoryLength);

    AudioStream stream(48000.0f, 1024);
    SurroundAnalyzer analyzer;
    SpectrumAnalyzer spectrumAnalyzer;

    constexpr int kFrameCount = 120;
    for (int frame = 0; frame < kFrameCount; ++frame) {
      const auto block = stream.nextBlock();
      const auto analysis = analyzer.analyze(block);
      const auto spectrum = spectrumAnalyzer.analyze(block, static_cast<int>(kFftSize));
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
