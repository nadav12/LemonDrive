/*
  ==============================================================================

    TLabel.h
    Created: 9 Jun 2022 9:28:33pm
    Author:  irishill

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "KnobDesign.h"


class TLabel : public juce::Label
{
    public:
    TLabel(){
        
       setJustificationType(juce::Justification::centredBottom);
        setFont (juce::Font (16.0f, juce::Font::bold));
        setColour (juce::Label::textColourId, juce::Colours::Sblackblue);
    }


};

;
