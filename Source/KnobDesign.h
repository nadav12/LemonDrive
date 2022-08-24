/*
  ==============================================================================

    KnobDesign.h
    Created: 28 Jun 2021 1:21:21am
    Author:  irishill

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
using namespace juce;

class KnobDesign : public juce::LookAndFeel_V4
{
public:
    juce::Slider::SliderLayout getSliderLayout (juce::Slider& slider) override
    {
        auto localBounds = slider.getLocalBounds();

        juce::Slider::SliderLayout layout;

        layout.textBoxBounds = localBounds.withY (-1);
        layout.sliderBounds = localBounds;
        
        return layout;
    }
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                            const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override {
         auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
         auto centreX = (float) x + (float) width  * 0.5f;
         auto centreY = (float) y + (float) height * 0.5f;
         auto rx = centreX - radius;
         auto ry = centreY - radius;
         auto rw = radius * 2.0f;
         auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
         
         // fill
     
         
        juce::Image fillImg = juce::ImageCache::getFromMemory(BinaryData::KnobImg_png,BinaryData::KnobImg_pngSize);
        g.drawImageWithin(fillImg, rx, ry, rw, rw, juce::RectanglePlacement::stretchToFit);
        g.fillEllipse (rx, ry, rw, rw);
         // outline
         g.setColour (juce::Colours::skyblue);
         g.drawEllipse (rx, ry, rw, rw, 1.2f);
         
         juce::Path p;
         auto pointerLength = radius * 0.40f;
         auto pointerThickness = 3.0f;
         p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
         p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
         g.setColour (juce::Colours::yellow);
           g.fillPath (p);
     }
    juce::Label* createSliderTextBox (juce::Slider& slider) override
    {
        auto* l = new juce::Label();

        l->setJustificationType (juce::Justification::centred);
        l->setColour (juce::Label::textColourId, slider.findColour (juce::Slider::textBoxTextColourId));
        l->setColour (juce::Label::textWhenEditingColourId, slider.findColour (juce::Slider::textBoxTextColourId));
        l->setColour (juce::Label::outlineWhenEditingColourId, juce::Colours::transparentWhite);
        l->setInterceptsMouseClicks (false, false);
        l->setFont (14.0f);

        return l;
    }


//    KnobDesign(){
//        setColour(Slider::rotarySliderOutlineColourId, Colours::rebeccapurple);
//        setColour(Slider::rotarySliderFillColourId, Colours::rebeccapurple);
//        setColour(Slider::thumbColourId, Colours::yellowgreen);
//    }
};
