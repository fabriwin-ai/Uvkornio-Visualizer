#include "enki_ts.h"
#include "microphone_input.h"
#include "spectrum_analyzer.h"
#include "surround_analyzer.h"
#include "vulkan_app.h"
#include "visualizer.h"
#include "visualizer_presets.h"
#include <iostream>
#include <string>

namespace uvk {

class VisualizerApp {
 public:
  void run(const SpectrumPreset& preset, const std::string& backendName) {
    app_.initialize("Uvkornio Visualizer", 1280, 720);
    const size_t fftSize = static_cast<size_t>(preset.fftSize);
    constexpr size_t kHistoryLength = 120;
    visualizer_.initialize(app_.context(), fftSize / 2, kHistoryLength);
    app_.setWaterfallSource(visualizer_.waterfallBuffer(), visualizer_.waterfallBinCount(),
                            visualizer_.waterfallHistoryLength());
    app_.setAnalysisSource(visualizer_.analysisBuffer());

    MicrophoneInput microphone(48000.0f, 1024);
    microphone.selectBackend(backendName);
    SurroundAnalyzer analyzer;
    SpectrumAnalyzer spectrumAnalyzer;
    EnkiTaskScheduler scheduler;
    scheduler.initialize();
    std::cout << "Preset: " << preset.name << " | Backend: " << microphone.activeBackend()
              << '\n';
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

int main(int argc, char** argv) {
  try {
    std::string presetName = "Wideband";
    std::string backendName = "simulator";
    bool listPresets = false;
    bool listBackends = false;
    for (int i = 1; i < argc; ++i) {
      const std::string arg = argv[i];
      if (arg.rfind("--preset=", 0) == 0) {
        presetName = arg.substr(9);
      } else if (arg.rfind("--backend=", 0) == 0) {
        backendName = arg.substr(10);
      } else if (arg == "--list-presets") {
        listPresets = true;
      } else if (arg == "--list-backends") {
        listBackends = true;
      } else if (arg == "--help") {
        std::cout
            << "Usage: uvkornio_visualizer [--preset=Name] [--backend=simulator|alsa]\n"
               "       uvkornio_visualizer --list-presets\n"
               "       uvkornio_visualizer --list-backends\n";
        return 0;
      }
    }
    if (listPresets) {
      std::cout << "Available presets:\n";
      for (const auto& name : uvk::availablePresetNames()) {
        std::cout << " - " << name << '\n';
      }
      return 0;
    }
    if (listBackends) {
      std::cout << "Available backends:\n"
                   " - simulator\n"
                   " - alsa (if enabled at build time)\n";
      return 0;
    }
    bool presetFound = false;
    const auto preset = uvk::presetByName(presetName, &presetFound);
    if (!presetFound) {
      std::cerr << "Unknown preset '" << presetName << "', falling back to Wideband.\n";
    }
    uvk::VisualizerApp app;
    app.run(preset, backendName);
  } catch (const std::exception& ex) {
    std::cerr << "Visualizer failed: " << ex.what() << '\n';
    return 1;
  }
  return 0;
}
