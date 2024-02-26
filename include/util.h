#include <vector>
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
  for (auto &ch : output) {
    for (auto &sample : ch) {
      sample = input.at(sampleCounter);
      sampleCounter++;
    }
  }

  return 0;
}
template <typename T>
int interleaveAudio(std::vector<std::vector<T>> &input,
                    std::vector<T> &output) {
  size_t inputlength{0}, counter{0};
  inputlength = input.size() * input.at(0).size();
  if (output.size() != inputlength) {
    return -1;
  }
  for (auto &ch : input) {
    for (auto &sample : ch) {
      output.at(counter) = sample;
      counter++;
    }
  }
  return 0;
}

unsigned int calcOutputLength(unsigned int inputLength,
                              unsigned int numChannels,
                              unsigned int analysisHopsize,
                              unsigned int synthesisHopsize
                              ) {
  unsigned int numFrames = inputLength / analysisHopsize;

  return numChannels * synthesisHopsize + synthesisHopsize * (numFrames - 1);
}
} // namespace audiostretch