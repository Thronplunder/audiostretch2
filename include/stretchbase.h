// all algorithms are based on this paper bei meinard müller and jonathan driedger
// https://www.mdpi.com/2076-3417/6/2/57

//base class for time stretching algorithms

#include<vector>
#include "windowfunction.h"

namespace audiotretch{

    template<typename T>
    class stretchBase{
        public:
    stretchBase(int frameSize, float stretchFactor);
    virtual void process(std::vector<T> &input, std::vector<T> &output);
    virtual void changeStretchfactor(float newFactor);
    virtual void changeFramesize(unsigned int newFramesize);

    private:
    int analysisFramesize, analysisHopsize, synthesisHopsize;
    float stretchFactor;
    std::vector<T> synthesisFrame;
    windowFunction<T> window;
    
    };

    
}