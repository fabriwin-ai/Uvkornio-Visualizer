#include "microphone_input.h"

#ifdef UVK_ENABLE_ALSA
#include <algorithm>
#include <alsa/asoundlib.h>
#include <vector>
#endif

namespace uvk {

#ifdef UVK_ENABLE_ALSA
struct MicrophoneInput::AlsaState {
  snd_pcm_t* handle{};
  unsigned int sampleRate{};
  int blockSize{};
  int channels{};
};
#endif

MicrophoneInput::MicrophoneInput(float sampleRate, int blockSize)
    : fallbackStream_(sampleRate, blockSize) {
#ifdef UVK_ENABLE_ALSA
  auto state = new AlsaState{};
  state->sampleRate = static_cast<unsigned int>(sampleRate);
  state->blockSize = blockSize;
  state->channels = 1;
  alsaState_ = state;
  if (initializeAlsa()) {
    activeBackend_ = "alsa";
  }
#endif
}

MicrophoneInput::~MicrophoneInput() {
#ifdef UVK_ENABLE_ALSA
  if (alsaState_) {
    if (alsaState_->handle) {
      snd_pcm_close(alsaState_->handle);
    }
    delete alsaState_;
    alsaState_ = nullptr;
  }
#endif
}

SurroundBlock MicrophoneInput::captureBlock() {
#ifdef UVK_ENABLE_ALSA
  if (activeBackend_ == "alsa") {
    return captureFromAlsa();
  }
#endif
  return fallbackStream_.nextBlock();
}

void MicrophoneInput::selectBackend(const std::string& name) {
  if (name == "alsa") {
#ifdef UVK_ENABLE_ALSA
    if (initializeAlsa()) {
      activeBackend_ = "alsa";
      return;
    }
#endif
    activeBackend_ = "simulator";
    return;
  }
  activeBackend_ = "simulator";
}

#ifdef UVK_ENABLE_ALSA
bool MicrophoneInput::initializeAlsa() {
  if (!alsaState_) {
    return false;
  }
  if (alsaState_->handle) {
    return true;
  }
  if (snd_pcm_open(&alsaState_->handle, "default", SND_PCM_STREAM_CAPTURE, 0) < 0) {
    alsaState_->handle = nullptr;
    return false;
  }
  if (snd_pcm_set_params(alsaState_->handle, SND_PCM_FORMAT_FLOAT_LE,
                         SND_PCM_ACCESS_RW_INTERLEAVED, alsaState_->channels,
                         alsaState_->sampleRate, 1, 500000) < 0) {
    snd_pcm_close(alsaState_->handle);
    alsaState_->handle = nullptr;
    return false;
  }
  return true;
}

SurroundBlock MicrophoneInput::captureFromAlsa() {
  SurroundBlock block;
  block.sampleRate = fallbackStream_.sampleRate();
  block.timestampSeconds = 0.0;

  if (!alsaState_ || !alsaState_->handle) {
    return fallbackStream_.nextBlock();
  }

  std::vector<float> mono(static_cast<size_t>(alsaState_->blockSize));
  const int frames =
      snd_pcm_readi(alsaState_->handle, mono.data(), static_cast<snd_pcm_uframes_t>(mono.size()));
  if (frames < 0) {
    snd_pcm_prepare(alsaState_->handle);
    return fallbackStream_.nextBlock();
  }

  const int actualFrames = std::min<int>(frames, alsaState_->blockSize);
  block.samples.resize(static_cast<size_t>(actualFrames));
  for (int i = 0; i < actualFrames; ++i) {
    std::array<float, 8> sample{};
    const float value = mono[static_cast<size_t>(i)];
    sample.fill(value);
    block.samples[static_cast<size_t>(i)] = sample;
  }
  return block;
}
#endif

}  // namespace uvk
