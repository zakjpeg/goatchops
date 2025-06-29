/*
  ==============================================================================

    ChopButton.cpp
    Created: 28 Jun 2025 6:11:05pm
    Author:  dzak1

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ChopButton.h"
#include <sstream>


ChopButton::ChopButton(const juce::String& name) : juce::TextButton(name)
{
    // Initialize timing to 0
    timing = 0;
    timingInMinutesSeconds = "0:00";
}

void ChopButton::mouseUp(const juce::MouseEvent& event)
{
    // If user right clicks this chop button
    if (event.mods.isRightButtonDown())
    {
        // Create context menu
        juce::PopupMenu menu;
        menu.addItem(1, "Pull 0.1s");
        menu.addItem(2, "Push 0.1s");

        // Show menu
        int result = menu.show();

        // Check was user selects
        if (result == 1)
        {
            // LOGIC GOES HERE
            DBG("Pull 0.1s clicked");
        }
        else if (result == 2)
        {
            DBG("Push 0.1s clicked");
        }
    }
    else
    {
        // Continue with mouse event if it's not rightclick
        TextButton::mouseUp(event);
    }
}

// Function: setTiming
// Purpose: Setter functions for a ChopButton's timing
// Input: double representing new timing
// Output: none
//
void ChopButton::setTiming(const double inTiming)
{
    // Set timing
    timing = inTiming;

    // Build new timingInMinutesSeconds
    int timeSeconds = std::round(inTiming);
    int secondsLeft = timeSeconds % 60;
    int minutesLeft = timeSeconds / 60;
    std::stringstream ss;
    ss << minutesLeft << ":";
    if (secondsLeft < 10) {
        ss << "0";
    }
    ss << secondsLeft;
    
    // Update timingInMinutesSeconds
    timingInMinutesSeconds = ss.str();
}

// Function: getTiming
// Purpose: Getter function for a ChopButton's timing
// Input: none
// Output: a double representing this ChopButton's timing
double ChopButton::getTiming()
{
    return timing;
}

// Function: getTimingInMinutesSeconds
// Purpose: Getter function for a ChopButton's timing in minutes/seconds (ex: 2:14)
// Input: none
// Output: a string representing this ChopButton's timing in minutes/seconds
juce::String ChopButton::getTimingInMinutesSeconds()
{
    return timingInMinutesSeconds;
}
