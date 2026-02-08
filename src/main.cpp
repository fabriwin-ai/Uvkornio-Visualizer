#include "audio_stream.h"
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
    visualizer_.initialize(context_);

    AudioStream stream(48000.0f, 1024);
    SurroundAnalyzer analyzer;

    constexpr int kFrameCount = 120;
    for (int frame = 0; frame < kFrameCount; ++frame) {
      const auto surroundFrame = stream.nextFrame();
      const auto analysis = analyzer.analyze(surroundFrame);
      visualizer_.update(analysis);
      visualizer_.renderFrame();

      std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
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
