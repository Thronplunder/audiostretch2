// all algorithms are based on this paper bei meinard müller and jonathan driedger
// https://www.mdpi.com/2076-3417/6/2/57
//processes one audio channel

#include<vector>
#include "windowfunction.h"
#include <span>
#include <utility>
#include <iostream>

namespace audiostretch {
class wsola{
    public:
    wsola(int framelength, float stretchFactor);
    void process(std::vector<float> &input, std::vector<float> &output);
    void changeStretchfactor(float newFactor);
    void changeFramesize(unsigned int newFramesize);
    unsigned int getAnalysisHopsize();
    unsigned int getSynthesisHopsize();

    private:
    unsigned int framesize, analysisHopsize, synthesisHopsize, analysisframeSearchRadius, previousFrameOffset;
    float stretchFactor;
    std::vector<float> synthesisFrame;
    windowFunction<float> window;
    float crossCorrelate(std::span<float> previousFrame, std::span<float> nextFrame);
    void fillFrame(std::span<float> input);
    void addToOutput(std::vector<float>& src, std::vector<float>& dest, unsigned int offset);
    unsigned int findNextFrame(std::span<float> inputSlice, std::span<float> outputSlice);
};

wsola::wsola(int framelength, float stretchFactor) : stretchFactor(stretchFactor), framesize(framelength),
                                                   synthesisHopsize(float(framesize) / 2.f),
                                                   window(framesize, windowType::Hann){
    analysisHopsize = synthesisHopsize / stretchFactor;
    synthesisFrame.resize(framesize);
    analysisframeSearchRadius = framesize / 4; 
    }

void wsola::changeStretchfactor(float newFactor){
    stretchFactor = newFactor;
    analysisHopsize = synthesisHopsize / stretchFactor;
}

unsigned int wsola::getAnalysisHopsize(){
    return analysisHopsize;
}

unsigned int wsola::getSynthesisHopsize(){
    return synthesisHopsize;
}

void wsola::changeFramesize(unsigned int newFramesize){
    framesize = newFramesize;
    synthesisHopsize = framesize / 2;
    window.changeSize(framesize);
    analysisHopsize = synthesisHopsize / stretchFactor;
    synthesisFrame.resize(framesize);
    analysisframeSearchRadius = 0.25 * framesize;
}


float wsola::crossCorrelate(std::span<float> previousFrame, std::span<float> nextFrame){
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

void wsola::addToOutput(std::vector<float>& src, std::vector<float>& dest, unsigned int offset){
    int counter{0};
    for(auto &sample : src){
        if(offset + counter < dest.size()){
        dest.at(offset + counter) += sample;
        }
        counter++;
    }
}
void wsola::fillFrame(std::span<float> input){
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

unsigned int wsola::findNextFrame(std::span<float> inputSlice, std::span<float> outputSlice){
    unsigned int maxIndex{0};
    float result{0.f}, maxValue{std::numeric_limits<float>::lowest()};
    std::span<float> potentialFrame;
    maxIndex = 0;

    for(unsigned int i = 0; i + framesize < inputSlice.size(); i++ ){
        
        //end early if no frame fits into the slice anymore, probably not needed
        potentialFrame = std::span<float>(inputSlice.begin() + i, inputSlice.begin() + i + framesize);
        
        result = crossCorrelate(outputSlice, potentialFrame);
        if(result > maxValue){
            maxValue = result;
            maxIndex = i;
        }
    }
    return maxIndex;
}

void wsola::process(std::vector<float> &input, std::vector<float> &output){
    //we are no longer testing for correct vector sizes

    //copy the first frame
    fillFrame(std::span<float>(input).subspan(0, framesize));
    window.applyWindow(synthesisFrame);
    addToOutput(synthesisFrame, output, 0);

    unsigned int numFrames = input.size() / analysisHopsize;
    for(unsigned int i = 1; i < numFrames; i++){
        auto startSearch = input.begin() + (i * analysisHopsize) - analysisframeSearchRadius;
        auto endSearch = input.begin() + (i * analysisHopsize) + framesize + analysisframeSearchRadius;

        if(i * analysisHopsize + framesize + analysisframeSearchRadius > input.size()){
            endSearch = input.end();
            }
        unsigned int outputOffset = (i - 1) * synthesisHopsize;

        //a slice to compare against at last frame + n/2 or synthesishopsize, since this is the future position
        auto compareSliceStart = input.begin() + (i - 1) * analysisHopsize + synthesisHopsize;
        auto compareSliceEnd =input.begin() + (i - 1) * analysisHopsize + synthesisHopsize + framesize;

        if((i - 1) * analysisHopsize + synthesisHopsize + framesize > input.size()){
            compareSliceEnd = input.end();
        }
        if(output.size() - outputOffset < framesize  ){
            return;
        }
        unsigned int nextFrame = findNextFrame(std::span<float>(startSearch, endSearch), std::span<float>(compareSliceStart, compareSliceEnd));

        //output the next frame for debugging
        //std::cout << nextFrame << " ";
        //std::cout.flush();

        fillFrame(std::span<float>(input).subspan(i * analysisHopsize + nextFrame - analysisframeSearchRadius, framesize));
        window.applyWindow(synthesisFrame);
        addToOutput(synthesisFrame, output, i * synthesisHopsize);
    }
}
}