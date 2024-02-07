// all algorithms are based on this paper bei meinard müller and jonathan driedger
// https://www.mdpi.com/2076-3417/6/2/57
//processes one audio channel

#include<vector>
#include "windowfunction.h"
#include <span>
#include <utility>
#include <iostream>

namespace audiostretch {
template<typename T>
class wsola{
    public:
    wsola(int framelength, float stretchFactor);
    void process(std::vector<T> &input, std::vector<T> &output);
    void changeStretchfactor(float newFactor);
    void changeFramesize(unsigned int newFramesize);

    private:
    unsigned int framesize, analysisHopsize, synthesisHopsize, analysisframeSearchRadius, previousFrameOffset;
    float stretchFactor;
    std::vector<T> synthesisFrame;
    windowFunction<T> window;
    float crossCorrelate(std::span<T> previousFrame, std::span<T> nextFrame);
    void fillFrame(std::span<T> input);
    void addToOutput(std::vector<T>& src, std::vector<T>& dest, unsigned int offset);
    unsigned int findNextFrame(std::span<T> input);
};

template<typename T>
wsola<T>::wsola(int framelength, float stretchFactor) : stretchFactor(stretchFactor), framesize(framelength),
                                                   synthesisHopsize(float(framesize) / 2.f),
                                                   window(framesize, windowType::Hann){
    analysisHopsize = synthesisHopsize / stretchFactor;
    synthesisFrame.resize(framesize);
    analysisframeSearchRadius = 50; 
    }

template <typename T>
void wsola<T>::changeStretchfactor(float newFactor){
    stretchFactor = newFactor;
    analysisHopsize = synthesisHopsize / stretchFactor;
}

template<typename T>
void wsola<T>::changeFramesize(unsigned int newFramesize){
    framesize = newFramesize;
    synthesisHopsize = framesize / 2;
    window.changeSize(framesize);
    analysisHopsize = synthesisHopsize / stretchFactor;
    synthesisFrame.resize(framesize);
    analysisframeSearchRadius = 0.1 * framesize;
}
template<typename T>
float wsola<T>::crossCorrelate(std::span<T> previousFrame, std::span<T> nextFrame){
    float sum{0};
    for(int i = 0; i < previousFrame.size(); i++){
        if(i < nextFrame.size()){
            sum += previousFrame[i] * nextFrame[i];
        }
        else {
            sum += previousFrame[i] * 0;
        }
    }
    return sum;
}

template <typename T>
void wsola<T>::addToOutput(std::vector<T>& src, std::vector<T>& dest, unsigned int offset){
    int counter{0};
    for(auto &sample : src){
        if(offset + counter < dest.size()){
        dest.at(offset + counter) += sample;
        }
    }
}
template< typename T>
void wsola<T>::fillFrame(std::span<T> input){
    int counter{0};
    if(input.size() == synthesisFrame.size()){
        synthesisFrame.assign(input.begin(), input.end());
    }
    else {
        for(int i = 0; i < framesize; i++){
            if(i < input.size()){
                synthesisFrame.at(i) = input[i];
            }
            else {
                synthesisFrame.at(i) = 0;
            }
        }
    }
}

template<typename T>
unsigned int wsola<T>::findNextFrame(std::span<T> input){
    unsigned int maxIndex{0};
    float result{0.f}, maxValue{0};
    std::span<T> potentialFrame;
    unsigned int startOffset = previousFrameOffset + analysisHopsize - analysisframeSearchRadius;
    auto  previousFrame = std::span<T>(input).subspan(previousFrameOffset, framesize);
    maxIndex = startOffset;
    for(unsigned int i = startOffset; i < startOffset + analysisframeSearchRadius * 2; i ++ ){
        if(i + framesize < input.size()){
            potentialFrame = std::span<T>(input.begin() + i, input.begin() + i + framesize);
        }
        else if(input.begin() + i != input.end()) {
            potentialFrame = std::span<T>(input.begin() + i, input.end());
        }
        else {
            return i;
        }
        result = crossCorrelate(previousFrame, potentialFrame);
        if(result > maxValue){
            maxValue = result;
            maxIndex = i;
        }
    }
    if(maxIndex - previousFrameOffset > 1000)
    {std::cout << "WTF!" << std::endl;}
    return maxIndex;
}

template<typename T>
void wsola<T>::process(std::vector<T> &input, std::vector<T> &output){
    //we are no longer testing for correct vector sizes
    unsigned int synthesisframeCounter{1};
    //we make frames until we left the input vector. we also use std::span now
    previousFrameOffset = 0;
    //copy the first frame
    fillFrame(std::span<T>(input).subspan(0, framesize));
    window.applyWindow(synthesisFrame);
    addToOutput(synthesisFrame, output, previousFrameOffset);
    while(previousFrameOffset + (analysisHopsize + analysisframeSearchRadius + framesize) < input.size()){
        unsigned int nextFrame = findNextFrame(input);
        fillFrame(std::span<T>(input).subspan(nextFrame, framesize));
        window.applyWindow(synthesisFrame);
        addToOutput(synthesisFrame, output, synthesisframeCounter * synthesisHopsize);
        synthesisframeCounter++;    
        previousFrameOffset = nextFrame;
    }
}
}