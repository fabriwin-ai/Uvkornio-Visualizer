#pragma once

#include "audio_stream.h"

namespace uvk {

class MicrophoneInput {
 public:
  MicrophoneInput(float sampleRate, int blockSize);

  SurroundBlock captureBlock();

 private:
  AudioStream fallbackStream_;
};

}  // namespace uvk
