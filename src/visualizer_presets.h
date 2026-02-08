#pragma once

#include <string>
#include <vector>

namespace uvk {

struct SpectrumPreset {
  std::string name;
  int fftSize;
  std::vector<float> bandEdgesHz;
};

inline SpectrumPreset makeWidebandPreset() {
  return {"Wideband", 512, {20.0f, 60.0f, 250.0f, 1000.0f, 4000.0f, 12000.0f, 20000.0f}};
}

inline SpectrumPreset makeSubwooferPreset() {
  return {"Subwoofer", 256, {20.0f, 40.0f, 80.0f, 120.0f, 200.0f}};
}

inline SpectrumPreset makePresencePreset() {
  return {"Presence", 512, {200.0f, 500.0f, 1000.0f, 2500.0f, 5000.0f, 8000.0f}};
}

inline std::vector<SpectrumPreset> availablePresets() {
  return {makeWidebandPreset(), makeSubwooferPreset(), makePresencePreset()};
}

inline std::vector<std::string> availablePresetNames() {
  std::vector<std::string> names;
  for (const auto& preset : availablePresets()) {
    names.push_back(preset.name);
  }
  return names;
}

inline SpectrumPreset presetByName(const std::string& name, bool* found = nullptr) {
  for (const auto& preset : availablePresets()) {
    if (preset.name == name) {
      if (found) {
        *found = true;
      }
      return preset;
    }
  }
  if (found) {
    *found = false;
  }
  return makeWidebandPreset();
}

}  // namespace uvk
