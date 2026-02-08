#include "spectrum_analyzer.h"

#include <cmath>

namespace uvk {

float SpectrumAnalyzer::windowedSample(float sample, int index, int size) {
  const float alpha = 0.5f;
  const float phase = static_cast<float>(index) / static_cast<float>(size - 1);
  const float window = alpha - (1.0f - alpha) * std::cos(2.0f * 3.14159265f * phase);
  return sample * window;
}

SpectrumFrame SpectrumAnalyzer::analyze(const SurroundBlock& block, int fftSize) const {
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

  constexpr float kTwoPi = 6.283185307f;
  for (int k = 0; k < binCount; ++k) {
    float real = 0.0f;
    float imag = 0.0f;
    for (int n = 0; n < size; ++n) {
      const float sample = windowedSample(mono[static_cast<size_t>(n)], n, size);
      const float angle = kTwoPi * static_cast<float>(k * n) / static_cast<float>(size);
      real += sample * std::cos(angle);
      imag -= sample * std::sin(angle);
    }
    const float magnitude = std::sqrt(real * real + imag * imag) / static_cast<float>(size);
    frame.magnitudes[static_cast<size_t>(k)] = magnitude;
  }

  const float sampleRate = block.sampleRate > 0.0f ? block.sampleRate : 48000.0f;
  for (int k = 0; k < binCount; ++k) {
    frame.frequenciesHz[static_cast<size_t>(k)] =
        sampleRate * static_cast<float>(k) / static_cast<float>(size);
  }

  return frame;
}

}  // namespace uvk
