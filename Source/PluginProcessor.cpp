/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
LemonDriveAudioProcessor::LemonDriveAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

LemonDriveAudioProcessor::~LemonDriveAudioProcessor()
{
}

//==============================================================================
const juce::String LemonDriveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LemonDriveAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LemonDriveAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LemonDriveAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LemonDriveAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LemonDriveAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LemonDriveAudioProcessor::getCurrentProgram()
{
    return 0;
}

void LemonDriveAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String LemonDriveAudioProcessor::getProgramName (int index)
{
    return {};
}

void LemonDriveAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LemonDriveAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    updateFilters();
    leftChain.prepare(spec);
    rightChain.prepare(spec);
}
ChainSettings getChainSettings (juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.lowCutFreq = apvts.getRawParameterValue ("LOWCUT")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HIGHCUT")->load();

    return settings;
}
void LemonDriveAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LemonDriveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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


void LemonDriveAudioProcessor::updateLowCutFilter (const ChainSettings& chainSettings)
{
    auto lowCutCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass (getSampleRate(),
                                                                                 chainSettings.lowCutFreq
                                                                                        );
        
    *leftChain.get<ChainPositions::LowCut>().coefficients = *lowCutCoefficients;
    *rightChain.get<ChainPositions::LowCut>().coefficients = *lowCutCoefficients;
}
void LemonDriveAudioProcessor::updateHighCutFilter (const ChainSettings& chainSettings)
{
    auto highCutCoefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(getSampleRate(), chainSettings.highCutFreq);
    *leftChain.get<ChainPositions::HighCut>().coefficients = *highCutCoefficients;
    *rightChain.get<ChainPositions::HighCut>().coefficients = *highCutCoefficients;

}
void LemonDriveAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings (apvts);
    updateHighCutFilter(chainSettings);
    updateLowCutFilter (chainSettings);
}
void LemonDriveAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    auto drive = apvts.getRawParameterValue("DRIVE");
    auto range = apvts.getRawParameterValue("RANGE");
    auto volume = apvts.getRawParameterValue("VOLUME");
    auto curve = apvts.getRawParameterValue("CURVE");
  
    float driver = drive->load();
    float ranger = range->load();
    float volumer = volume->load();
    float curver = curve->load();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear (i, 0, buffer.getNumSamples());
    updateFilters();

    juce::dsp::AudioBlock<float> block (buffer);
    
    auto leftBlock = block.getSingleChannelBlock (0);
    auto rightBlock = block.getSingleChannelBlock (1);
    
    juce::dsp::ProcessContextReplacing<float> leftContext (leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext (rightBlock);
    
    leftChain.process (leftContext);
    rightChain.process (rightContext);
    
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for(int sample = 0; sample < buffer.getNumSamples(); sample ++)
        {

            channelData[sample] *= (driver * ranger);

            auto drivenSignal = 2.0f / M_PI * atan(M_PI/(1-curver) * channelData[sample]);
            channelData[sample] = drivenSignal * volumer;

        }
        
   }

}

//==============================================================================
bool LemonDriveAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LemonDriveAudioProcessor::createEditor()
{
    return new LemonDriveAudioProcessorEditor (*this);
}

//==============================================================================
void LemonDriveAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LemonDriveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}
void LemonDriveAudioProcessor::reset()
{
    rightChain.reset();
    leftChain.reset();
}

juce::AudioProcessorValueTreeState::ParameterLayout LemonDriveAudioProcessor:: createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DRIVE", "Drive", 0.f, 1.f, 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RANGE", "Range", 1.f, 150.f, 40.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VOLUME", "Volume", 0.f, 1.f, 0.999f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("LOWCUT", "LowCut", 20.f, 600.f, 250.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("HIGHCUT", "HighCut", 2000.f, 20000.f, 18000.f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CURVE", "Curve", 0.f, 0.9f, 0.5f));
    return {params.begin(), params.end()};
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LemonDriveAudioProcessor();
}