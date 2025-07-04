#pragma once

#include <JuceHeader.h>
#include <vector>
#include "ChopButton.h"


class FlatLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& /*backgroundColour*/,
        bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // Use a base fill colour (white by default)
        auto baseColour = button.findColour(juce::TextButton::buttonColourId);

        if (isButtonDown)
            baseColour = baseColour.darker(0.2f);
        else if (isMouseOverButton)
            baseColour = baseColour.brighter(0.1f);

        float cornerSize = 15.0f;
        float strokeThickness = 1.5f;

        // Fill the background
        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, cornerSize);

        // Draw outline
        g.setColour(juce::Colour::fromString("22444444")); // semi-transparent grey stroke
        g.drawRoundedRectangle(bounds.reduced(strokeThickness * 0.5f), cornerSize, strokeThickness);
    }


    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);

        g.setColour(juce::Colour::fromString("FF444444"));  // <-- your desired text color

        const int yIndent = juce::jmin(4, button.getHeight() / 30);
        const int cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;

        juce::Rectangle<int> textArea(button.getLocalBounds().reduced(cornerSize / 2, yIndent));

        g.drawFittedText(button.getButtonText(), textArea, juce::Justification::centred, 2);
    }
};


class MinorLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& /*backgroundColour*/,
        bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // Use a base fill colour (white by default)
        auto baseColour = button.findColour(juce::TextButton::buttonColourId);

        if (isButtonDown)
            baseColour = baseColour.darker(0.2f);
        else if (isMouseOverButton)
            baseColour = baseColour.brighter(0.1f);

        float cornerSize = 15.0f;
        float strokeThickness = 0.0f;

        // Fill the background
        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, cornerSize);

        // Draw outline
        g.setColour(juce::Colour::fromString("22444444")); // semi-transparent grey stroke
        g.drawRoundedRectangle(bounds.reduced(strokeThickness * 0.5f), cornerSize, strokeThickness);
    }


    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);

        g.setColour(juce::Colour::fromString("FF444444"));  // <-- your desired text color

        const int yIndent = juce::jmin(4, button.getHeight() / 30);
        const int cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;

        juce::Rectangle<int> textArea(button.getLocalBounds().reduced(cornerSize / 2, yIndent));

        g.drawFittedText(button.getButtonText(), textArea, juce::Justification::centred, 2);
    }
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener,
                       private juce::Timer // for Audio Thumbnail playhead
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;


private:
    //==============================================================================
    // Your private member variables go here...
        // TRANSPORT STATE
    enum TransportState
    {
        Stopped,
        Starting,
        Stopping,
    };

    //=======================Zak's Implemented Funcs================================
    void loadFile();
    void playButtonClicked();
    void stopButtonClicked();
    void transportStateChanged(TransportState newState);
    void setChops();
    bool keyPressed(const juce::KeyPress& key) override;
    bool keyStateChanged(bool isKeyDown) override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    void transportSourceChanged();
    void thumbnailChanged();
    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<float>& thumbnailBounds, const juce::Rectangle<int>& thumbnailBoundsInt);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<float>& thumbnailBounds, const juce::Rectangle<int>& thumbnailBoundsInt);
    void timerCallback() override;


    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;

    // LOOK AND FEEL
    FlatLookAndFeel flatLookAndFeel;
    MinorLookAndFeel minorLookAndFeel;
    std::vector<juce::Colour> chopColors = {
        juce::Colour::fromString("FF5DAEFF"), // Soft Blue
        juce::Colour::fromString("FF4FD1C5"), // Aqua Mint
        juce::Colour::fromString("FFFFD166"), // Warm Yellow
        juce::Colour::fromString("FFFF7F66"), // Gentle Coral
        juce::Colour::fromString("FFF6AD55"), // Light Orange
        juce::Colour::fromString("FFF687B3"), // Pale Pink
        juce::Colour::fromString("FFB794F4"), // Soft Lavender
        juce::Colour::fromString("FF68D391"), // Fresh Green
        juce::Colour::fromString("FFA0AEC0"), // Cool Slate
        juce::Colour::fromString("FFE2E8F0"), // Subtle Gray
        juce::Colour::fromString("FFC3DAFE"), // Soft Light Blue
        juce::Colour::fromString("FFD6BCFA")  // Pastel Purple
    };


    // FILE LOADING
    juce::String filePath;
    juce::Label fileName;
    std::unique_ptr<juce::FileChooser> fileChooser;
    bool isFileLoaded;

    // AUDIO TRANSPORT
    juce::AudioTransportSource transport;
    TransportState state;

    // CHOP DATA
    juce::OwnedArray<ChopButton> chopButtons;

    // PLAYBACK CONTROL BUTTONS
    juce::TextButton* loadSampleButton = nullptr;
    juce::TextButton* playButton = nullptr;
    juce::TextButton* stopButton = nullptr;
    juce::Component fileHandlingContainer;

    // AUDIO THUMBNAIL WAVEFORM
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    // TOGGLE HOLD BUTTON
    juce::TextButton* toggleHoldButton;
    bool toggleHold;
    juce::KeyPress activeKey;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent);
};

// DEV NOTE: Left off at 29:01, TAP JUCE tutorial #37 https://www.youtube.com/watch?v=vVnu-L712ho&list=PLDaaBmiCre9BAUFJ1TQHdBJ0ouPl33dGe&index=7
// 6/27/2025 DEV NOTE: Tomorrow I want to add the playback marker, and chop markers. Maybe even make them interactable
