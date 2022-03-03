/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "TapManager.hpp"


//==============================================================================
/**
*/
class MetroAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    MetroAudioProcessor();
    ~MetroAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void updateMIDIFile(String midiInput);
    void printMIDIMessages();
    void findAlignedMidiNotes();
    
    
    // This needs to be set to the right path by the user! I'll fix this in the next version.
    // cuurently needs a foward slash on the end...
//     String LocalDataPath = "/Users/ryanstables/Repos/AdaptiveMetonome/Data/";
    String LocalDataPath = "/Users/dmtlab/Documents/Repos/AdaptiveMetonome/Data/";
    String midiFileName = "haydn2.mid";
    
    // parameters for the gains and noise...
    AudioParameterInt *gainsParam, *velParam1, *velParam2, *velParam3;
    AudioParameterFloat *TKNoiseParam1, *TKNoiseParam2, *TKNoiseParam3;
    AudioParameterFloat *MNoiseParam1, *MNoiseParam2, *MNoiseParam3;
    
    void updateInputTapperVelocity(int vel);
    void updatedSynthTapperVelocity(int tapperNum, int vel);
    void updateSynthTapperTKNoise(int tapperNum, float noiseInMs);
    void updateSynthTapperMNoise(int tapperNum, float noiseInMs);
    void transferUIParamsToTappers();
    int getNumTappers(){return numSynthesizedTappers;};

    ScopedPointer<TapGenerator>  tapManager; // this can be edited by the UI

private:
    // MIDI Input Data...
    File inputmidifile;
    OwnedArray<MidiMessageSequence> inputMIDISeq;
    
    // tappers...
    int numSynthesizedTappers    = 3;
    int numInputTappers          = 1;
    
    bool tappersAlreadyAllocated = false, bpmAlreadyAllocated = false;
    Counter globalCounter, frameCounter;
    
    Synthesiser synth;
    AudioPlayHead::CurrentPositionInfo playhead;
    double currentBPM = 0, prevBPM = 0;
    
    bool bpmValueChanged();
    
    //parameters to pass to the TapManager (to be removed and inserted directly)
    int gain1;
    
    bool thisBlockPlaying = false,
         lastBlockPlaying = false;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MetroAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
