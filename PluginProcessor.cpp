/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Synth.h"

//==============================================================================
MetroAudioProcessor::MetroAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // read MIDI file into a stream...
    File currentMidiFile(LocalDataPath+midiFileName);
    if (currentMidiFile.exists()) {
        updateMIDIFile(LocalDataPath+midiFileName);
    }
    
    // init the synth...
    const int numVoices = 8;
    for (int i = numVoices; --i >= 0;)
        synth.addVoice (new SineWaveVoice());
    synth.addSound (new SineWaveSound());
    
    // init inputTapper velocity parameter...
    addParameter(gainsParam = new AudioParameterInt("inputVel", "Vel of input Tapper", 0, 127, 127));
    addParameter(velParam1 = new AudioParameterInt("synthVel1", "Vel of synth Tapper 1", 0, 127, 127));
    addParameter(velParam2 = new AudioParameterInt("synthVel2", "Vel of synth Tapper 2", 0, 127, 127));
    addParameter(velParam3 = new AudioParameterInt("synthVel3", "Vel of synth Tapper 3", 0, 127, 127));
    addParameter(TKNoiseParam1 = new AudioParameterFloat("synthTKNoise1", "Time Keeper Noise of synth Tapper 1", 0, 200, 0));
    addParameter(TKNoiseParam2 = new AudioParameterFloat("synthTKNoise2", "Time Keeper Noise of synth Tapper 2", 0, 200, 0));
    addParameter(TKNoiseParam3 = new AudioParameterFloat("synthTKNoise3", "Time Keeper Noise of synth Tapper 3", 0, 200, 0));
    addParameter(MNoiseParam1 = new AudioParameterFloat("synthMNoise1", "Motor Noise of synth Tapper 1", 0, 50, 0));
    addParameter(MNoiseParam2 = new AudioParameterFloat("synthMNoise2", "Motor Noise of synth Tapper 2", 0, 50, 0));
    addParameter(MNoiseParam3 = new AudioParameterFloat("synthMNoise3", "Motor Noise of synth Tapper 3", 0, 50, 0));
}

MetroAudioProcessor::~MetroAudioProcessor()
{
}


//==============================================================================
void MetroAudioProcessor::updateMIDIFile(String midiInputString)
{
    //clear MIDISeq buffers...
    for (int i=0; i<inputMIDISeq.size(); i++) {
        inputMIDISeq[i]->clear();
    }
    inputMIDISeq.clear();
    
    Logger::outputDebugString("in Processor: "+midiInputString+" has been loaded");
    
    // create input stream from file...
    inputmidifile = midiInputString;
    
    //
    FileInputStream MIDIData(inputmidifile);
    
    // create a MIDIFile object from the stream...
    MidiFile midiFileInput;
    midiFileInput.readFrom(MIDIData);
    
    // Create a sequence of MidiMessages...
    int numTracks = midiFileInput.getNumTracks();
    int activeTracknum = 0;
    for (int trackNum=0; trackNum<numTracks; trackNum++)
    {
        // iterate midimessages to check for noteOns...
        MidiMessageSequence currentTrack = *midiFileInput.getTrack(trackNum);
        int trackContainsNoteOnMessages = 0;
        for (int eventNum=0; eventNum<currentTrack.getNumEvents(); eventNum++)
        {
            if (currentTrack.getEventPointer(eventNum)->message.isNoteOn())
            {
                trackContainsNoteOnMessages++;
            }
        }
        
        // add the tracks to the inputMIDISeq array
        if(trackContainsNoteOnMessages)
        {
            inputMIDISeq.add(new MidiMessageSequence);
            inputMIDISeq[activeTracknum]->addSequence(currentTrack, 0.00);
            activeTracknum++;
        }
    }
}


void MetroAudioProcessor::printMIDIMessages()
{
    Logger::outputDebugString("File: "+inputmidifile.getFileName()+"..............");
    int numTracks = inputMIDISeq.size();
    for (int trackNum=0; trackNum < numTracks; trackNum++)
    {
        int activeNoteCounter = 0;
        int numEvents = inputMIDISeq[trackNum]->getNumEvents();
        for (int eventNum=0; eventNum<numEvents; eventNum++)
        {
            MidiMessageSequence::MidiEventHolder *tempEventHolder = inputMIDISeq[trackNum]->getEventPointer(eventNum);
            if(tempEventHolder->message.isNoteOn())
            {
                Logger::outputDebugString("x{"+String(trackNum+1)+"}("+String(activeNoteCounter+1)+") = "+String(inputMIDISeq[trackNum]->getEventTime(eventNum)));
                activeNoteCounter++;
            }
        }
    }
}

void MetroAudioProcessor::findAlignedMidiNotes()
{
    //only keep the midiMessages with N players performing simultaneously...
    
}


const String MetroAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MetroAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MetroAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double MetroAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MetroAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MetroAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MetroAudioProcessor::setCurrentProgram (int index)
{
}

const String MetroAudioProcessor::getProgramName (int index)
{
    return String();
}

void MetroAudioProcessor::changeProgramName (int index, const String& newName)
{

}


void MetroAudioProcessor::updateInputTapperVelocity(int vel)
{
    *gainsParam = vel;
    
    // check tapManager exists...
    if(tappersAlreadyAllocated)
    {
        tapManager->inputTapper.setVel(*gainsParam);
    }
    Logger::outputDebugString("Processor (input gain): "+String(*gainsParam));
}

