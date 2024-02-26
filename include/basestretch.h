
#include "windowfunction.h"
#include <span>
#include <utility>
#include <vector>

#pragma once

namespace audiostretch {
class basestretch {
protected:
  basestretch();
  basestretch(unsigned int frameLength, float stretchFac);

public:
  unsigned int framesize, analysisHopsize, synthesisHopsize;
  virtual void process(std::vector<float> &input, std::vector<float> &output) = 0;
  float stretchFactor;
  std::vector<float> synthesisFrame;
  windowFunction<float> window;
  void changeStretchfactor(float newFactor);
  virtual void changeFramesize(unsigned int newFramesize);
  unsigned int getAnalysisHopsize();
  unsigned int getSynthesisHopsize();

  void fillFrame(std::span<float> input);
  void addToOutput(std::vector<float> &src, std::vector<float> &dest,
                   unsigned int offset);
};

basestretch::basestretch(unsigned int frameLength, float stretchFac)
    : stretchFactor(stretchFac), framesize(frameLength),
      synthesisHopsize(float(framesize) / 2.f),
      window(framesize, windowType::Hann) {
  analysisHopsize = synthesisHopsize / stretchFactor;
  synthesisFrame.resize(framesize);
}
unsigned int basestretch::getAnalysisHopsize() { return analysisHopsize; }

unsigned int basestretch::getSynthesisHopsize() { return synthesisHopsize; }

void basestretch::changeStretchfactor(float newFactor) {
  stretchFactor = newFactor;
  analysisHopsize = synthesisHopsize / stretchFactor;
}
void basestretch::changeFramesize(unsigned int newFramesize) {
  framesize = newFramesize;
  synthesisHopsize = framesize / 2.f;
  window.changeSize(framesize);
  analysisHopsize = synthesisHopsize / stretchFactor;
  synthesisFrame.resize(framesize);
}

void basestretch::fillFrame(std::span<float> input) {
  int counter{0};
  if (input.size() == synthesisFrame.size()) {
    synthesisFrame.assign(input.begin(), input.end());
  } else {
    for (int i = 0; i < framesize; i++) {
      if (i < input.size()) {
        synthesisFrame.at(i) = input[i];
      } else {
        synthesisFrame.at(i) = 0;
      }
    }
  }
}

void basestretch::addToOutput(std::vector<float> &src, std::vector<float> &dest,
                        unsigned int offset) {
  int counter{0};
  for (auto &sample : src) {
    if (offset + counter < dest.size()) {
      dest.at(offset + counter) += sample;
    }
    counter++;
  }
}
} // namespace audiostretch