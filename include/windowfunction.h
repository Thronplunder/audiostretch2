#include<vector>
#include<numbers>
#include<cmath>

#pragma once
namespace audiostretch{

    enum class windowType{
        Hann,
        Triangle,
        Sine, 

    };
    template<typename T>
    class windowFunction{
        std::vector<float> window;
        windowType currentType;
        unsigned int size;

        void fillWindow();

    public:
    windowFunction(unsigned int length, windowType type);
    void changeType(windowType newType);
    void changeSize(unsigned int newSize);
    unsigned int getSize();
    windowType getCurrentWindowType();
    void applyWindow(std::vector<T> &input);
    };

    template<typename T>
    windowFunction<T>::windowFunction(unsigned int length, windowType type){
        currentType = type;
        window.resize(length);
        size = length;
        fillWindow();
    }

    template<typename T>
    void windowFunction<T>::changeType(windowType newType){
        currentType = newType;
        fillWindow();
    }

    template<typename T>
    void windowFunction<T>::changeSize(unsigned int newSize){
        window.resize(newSize);
        size = newSize;
        fillWindow();
    }

    template<typename T>
    unsigned int windowFunction<T>::getSize(){
        return size;
    }

    template<typename T>
    void windowFunction<T>::fillWindow(){
        switch (currentType)
        {
        case windowType::Hann:
            for(int i = 0; i < window.size(); i++ ){
                window.at(i) = 0.5 * (1 - cos(2 * std::numbers::pi * ((float)i / size)));
            }
            break;
        case windowType::Triangle:
        for(int i = 0; i < window.size(); i++ ){
                window.at(i) = 1 - std::abs((float)i / (size * 0.5) - 1);
            }
            break;
        case windowType::Sine:
        for(int i = 0; i < window.size(); i++ ){
                window.at(i) = sin((std::numbers::pi * ((float)i )/size));
            }
            break;
        default:
            break;
        }
    }

    template<typename T>
    void windowFunction<T>::applyWindow(std::vector<T> &input){
        if(input.size() != size){
            return;
        }
        for(int i = 0; i < size; i++){
            input.at(i) = input.at(i) * window.at(i);
        }
    }
}