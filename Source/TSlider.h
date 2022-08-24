/*
  ==============================================================================

    TSlider.h
    Created: 28 Jun 2021 3:21:56am
    Author:  irishill

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "KnobDesign.h"
using namespace juce;

class TSlider : public juce::Slider
{
    public:
    TSlider(){
    setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
//    setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 50);
 
    }
};
