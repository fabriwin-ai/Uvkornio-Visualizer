#pragma once

#include "audio_stream.h"

#include <array>
#include <string>

namespace uvk {

struct SurroundAnalysis {
  std::array<float, 8> rms{};
  float energy{};
  float azimuthDegrees{};
  float elevationDegrees{};
  std::string dominantChannel;
};

class SurroundAnalyzer {
 public:
  SurroundAnalysis analyze(const SurroundBlock& block) const;
};

}  // namespace uvk
