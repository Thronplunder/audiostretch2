// all algorithms are based on this paper bei meinard müller and jonathan
// driedger https://www.mdpi.com/2076-3417/6/2/57
// processes one audio channel

#include "basestretch.h"
#include <iostream>
#include <span>
#include <utility>
#include <vector>

#pragma once

namespace audiostretch {
class wsola : public basestretch {
public:
  wsola(int framelength, float stretchFactor);
  void process(std::vector<float> &input, std::vector<float> &output);
  void changeFramesize(unsigned int newFramesize);

private:
  unsigned int analysisframeSearchRadius, previousFrameOffset;
  float crossCorrelate(std::span<float> previousFrame,
                       std::span<float> nextFrame);
  void fillFrame(std::span<float> input);
  void addToOutput(std::vector<float> &src, std::vector<float> &dest,
                   unsigned int offset);
  unsigned int findNextFrame(std::span<float> inputSlice,
                             std::span<float> outputSlice);
};

wsola::wsola(int frameLength, float stretchFactor)
    : basestretch(frameLength, stretchFactor) {
  analysisframeSearchRadius = framesize / 4;
}

void wsola::changeFramesize(unsigned int newFramesize) {
  basestretch::changeFramesize(newFramesize);
  analysisframeSearchRadius = 0.25 * framesize;
}

float wsola::crossCorrelate(std::span<float> previousFrame,
                            std::span<float> nextFrame) {
  float sum{0};
  for (int i = 0; i < previousFrame.size(); i++) {
    if (i < nextFrame.size()) {
      sum += previousFrame[i] * nextFrame[i];
    } else {
      sum += previousFrame[i] * 0;
    }
  }
  return sum;
}

unsigned int wsola::findNextFrame(std::span<float> inputSlice,
                                  std::span<float> outputSlice) {
  unsigned int maxIndex{0};
  float result{0.f}, maxValue{std::numeric_limits<float>::lowest()};
  std::span<float> potentialFrame;
  maxIndex = 0;

  for (unsigned int i = 0; i + framesize < inputSlice.size(); i++) {

    // end early if no frame fits into the slice anymore, probably not needed
    potentialFrame = std::span<float>(inputSlice.begin() + i,
                                      inputSlice.begin() + i + framesize);

    result = crossCorrelate(outputSlice, potentialFrame);
    if (result > maxValue) {
      maxValue = result;
      maxIndex = i;
    }
  }
  return maxIndex;
}

void wsola::process(std::vector<float> &input, std::vector<float> &output) {

  // copy the first frame
  fillFrame(std::span<float>(input).subspan(0, framesize));
  window.applyWindow(synthesisFrame);
  addToOutput(synthesisFrame, output, 0);

  unsigned int numFrames = input.size() / analysisHopsize;
  for (unsigned int i = 1; i < numFrames; i++) {
    auto startSearch =
        input.begin() + (i * analysisHopsize) - analysisframeSearchRadius;
    auto endSearch = input.begin() + (i * analysisHopsize) + framesize +
                     analysisframeSearchRadius;

    if (i * analysisHopsize + framesize + analysisframeSearchRadius >
        input.size()) {
      endSearch = input.end();
    }
    unsigned int outputOffset = (i - 1) * synthesisHopsize;

    // a slice to compare against at last frame + n/2 or synthesishopsize, since
    // this is the future position
    auto compareSliceStart =
        input.begin() + (i - 1) * analysisHopsize + synthesisHopsize;
    auto compareSliceEnd = input.begin() + (i - 1) * analysisHopsize +
                           synthesisHopsize + framesize;

    if ((i - 1) * analysisHopsize + synthesisHopsize + framesize >
        input.size()) {
      compareSliceEnd = input.end();
    }
    if (output.size() - outputOffset < framesize) {
      return;
    }
    unsigned int nextFrame =
        findNextFrame(std::span<float>(startSearch, endSearch),
                      std::span<float>(compareSliceStart, compareSliceEnd));

    fillFrame(std::span<float>(input).subspan(i * analysisHopsize + nextFrame -
                                                  analysisframeSearchRadius,
                                              framesize));
    window.applyWindow(synthesisFrame);
    addToOutput(synthesisFrame, output, i * synthesisHopsize);
  }
}
} // namespace audiostretch