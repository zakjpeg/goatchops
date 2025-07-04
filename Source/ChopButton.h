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
    void shiftTiming(const double inShiftTiming);
    void setTiming(const double inTiming);
    double getTiming();
    juce::String getTimingInMinutesSeconds();

private:
    double timing;

    // Function: getTimingInMinutesSeconds
    // Purpose: Getter function for a ChopButton's timing in minutes/seconds (ex: 2:14)
    // Input: none
    // Output: a string representing this ChopButton's timing in minutes/seconds
    juce::String timingInMinutesSeconds;
};
