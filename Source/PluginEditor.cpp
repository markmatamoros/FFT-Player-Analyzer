/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

 Author: Mark Matamoros
 File Description: PluginEditor implementation file - for Plugin GUI (Animation window)
 Reference Link: https://docs.juce.com/master/tutorial_simple_fft.html
 Plugin Description: FFT Analyzer built with JUCE's FFT tutorial as a reference point
                     Plugin is intended for live performance
                     Further animation can be handled with sound sources in DAW
 
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setSize (700, 500);     //set default window size
    startTimerHz (30);      //sets callback timing (for redrawing): "1000/n"
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);       //set background to black
    g.setOpacity (1.0f);                    //set opacity of animation
    
    //draw GUI window with recieved image (bitmap)
    g.drawImage (audioProcessor.getImage(), getLocalBounds().toFloat());
}

void NewProjectAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    //set GUI window to be resizable
    setResizable(true, true);
}
