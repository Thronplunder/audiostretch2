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
    unsigned int getAnalysisHopsize();
    unsigned int getSynthesisHopsize();

    private:
    unsigned int framesize, analysisHopsize, synthesisHopsize, analysisframeSearchRadius, previousFrameOffset;
    float stretchFactor;
    std::vector<T> synthesisFrame;
    windowFunction<T> window;
    float crossCorrelate(std::span<T> previousFrame, std::span<T> nextFrame);
    void fillFrame(std::span<T> input);
    void addToOutput(std::vector<T>& src, std::vector<T>& dest, unsigned int offset);
    unsigned int findNextFrame(std::span<T> inputSlice, std::span<T> outputSlice);
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

template <typename T>
unsigned int wsola<T>::getAnalysisHopsize(){
    return analysisHopsize;
}

template <typename T>
unsigned int wsola<T>::getSynthesisHopsize(){
    return synthesisHopsize;
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
        counter++;
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
unsigned int wsola<T>::findNextFrame(std::span<T> inputSlice, std::span<T> outputSlice){
    unsigned int maxIndex{0};
    float result{0.f}, maxValue{std::numeric_limits<float>::lowest()};
    std::span<T> potentialFrame;
    maxIndex = 0;

    for(unsigned int i = 0; i < inputSlice.size() - framesize; i++ ){
        
        potentialFrame = std::span<T>(inputSlice.begin() + i, inputSlice.begin() + i + framesize);
        
        result = crossCorrelate(outputSlice, potentialFrame);
        if(result > maxValue){
            maxValue = result;
            maxIndex = i;
        }
    }
    return maxIndex;
}

template<typename T>
void wsola<T>::process(std::vector<T> &input, std::vector<T> &output){
    //we are no longer testing for correct vector sizes

    //copy the first frame
    fillFrame(std::span<T>(input).subspan(0, framesize));
    window.applyWindow(synthesisFrame);
    addToOutput(synthesisFrame, output, 0);

    unsigned int numFrames = input.size() / analysisHopsize;
    unsigned int numFramesOutput = output.size() / synthesisHopsize;
    for(unsigned int i = 1; i < numFrames; i++){
        auto startSearch = input.begin() + (i * analysisHopsize) - analysisframeSearchRadius;
        auto endSearch = input.begin() + (i * analysisHopsize) + framesize + analysisframeSearchRadius;

        if(i * analysisHopsize + framesize + analysisframeSearchRadius > input.size()){
            endSearch = input.end();
            }
        unsigned int outputOffset = (i - 1) * synthesisHopsize;
        auto outputSliceStart = output.begin() + outputOffset;
        auto outputSliceEnd = output.begin() + outputOffset + framesize;
        if(output.size() - outputOffset < framesize  ){
            return;
        }
        unsigned int nextFrame = findNextFrame(std::span<T>(startSearch, endSearch), std::span<T>(outputSliceStart, outputSliceEnd));
        fillFrame(std::span<T>(input).subspan(i * analysisHopsize + nextFrame - analysisframeSearchRadius, framesize));
        window.applyWindow(synthesisFrame);
        addToOutput(synthesisFrame, output, i * synthesisHopsize);
    }
}
}