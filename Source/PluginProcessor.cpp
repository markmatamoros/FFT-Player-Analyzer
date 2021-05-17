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

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    forwardFFT(fftOrder), spectrogramImage(juce::Image::RGB, 512, 512, true)
    //ForwardFFT and spectrogramImage initialization

#endif
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
        
        //push samples into pre-FFT array
        for (auto i = 0; i < buffer.getNumSamples(); ++i){
            pushNextSampleIntoFifo(channelData[i]);
        }
    }
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}

//Recieves samples and prepares an array for FFT processing
void NewProjectAudioProcessor::pushNextSampleIntoFifo(float sample)
{
    //processing commences upon recieving a full array (block) of samples
    if (fifoIndex == NewProjectAudioProcessor::fftSize)
    {
        //push first fifo array (unprocessed samples) into
        //first half of fifoData array (to be processed)
        if (!nextFFTBlockReady)
        {
            std::fill (fftData.begin(), fftData.end(), 0.0f);
            std::copy (fifo.begin(), fifo.end(), fftData.begin());
            nextFFTBlockReady = true;
        }
        
        //reset index upon filling
        fifoIndex = 0;
    }
    
    //fill fifo array with samples
    fifo[(size_t) fifoIndex++] = sample;
}

//handles FFT animation
void NewProjectAudioProcessor::drawFFTAnimation()
{
    float xCoord = 0.0;     //holds drawn rectangle's x-coordinate position
    
    juce::Graphics g (spectrogramImage);        //set image to be drawn upon
    
    //Clear plugin window by drawing a black background
    g.setColour (juce::Colours::black);
    g.fillRect (0, 0, spectrogramImage.getWidth(), spectrogramImage.getHeight());
    
    // then render our FFT data..
    forwardFFT.performFrequencyOnlyForwardTransform (fftData.data());
    
    //set drawing color to red for drawn rectangles
    g.setColour (juce::Colours::red);

    //loop through processed FFT block for drawing coordinates
    for (int i = 0; i < fftData.size(); i++)
    {
        //determine rectangle's x-coordinate via frequency band (array position)
        xCoord = (float)i/128.0 * spectrogramImage.getWidth();
        
        //draw rectangle (not filled, only stroke)
        g.drawRect(xCoord, (float)0, (float)fftData[i], (float)spectrogramImage.getHeight(), 1.0f);
    }
}

//for grabbing bitmap (pluginEditor call)
juce::Image NewProjectAudioProcessor::getImage()
{
    return spectrogramImage;
}

//checks bool value for commencing draw (pluginEditor call)
bool NewProjectAudioProcessor::getIsReady()
{
    return nextFFTBlockReady;
}

//resets bool value via pluginEditor call (bool utilized in pushNextSampleIntoFifo function)
void NewProjectAudioProcessor::setIsReady()
{
    nextFFTBlockReady = false;
}
