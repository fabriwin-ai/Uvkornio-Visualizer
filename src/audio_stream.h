#pragma once

#include <array>

namespace uvk {

struct SurroundFrame {
  std::array<float, 8> channels{};
  double timestampSeconds{};
};

class AudioStream {
 public:
  AudioStream(float sampleRate, int blockSize);

  [[nodiscard]] float sampleRate() const noexcept { return sampleRate_; }
  [[nodiscard]] int blockSize() const noexcept { return blockSize_; }

  SurroundFrame nextFrame();

 private:
  float sampleRate_{};
  int blockSize_{};
  double phase_{};
};

}  // namespace uvk
