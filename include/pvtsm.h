// all algorithms are based on this paper bei meinard müller and jonathan
// driedger https://www.mdpi.com/2076-3417/6/2/57
// processes one audio channel
#include "basestretch.h"
#include <span>
#include <utility>
#include <vector>

#pragma once

namespace audiostretch {
class pvtsm : public basestretch {
public:
  pvtsm(unsigned int frameLength, float stretchFac, unsigned int sr);
  void process(std::vector<float> &input, std::vector<float> &output) override;

private:
  audiostretch::windowFunction<float> lastWindow;
  unsigned int sampleRate;
};

pvtsm::pvtsm(unsigned int frameLength, float stretchFac, unsigned int sr)
    : basestretch(frameLength, stretchFac),
      lastWindow(frameLength, audiostretch::windowType::Hann), sampleRate(sr) {}
} // namespace audiostretch