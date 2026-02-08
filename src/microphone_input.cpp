#include "microphone_input.h"

namespace uvk {

MicrophoneInput::MicrophoneInput(float sampleRate, int blockSize)
    : fallbackStream_(sampleRate, blockSize) {}

SurroundBlock MicrophoneInput::captureBlock() {
  return fallbackStream_.nextBlock();
}

}  // namespace uvk
