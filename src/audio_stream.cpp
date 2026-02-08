#include "audio_stream.h"

#include <cmath>

namespace uvk {

AudioStream::AudioStream(float sampleRate, int blockSize)
    : sampleRate_(sampleRate), blockSize_(blockSize) {}

SurroundFrame AudioStream::nextFrame() {
  constexpr float kTwoPi = 6.283185307f;
  SurroundFrame frame;
  frame.timestampSeconds = phase_ / sampleRate_;
  const float baseFrequency = 110.0f;

  for (int channel = 0; channel < 8; ++channel) {
    const float channelOffset = static_cast<float>(channel) * 0.15f;
    const float frequency = baseFrequency * (1.0f + channelOffset);
    const float amplitude = 0.5f + 0.5f * std::sin(phase_ * 0.0005);
    const float sample = amplitude * std::sin(kTwoPi * frequency * frame.timestampSeconds);
    frame.channels[static_cast<size_t>(channel)] = sample;
  }

  phase_ += static_cast<double>(blockSize_);
  return frame;
}

}  // namespace uvk
