#include <iostream>
#include <filesystem>
#include <argparse/argparse.hpp>
#include <sndfile.h>
#include <sndfile.hh>
#include <format>
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include "util.h"
#include <vector>
#include "ola.h"
#include "wsola.h"


int main(int argc, char** argv)
{
    argparse::ArgumentParser programm("Audiostretcher2");
    std::filesystem::path inputfile, outputfolder, outputfile;
    const std::filesystem::path currentFolder = std::filesystem::current_path();
    float stretchingFactor;
    unsigned int framesize;
    SF_INFO inputInfo, outputinfo;
    SNDFILE *audiofile;
    std::vector<std::vector<float>> inputAudiochannels, outputAudiochannels;
    std::vector<float> tempAudio;
    unsigned int totalSamples;
    audiostretch::ola timeStretcher{512, 1.1};
    audiostretch::wsola wsolastretcher{512, 1.1};


    plog::init(plog::debug, "testing/log.txt");

    programm.add_argument("input")
    .help("Audiofile to stretch");

    programm.add_argument("--stretchfactor", "-s").help("Time stretch factor; >1 means slower audio.")
    .default_value(1.5f).scan<'f', float>();

    programm.add_argument("--output", "-o").help("output folder").default_value(currentFolder.string());

    programm.add_argument("--framesize", "-f").help("The size of the analysis frame for the stretching algorithm")
    .default_value(512).scan<'i', int>();

    try{
        programm.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << programm;
        std::exit(69);
}
    inputfile = std::filesystem::path(programm.get("input"));
    outputfolder = std::filesystem::path(programm.get("-o"));
    stretchingFactor = programm.get<float>("--stretchfactor");
    framesize = programm.get<int>("--framesize");

    timeStretcher.changeStretchfactor(stretchingFactor);
    timeStretcher.changeFramesize(framesize);
    wsolastretcher.changeStretchfactor(stretchingFactor);
    wsolastretcher.changeFramesize(framesize);

    if(!std::filesystem::is_regular_file(inputfile) || !std::filesystem::exists(inputfile)){
        PLOGE << inputfile.string() << " is not a file";
        return 1;
    }

    if(!std::filesystem::is_directory(outputfolder) || !std::filesystem::exists(outputfolder)){
        PLOG_ERROR << outputfolder.string() << " not a path";
        return 1;
    }
    PLOG_DEBUG << std::format("Running Audiostretcher2 with file {0} outputting to {1} with stretching factor {2}. It has {3} channels ", inputfile.string(), outputfolder.string(), stretchingFactor, inputInfo.channels);

    //open the sound file
    audiofile = sf_open(inputfile.string().c_str(), SFM_READ, &inputInfo);
    if(sf_error(audiofile)){
        PLOGE << std::format("Error opening the audio file with error:  {}", sf_strerror(audiofile)); 
    }

    if(inputInfo.channels > 2 ){
        PLOG_ERROR << "Audio file has more than 2 channels. Only mono or stereo are supported at the moment";
        return 1;
    }
    totalSamples = inputInfo.channels * inputInfo.frames;

    //prepare audio buffers
    inputAudiochannels.resize(inputInfo.channels);
    for(auto &channel : inputAudiochannels){
        channel.resize(inputInfo.frames);
    }
    PLOGD << std::format("configured {0}  channels with {1} samples", inputAudiochannels.size(), inputAudiochannels.at(0).size());
    tempAudio.resize(totalSamples);
    int dataread = sf_read_float(audiofile, tempAudio.data(), totalSamples);
    if(dataread != totalSamples){
        PLOGE << "Didnt read the whole audiofile. Yikers!";
    }
    sf_close(audiofile);
    if(audiostretch::deinterleaveArray<float>(tempAudio, inputAudiochannels, inputInfo.channels)){
        PLOGE << "Could not deinterleave the array";
    };
    tempAudio.clear();

    unsigned int outputLength = audiostretch::calcOutputLength(inputInfo.frames, timeStretcher.getAnalysisHopsize(), timeStretcher.getSynthesisHopsize());
    //prepare output buffers
    outputAudiochannels.resize(inputAudiochannels.size());
    for(auto &ch : outputAudiochannels){
        ch.resize(outputLength);
    }

    //we try to stretch
    
    for (int i = 0; i < inputInfo.channels; i++){
        timeStretcher.process(inputAudiochannels.at(i), outputAudiochannels.at(i));
    }

    tempAudio.resize(outputAudiochannels.size() * outputAudiochannels.at(0).size());
    audiostretch::interleaveAudio(outputAudiochannels, tempAudio);
    //write the output file
    outputfile = outputfolder / inputfile.stem().concat( "_ola");
    outputfile += inputfile.extension();
    outputinfo.format = inputInfo.format;
    outputinfo.channels = inputInfo.channels;
    outputinfo.samplerate = inputInfo.samplerate;
    audiofile = sf_open(outputfile.string().c_str(), SFM_WRITE, &outputinfo);
    sf_write_float(audiofile, tempAudio.data(), tempAudio.size());
    sf_close(audiofile);
    PLOGD << std::format("Outputting into {}", outputfile.string());
    
    for(auto &ch : outputAudiochannels){
        for(auto &sample : ch){
            sample = 0;
        }
    }

    //the same thing but with wsola
    for (int i = 0; i < inputInfo.channels; i++){
        wsolastretcher.process(inputAudiochannels.at(i), outputAudiochannels.at(i));
    }

    tempAudio.resize(outputAudiochannels.size() * outputAudiochannels.at(0).size());
    audiostretch::interleaveAudio(outputAudiochannels, tempAudio);
    //write the output file
    outputfile = outputfolder / inputfile.stem().concat( "_wsola");
    outputfile += inputfile.extension();
    outputinfo.format = inputInfo.format;
    outputinfo.channels = inputInfo.channels;
    outputinfo.samplerate = inputInfo.samplerate;
    audiofile = sf_open(outputfile.string().c_str(), SFM_WRITE, &outputinfo);
    sf_write_float(audiofile, tempAudio.data(), tempAudio.size());
    sf_close(audiofile);
    PLOGD << std::format("Outputting into {}", outputfile.string());
    return 0;   
}