void MetroAudioProcessor::updateSynthTapperTKNoise(int tapperNum, float noiseInMs)
{
    if (tapperNum==0)
        *TKNoiseParam1 = noiseInMs;
    else if (tapperNum==1)
        *TKNoiseParam2 = noiseInMs;
    else if (tapperNum==2)
        *TKNoiseParam3 = noiseInMs;
    // check tapManager exists...
    if(tappersAlreadyAllocated)
    {
        for (int  tapper = 0; tapper<numSynthesizedTappers; tapper++)
        {
            if (tapperNum==tapper)
                tapManager->synthesizedTappers[tapper]->TKNoiseStd = noiseInMs;
        }
    }
    Logger::outputDebugString("Processor (synth"+String(tapperNum)+" noise): "+String(tapManager->synthesizedTappers[tapperNum]->TKNoiseStd));
}

void MetroAudioProcessor::updateSynthTapperMNoise(int tapperNum, float noiseInMs)
{
    if (tapperNum==0)
        *MNoiseParam1 = noiseInMs;
    else if (tapperNum==1)
        *MNoiseParam2 = noiseInMs;
    else if (tapperNum==2)
        *MNoiseParam3 = noiseInMs;
    // check tapManager exists...
    if(tappersAlreadyAllocated)
    {
        for (int  tapper = 0; tapper<numSynthesizedTappers; tapper++)
        {
            if (tapperNum==tapper)
                tapManager->synthesizedTappers[tapper]->MNoiseStd = noiseInMs;
        }
    }
}

void MetroAudioProcessor::updatedSynthTapperVelocity(int tapperNum, int vel)
{
    // this is stupid - figure out the loop method and implement it here!
    if (tapperNum==0)
    {
        *velParam1 = vel;
    }
    else if (tapperNum==1)
    {
        *velParam2 = vel;
    }
    else if (tapperNum==2)
    {
        *velParam3 = vel;
    }
    
    // check tapManager exists...
    if(tappersAlreadyAllocated)
    {
        for (int  tapper = 0; tapper<numSynthesizedTappers; tapper++)
        {
            if (tapperNum==tapper)
                tapManager->synthesizedTappers[tapper]->setVel(vel);
        }
    }
    Logger::outputDebugString("Processor (synth"+String(tapperNum)+" gain): "+String(vel));
}

void MetroAudioProcessor::transferUIParamsToTappers()
{
    tapManager->velocities.set(0, *gainsParam);
    tapManager->velocities.set(1, *velParam1);
    tapManager->velocities.set(2, *velParam1);
    tapManager->velocities.set(3, *velParam1);
    tapManager->synthesizedTappers[0]->TKNoiseStd = *TKNoiseParam1;
    tapManager->synthesizedTappers[1]->TKNoiseStd = *TKNoiseParam2;
    tapManager->synthesizedTappers[2]->TKNoiseStd = *TKNoiseParam3;
    tapManager->synthesizedTappers[0]->MNoiseStd = *MNoiseParam1;
    tapManager->synthesizedTappers[1]->MNoiseStd = *MNoiseParam2;
    tapManager->synthesizedTappers[2]->MNoiseStd = *MNoiseParam3;
}

//==============================================================================
//========= Prepare to Play ====================================================
//==============================================================================
void MetroAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    if(!tappersAlreadyAllocated) // only allocate the tappers once
    {
        tapManager = new TapGenerator(numSynthesizedTappers+1, newSampleRate, samplesPerBlock, LocalDataPath);
        transferUIParamsToTappers();
        tappersAlreadyAllocated = true;
    }
    synth.setCurrentPlaybackSampleRate (newSampleRate);
}

bool MetroAudioProcessor::bpmValueChanged()
{
    if(currentBPM!=prevBPM)
    {
        prevBPM = currentBPM;
        return true;
    }
    else
    {
        prevBPM = currentBPM;
        return false;
    }
}

//==============================================================================
//========= Process Block ======================================================
//==============================================================================
void MetroAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    //update params...
    gain1 = *gainsParam;

    // get bpm...
    getPlayHead()->getCurrentPosition(playhead);
    currentBPM = playhead.bpm;
    
    // do this when plugin opens (we don't have access to an accurate bpm in prepareToPlay())
    if( (globalCounter.inSamples() == 0 && !bpmAlreadyAllocated)|| bpmValueChanged())
    {
        bpmAlreadyAllocated = true;
        tapManager->updateBPM(currentBPM);
        Logger::outputDebugString("global counter: "+String(globalCounter.inSamples()));
        Logger::outputDebugString("BPM changed to: "+String(currentBPM));
    }
    thisBlockPlaying = playhead.isPlaying;
    if (thisBlockPlaying) // ...playhead moving
    {
        // if the playhead is moving, start tapping...
        int blockSize = buffer.getNumSamples();
        tapManager->nextBlock(midiMessages, globalCounter, blockSize);
    }
    else if(!thisBlockPlaying && lastBlockPlaying) // ...playhead just stopped
    {
        // if the playhead just stopped moving, reset the tapManager and kill all tappers...
        tapManager->killActiveTappers(midiMessages);
        tapManager->reset();
        transferUIParamsToTappers();
        globalCounter.reset();
        tapManager->initLogFile();
    }
    else // ...playhead not moving
    {
        
    }
    
    // send the midi messages to the Synth...
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    // counter++
    frameCounter.iterate();
    lastBlockPlaying = thisBlockPlaying;
}


//==============================================================================
//========= Release ============================================================
//==============================================================================
void MetroAudioProcessor::releaseResources()
{
    // write everything that left to the flie...

    
}






#ifndef JucePlugin_PreferredChannelConfigurations
bool MetroAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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









//==============================================================================
bool MetroAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MetroAudioProcessor::createEditor()
{
    return new MetroAudioProcessorEditor (*this);
}

//==============================================================================
void MetroAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MetroAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MetroAudioProcessor();
}
