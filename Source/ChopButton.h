/*
  ==============================================================================

    ChopButton.h
    Created: 28 Jun 2025 6:10:58pm
    Author:  dzak1

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class ChopButton : public juce::TextButton
{
public:
    ChopButton(const juce::String& name);
    void mouseUp(const juce::MouseEvent& event) override;
    void setTiming(const double inTiming);
    double getTiming();
    juce::String getTimingInMinutesSeconds();

private:
    double timing;
    juce::String timingInMinutesSeconds;
};
