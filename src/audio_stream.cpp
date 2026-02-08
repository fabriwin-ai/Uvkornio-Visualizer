#include "audio_stream.h"

#include <cmath>

namespace uvk {

AudioStream::AudioStream(float sampleRate, int blockSize)
    : sampleRate_(sampleRate), blockSize_(blockSize) {}

SurroundBlock AudioStream::nextBlock() {
  constexpr float kTwoPi = 6.283185307f;
  SurroundBlock block;
  block.timestampSeconds = phase_ / sampleRate_;
  block.sampleRate = sampleRate_;
  const float baseFrequency = 110.0f;
  block.samples.resize(static_cast<size_t>(blockSize_));

  for (int sampleIndex = 0; sampleIndex < blockSize_; ++sampleIndex) {
    const double sampleTime = (phase_ + sampleIndex) / sampleRate_;
    std::array<float, 8> sample{};
    for (int channel = 0; channel < 8; ++channel) {
      const float channelOffset = static_cast<float>(channel) * 0.15f;
      const float frequency = baseFrequency * (1.0f + channelOffset);
      const float amplitude = 0.5f + 0.5f * std::sin(static_cast<float>(sampleTime) * 0.35f);
      sample[static_cast<size_t>(channel)] =
          amplitude * std::sin(kTwoPi * frequency * static_cast<float>(sampleTime));
    }
    block.samples[static_cast<size_t>(sampleIndex)] = sample;
  }

  phase_ += static_cast<double>(blockSize_);
  return block;
}

}  // namespace uvk
