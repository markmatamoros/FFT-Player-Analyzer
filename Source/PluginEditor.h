/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

 Author: Mark Matamoros
 File Description: PluginEditor header file - for Plugin GUI (Animation window)
 Reference Link: https://docs.juce.com/master/tutorial_simple_fft.html
 Plugin Description: FFT Analyzer built with JUCE's FFT tutorial as a reference point
                     Plugin is intended for live performance
                     Further animation can be handled with sound sources in DAW
 
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NewProjectAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //updates animation (spectogram)
    void timerCallback() override
    {
        //update GUI winow only when FFT block is ready for processing
        //In other words, "fifo" array has been dumped into "fifoData" array
        if (audioProcessor.getIsReady())
        {
            audioProcessor.drawFFTAnimation();         //call FFT animation function
            audioProcessor.setIsReady();               //prep FIFO function for filling
            
            repaint();      //redraw animation (check "paint" function)
        }
    }
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NewProjectAudioProcessor& audioProcessor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
