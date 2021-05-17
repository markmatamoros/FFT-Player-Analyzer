/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

 Author: Mark Matamoros
 File Description: PluginProcessor implementation file - for DSP (FFT)
 Reference Link: https://docs.juce.com/master/tutorial_simple_fft.html
 Plugin Description: FFT Analyzer built with JUCE's FFT tutorial as a reference point
                     Plugin is intended for live performance
                     Further animation can be handled with sound sources in DAW
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;
    
    //set FFT window size: 2^n points
    static constexpr auto fftOrder = 7;                 //128 points in this setup
    static constexpr auto fftSize  = 1 << fftOrder;     //use left bit shift operator for FFT analysis
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void pushNextSampleIntoFifo (float sample);     //pushes samples into an array for processing
    void drawFFTAnimation();                        //draws animation onto bitmap
    
    juce::Image getImage();                         //for grabbing bitmap (pluginEditor)
    bool getIsReady();                  //checks bool value for commencing draw (pluginEditor)
    void setIsReady();                  //resets bool value via pluginEditor (fifo function)
    
private:
    juce::dsp::FFT forwardFFT;                  //FFT object to perform FFT on
    juce::Image spectrogramImage;               //image for drawing analysis
    
    std::array<float, fftSize> fifo;            //array holds audio samples
    std::array<float, fftSize * 2> fftData;     //array holds results after FFT prcessing
    int fifoIndex = 0;                          //counter to track sample count in FIFO function
    bool nextFFTBlockReady = false;             //flag to trigger next FFT block analysis
    
//==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
