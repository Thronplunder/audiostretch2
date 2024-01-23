// all algorithms are based on this paper bei meinard müller and jonathan driedger
// https://www.mdpi.com/2076-3417/6/2/57
//processes one audio channel

#include<vector>
#include "windowfunction.h"

namespace audiostretch {
template<typename T>
class wsola{
    public:
    wsola(int frameSize, float stretchFactor);
    void process(std::vector<T> &input, std::vector<T> &output);

    private:
    int analysisFramesize, analysisHopsize, synthesisHopsize;
    float stretchFactor;
    std::vector<T> synthesisFrame;
    windowFunction<T> window;
    
};
}