#pragma once

#include "audio_stream.h"

#include <string>

namespace uvk {

class MicrophoneInput {
 public:
  MicrophoneInput(float sampleRate, int blockSize);
  ~MicrophoneInput();

  SurroundBlock captureBlock();
  void selectBackend(const std::string& name);
  [[nodiscard]] const std::string& activeBackend() const noexcept { return activeBackend_; }

 private:
  bool initializeAlsa();
  SurroundBlock captureFromAlsa();

  AudioStream fallbackStream_;
  std::string activeBackend_{"simulator"};
#ifdef UVK_ENABLE_ALSA
  struct AlsaState;
  AlsaState* alsaState_{nullptr};
#endif
};

}  // namespace uvk
