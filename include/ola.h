// all algorithms are based on this paper bei meinard müller and jonathan driedger
// https://www.mdpi.com/2076-3417/6/2/57
//processes one audio channel

#include<vector>
#include "windowfunction.h"

namespace audiostretch {
template<typename T>
class ola{
    public:
    ola(int frameSize, float stretchFactor);
    void process(std::vector<T> &input, std::vector<T> &output);
    void changeStretchfactor(float newFactor);
    void changeFramesize(unsigned int newFramesize);

    private:
    int analysisFramesize, analysisHopsize, synthesisHopsize;
    float stretchFactor;
    std::vector<T> synthesisFrame;
    windowFunction<T> window;
    
};

    template<typename T>
    ola<T>::ola(int frameSize, float stretchFactor) : stretchFactor(stretchFactor), analysisFramesize(frameSize),
                                                      synthesisHopsize(float(frameSize) / 2.f),
                                                      window(frameSize, windowType::Hann){
        analysisHopsize = synthesisHopsize / stretchFactor;
        synthesisFrame.resize(analysisFramesize);
    }

    template<typename T>
    void ola<T>::process(std::vector<T> &input, std::vector<T> &output){
        //null out output and resize it
        if(output.size() != input.size() * stretchFactor){
        output.resize(input.size() * stretchFactor);
        }
        for(auto &it : output){
            it = 0;
        }
        //run loop for every analysis frame
        unsigned int numFrames = input.size() / analysisHopsize; 
        for(size_t i = 0; i < numFrames; i++){
            //fill one frame
            size_t analysisOffset = i * analysisHopsize;
            for(int j = 0; j < analysisFramesize; j++){
                //get audio from input, if out of range set to 0 instead 
                if(analysisOffset + j < input.size()){
                    synthesisFrame.at(j) = input.at(analysisOffset + j);
                }
                else {
                    synthesisFrame.at(j) = 0;
                }  
            }
            //apply window
            window.applyWindow(synthesisFrame);
            
            //write synthesis frame to output
            size_t synthesisOffset = i * synthesisHopsize;
            for(int j = 0 ; j < analysisFramesize; j++){
                if(synthesisOffset+ j < output.size()){
                output.at(synthesisOffset + j) += synthesisFrame.at(j);
                }
            }
        }
    }

    template<typename T>
    void ola<T>::changeStretchfactor(float newFactor){
        stretchFactor = newFactor;
        analysisHopsize = synthesisHopsize / stretchFactor;
    }

    template<typename T>
    void ola<T>::changeFramesize(unsigned int newFramesize){
        analysisFramesize = newFramesize;
        synthesisHopsize = analysisFramesize / 2.f;
        window.changeSize(analysisFramesize);
        analysisHopsize = synthesisHopsize /stretchFactor;
        synthesisFrame.resize(analysisFramesize);
    }
}