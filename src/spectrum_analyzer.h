#pragma once

#include "audio_stream.h"
#include "enki_ts.h"

#include <vector>

namespace uvk {

struct SpectrumFrame {
  std::vector<float> magnitudes;
  std::vector<float> frequenciesHz;
  std::vector<float> bandCentersHz;
  std::vector<float> bandEnergies;
};

class SpectrumAnalyzer {
 public:
  SpectrumFrame analyze(const SurroundBlock& block, int fftSize) const;
  SpectrumFrame analyze(const SurroundBlock& block, int fftSize,
                        const std::vector<float>& bandEdgesHz,
                        EnkiTaskScheduler* scheduler) const;

 private:
  static float windowedSample(float sample, int index, int size);
  static void computeBins(const std::vector<float>& mono, int size, int binStart, int binEnd,
                          std::vector<float>& magnitudes);
};

}  // namespace uvk
