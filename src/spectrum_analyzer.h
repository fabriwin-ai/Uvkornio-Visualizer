#pragma once

#include "audio_stream.h"

#include <vector>

namespace uvk {

struct SpectrumFrame {
  std::vector<float> magnitudes;
  std::vector<float> frequenciesHz;
};

class SpectrumAnalyzer {
 public:
  SpectrumFrame analyze(const SurroundBlock& block, int fftSize) const;

 private:
  static float windowedSample(float sample, int index, int size);
};

}  // namespace uvk
