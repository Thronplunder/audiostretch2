// all algorithms are based on this paper bei meinard müller and jonathan
// driedger https://www.mdpi.com/2076-3417/6/2/57
// processes one audio channel
#include "basestretch.h"
#include "pocketfft_hdronly.h"
#include <complex>
#include <span>
#include <utility>
#include <vector>
#include <iostream>
#pragma once

namespace audiostretch {
class pvtsm : public basestretch {
public:
  pvtsm(unsigned int frameLength, float stretchFac, unsigned int sr);
  void process(std::vector<float> &input, std::vector<float> &output) override;
  void changeSamplerate(unsigned int newSamplerate);
  void changeFramesize(unsigned int newFramesize) override ;

private:
  std::vector<float> lastFrame, lastPhases;
  unsigned int sampleRate;
  std::vector<std::complex<float>> fftResult, lastResult;
};

pvtsm::pvtsm(unsigned int frameLength, float stretchFac, unsigned int sr)
    : basestretch(frameLength, stretchFac), sampleRate(sr) {
  fftResult.resize(frameLength);
  lastFrame.resize(frameLength);
  lastPhases.resize(frameLength);
}

void pvtsm::changeSamplerate(unsigned int newSampeRate) {
  sampleRate = newSampeRate;
}

void pvtsm::changeFramesize(unsigned int newFramesize) {
  basestretch::changeFramesize(newFramesize);
  fftResult.resize(newFramesize);
  lastResult.resize(newFramesize);
  lastPhases.resize(newFramesize);
}

void pvtsm::process(std::vector<float> &input, std::vector<float> &output) {
  // fft values
  pocketfft::shape_t shape = {synthesisFrame.size()};
  pocketfft::stride_t strideIn = {sizeof(float)};
  pocketfft::stride_t strideOut = {sizeof(std::complex<float>)};
  pocketfft::shape_t axis = {0};
  bool forward = pocketfft::FORWARD;
  float fct{1.f};

  // ifft values
  pocketfft::shape_t &ishape = shape;
  bool backward = pocketfft::BACKWARD;

  fillFrame(std::span<float>(input.begin(), framesize));
  pocketfft::r2c(shape, strideIn, strideOut, axis, forward,
                 synthesisFrame.data(), fftResult.data(), fct);
  pocketfft::c2r(ishape, strideOut, strideIn, axis, backward, fftResult.data(),
                 synthesisFrame.data(), fct);
                 window.applyWindow(synthesisFrame);
  addToOutput(synthesisFrame, output, 0);
  for(int i = 0; i < lastPhases.size(); i++){
    lastPhases.at(i) = std::arg(fftResult.at(i));
  }
  lastResult.assign(fftResult.begin(), fftResult.end());
  lastFrame.assign(synthesisFrame.begin(), synthesisFrame.end());

  //fill phases
  for(unsigned int i = 0; i < lastFrame.size(); i++){
    lastFrame.at(i) = std::arg(lastResult.at(i));
  }

  // now do it for all the other frames
  unsigned int numFrames = input.size() / analysisHopsize;
  for (unsigned int i = 1; i < numFrames; i++) {
    unsigned int analysisOffset = i * analysisHopsize;
    if (analysisOffset + framesize < input.size()) {
      fillFrame(std::span<float>(input.begin() + analysisOffset,
                                 input.begin() + analysisOffset + framesize));
    } else {
      fillFrame(std::span<float>(input.begin() + analysisOffset, input.end()));
    }

    pocketfft::r2c(shape, strideIn, strideOut, axis, forward,
                   synthesisFrame.data(), fftResult.data(), fct);

    // do all the pvtsm magic in here
    for(int bin =0; i < fftResult.size(); i++){
      float currPhase = std::arg(fftResult.at(bin));
      float lastPhase = lastPhases.at(bin);
      auto  phaseDiff = lastPhases.at(bin) - currPhase;
      float omega = (2 * std::numbers::pi_v<float> * bin) / framesize; //phase advance per sample
      float  timeDelta = analysisHopsize / sampleRate;
      float phaseAdv = omega * analysisHopsize; //expected phase advance 
      float phaseErr = (currPhase - lastPhase) - phaseAdv; //phase error
      phaseErr = audiostretch::wrapPi(phaseErr);
      float IF = omega;
      std::cout << phaseDiff << " ";
      lastPhases.at(i) = currPhase;
    }

    std::cout << std::endl << "----------------------------" << std::endl;

    pocketfft::c2r(ishape, strideOut, strideIn, axis, backward,
                   fftResult.data(), synthesisFrame.data(), fct);
    lastResult.assign(fftResult.begin(), fftResult.end());
    lastFrame.assign(synthesisFrame.begin(), synthesisFrame.end());
    window.applyWindow(synthesisFrame);
    addToOutput(synthesisFrame, output, i * synthesisHopsize);
  }
}
} // namespace audiostretch