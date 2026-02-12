#pragma once

#include <array>
#include <vector>

namespace uvk {

struct SurroundBlock {
  std::vector<std::array<float, 8>> samples;
  double timestampSeconds{};
  float sampleRate{};
};

class AudioStream {
 public:
  AudioStream(float sampleRate, int blockSize);

  [[nodiscard]] float sampleRate() const noexcept { return sampleRate_; }
  [[nodiscard]] int blockSize() const noexcept { return blockSize_; }

  SurroundBlock nextBlock();

 private:
  float sampleRate_{};
  int blockSize_{};
  double phase_{};
};

}  // namespace uvk
