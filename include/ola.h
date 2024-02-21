// all algorithms are based on this paper bei meinard müller and jonathan
// driedger https://www.mdpi.com/2076-3417/6/2/57
// processes one audio channel
#pragma once
#include "windowfunction.h"
#include <vector>
#include "basestretch.h"


namespace audiostretch {
class ola : public basestretch{
public:
  ola(int frameSize, float stretchFac);
  //ola();
  void process(std::vector<float> &input, std::vector<float> &output) override;
};


ola::ola(int frameLength, float stretchFac) : basestretch(frameLength, stretchFac) {
}

void ola::process(std::vector<float> &input, std::vector<float> &output) {
  // null out output and resize it if necessary
  for (auto &it : output) {
    it = 0;
  }
  // run loop for every analysis frame
  unsigned int numFrames = input.size() / analysisHopsize;
  for (size_t i = 0; i < numFrames; i++) {
    // fill one frame
    size_t analysisOffset = i * analysisHopsize;
    fillFrame(std::span<float>(input.begin() + analysisOffset , framesize));
    // apply window
    window.applyWindow(synthesisFrame);

    // write synthesis frame to output
    size_t synthesisOffset = i * synthesisHopsize;
    addToOutput(synthesisFrame, output, synthesisOffset);
  }
}

} // namespace audiostretch