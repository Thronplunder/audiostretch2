#include <vector>
#include <cmath>
#include <numbers>
namespace audiostretch {
template <typename T>
int deinterleaveArray(std::vector<T> &input,
                      std::vector<std::vector<T>> &output, size_t numChannels) {
  // test for right size
  if (output.size() != numChannels) {
    return -1;
  }
  for (auto &channel : output) {
    if (channel.size() != input.size() / numChannels) {
      return -1;
    }
  }

  unsigned int sampleCounter{0};
  // fill vectors
  for(int i = 0; i < input.size(); i = i + numChannels){
    for(int j = 0; j < numChannels; j++){
      output.at(j).at(sampleCounter) = input.at(i + j);
    }
    sampleCounter++;
  }

  return 0;
}

template <typename T>
int interleaveArray(std::vector<std::vector<T>> &input,
                    std::vector<T> &output) {
  size_t inputlength{0}, counter{0};
  inputlength = input.size() * input.at(0).size();
  if (output.size() != inputlength) {
    return -1;
  }
  unsigned int channels = input.size();
  for (unsigned int chan = 0; chan < channels; chan++) {
    for (unsigned int sample = 0; sample < input.at(0).size(); sample++) {
      output.at(sample * channels + chan ) = input.at(chan).at(sample);
    }
  }
  return 0;
}

unsigned int calcOutputLength(unsigned int inputLength,
                              unsigned int numChannels,
                              unsigned int analysisHopsize,
                              unsigned int synthesisHopsize) {
  unsigned int numFrames = inputLength / analysisHopsize;

  return numChannels * synthesisHopsize + synthesisHopsize * (numFrames - 1);
}
float wrapPi(float wrapped){
  return std::fmod(wrapped, std::numbers::pi_v<float>);
}

} // namespace audiostretch