#include "spectrum_analyzer.h"

#include <cmath>

namespace uvk {

float SpectrumAnalyzer::windowedSample(float sample, int index, int size) {
  const float alpha = 0.5f;
  const float phase = static_cast<float>(index) / static_cast<float>(size - 1);
  const float window = alpha - (1.0f - alpha) * std::cos(2.0f * 3.14159265f * phase);
  return sample * window;
}

void SpectrumAnalyzer::computeBins(const std::vector<float>& mono, int size, int binStart,
                                   int binEnd, std::vector<float>& magnitudes) {
  constexpr float kTwoPi = 6.283185307f;
  for (int k = binStart; k < binEnd; ++k) {
    float real = 0.0f;
    float imag = 0.0f;
    for (int n = 0; n < size; ++n) {
      const float sample = windowedSample(mono[static_cast<size_t>(n)], n, size);
      const float angle = kTwoPi * static_cast<float>(k * n) / static_cast<float>(size);
      real += sample * std::cos(angle);
      imag -= sample * std::sin(angle);
    }
    const float magnitude = std::sqrt(real * real + imag * imag) / static_cast<float>(size);
    magnitudes[static_cast<size_t>(k)] = magnitude;
  }
}

SpectrumFrame SpectrumAnalyzer::analyze(const SurroundBlock& block, int fftSize) const {
  return analyze(block, fftSize, {}, nullptr);
}

SpectrumFrame SpectrumAnalyzer::analyze(const SurroundBlock& block, int fftSize,
                                        const std::vector<float>& bandEdgesHz,
                                        EnkiTaskScheduler* scheduler) const {
  SpectrumFrame frame{};
  if (block.samples.empty() || fftSize <= 0) {
    return frame;
  }

  const int size = std::min<int>(fftSize, static_cast<int>(block.samples.size()));
  const int binCount = size / 2;
  frame.magnitudes.assign(static_cast<size_t>(binCount), 0.0f);
  frame.frequenciesHz.assign(static_cast<size_t>(binCount), 0.0f);

  std::vector<float> mono(static_cast<size_t>(size), 0.0f);
  for (int i = 0; i < size; ++i) {
    const auto& sample = block.samples[static_cast<size_t>(i)];
    float sum = 0.0f;
    for (float channelSample : sample) {
      sum += channelSample;
    }
    mono[static_cast<size_t>(i)] = sum / static_cast<float>(sample.size());
  }

  if (scheduler && scheduler->threadCount() > 1) {
    const int taskCount = static_cast<int>(scheduler->threadCount());
    const int binsPerTask = std::max(1, binCount / taskCount);
    std::vector<std::future<void>> tasks;
    for (int taskIndex = 0; taskIndex < taskCount; ++taskIndex) {
      const int start = taskIndex * binsPerTask;
      const int end = (taskIndex == taskCount - 1) ? binCount : start + binsPerTask;
      if (start >= binCount) {
        break;
      }
      tasks.emplace_back(scheduler->addTask(
          [&, start, end]() { computeBins(mono, size, start, end, frame.magnitudes); }));
    }
    scheduler->waitAll(tasks);
  } else {
    computeBins(mono, size, 0, binCount, frame.magnitudes);
  }

  const float sampleRate = block.sampleRate > 0.0f ? block.sampleRate : 48000.0f;
  for (int k = 0; k < binCount; ++k) {
    frame.frequenciesHz[static_cast<size_t>(k)] =
        sampleRate * static_cast<float>(k) / static_cast<float>(size);
  }

  if (!bandEdgesHz.empty() && bandEdgesHz.size() > 1) {
    frame.bandCentersHz.reserve(bandEdgesHz.size() - 1);
    frame.bandEnergies.assign(bandEdgesHz.size() - 1, 0.0f);
    for (size_t band = 0; band + 1 < bandEdgesHz.size(); ++band) {
      const float startHz = bandEdgesHz[band];
      const float endHz = bandEdgesHz[band + 1];
      frame.bandCentersHz.push_back(0.5f * (startHz + endHz));
      for (int k = 0; k < binCount; ++k) {
        const float freq = frame.frequenciesHz[static_cast<size_t>(k)];
        if (freq >= startHz && freq < endHz) {
          frame.bandEnergies[band] += frame.magnitudes[static_cast<size_t>(k)];
        }
      }
    }
  }

  return frame;
}

}  // namespace uvk
