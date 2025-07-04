#include "MainComponent.h"
#include <sstream>



//==============================================================================
MainComponent::MainComponent()
    // Initializer list
    : thumbnailCache(5), 
      thumbnail(512, formatManager, thumbnailCache)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    // Performance enhancements
    setOpaque(true);

    // Set keyboard focus to window
    setWantsKeyboardFocus(true);

    // Set isFileLoaded
    isFileLoaded = false;

    // Initialize audio thumbnail components
    thumbnail.addChangeListener(this);

    // Initialize chopButtons
    for (int i = 0; i < 9; i++)
    {   
        // OPTIONAL: Set chop name while initializing
        auto* button = new ChopButton("Chop " + juce::String(i + 1));
        button->setLookAndFeel(&flatLookAndFeel);
        button->setColour(juce::TextButton::buttonColourId, juce::Colour::fromString("FFF0F1F4"));
        addAndMakeVisible(button);
        chopButtons.add(button);
    }

    // Initialize fileName
    fileName.setText("Select a file...", juce::dontSendNotification);
    fileName.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(fileName);

    // Initialize loadSampleButton
    loadSampleButton = new juce::TextButton("Load Sample");
    loadSampleButton->setLookAndFeel(&minorLookAndFeel);
    loadSampleButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xffEDEFF2));
    loadSampleButton->setColour(juce::TextButton::textColourOnId, juce::Colour(0xff6B7A88));
    loadSampleButton->repaint();
    loadSampleButton->onClick = [this] { loadFile();  };
    addAndMakeVisible(loadSampleButton);
    addAndMakeVisible(fileName);

    // Initialize playButton
    playButton = new juce::TextButton("Play");
    playButton->setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton->onClick = [this] { playButtonClicked(); };
    playButton->setEnabled(true);
    addAndMakeVisible(playButton);

    // Initialize stopButton
    stopButton = new juce::TextButton("Stop");
    stopButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    stopButton->onClick = [this] { stopButtonClicked(); };
    stopButton->setEnabled(false);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(fileHandlingContainer);
    addAndMakeVisible(fileName);
    addAndMakeVisible(*loadSampleButton);

    // Initialize toggleHoldButton
    toggleHold = false;
    toggleHoldButton = new juce::TextButton("Toggle Hold");
    toggleHoldButton->onClick = [this] {
        toggleHold = !toggleHold;
        };
    addAndMakeVisible(*toggleHoldButton);

    // Initialize Transport State
    state = Stopped;

    // allow for .wav and .aif files in sample loading process
    formatManager.registerBasicFormats();

    // Start timer for Audio Thumbnail playhead
    startTimer(40);

    // REQUIRED
    setSize(600, 700);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::white);

    // Paint the audio thumbnail
    juce::Rectangle<float> thumbnailBounds(25, 70, getWidth() - 50, 100);
    juce::Rectangle<int> thumbnailBoundsInt(25, 70, getWidth() - 50, 100);

    if (thumbnail.getNumChannels() == 0)
    {
        paintIfNoFileLoaded(g, thumbnailBounds, thumbnailBoundsInt);
    }
    else {
        paintIfFileLoaded(g, thumbnailBounds, thumbnailBoundsInt);

        // Draw playhead
        auto drawArea = thumbnailBounds;
        double proportion = transport.getCurrentPosition() / transport.getLengthInSeconds();
        double xCoord = drawArea.getX() + proportion * drawArea.getWidth();
        g.setColour(juce::Colours::green);
        g.drawLine(xCoord, drawArea.getY(), xCoord, drawArea.getBottom(), 1.0f);

        // Draw trail when playing
        if (state == TransportState::Starting)
        {
            g.setColour(juce::Colour::fromString("FF64DC32")); // 100%
            g.drawLine(xCoord - 1, drawArea.getY(), xCoord - 1, drawArea.getBottom(), 1.0f);

            g.setColour(juce::Colour::fromString("8064DC32")); // 50%
            g.drawLine(xCoord - 2, drawArea.getY(), xCoord - 2, drawArea.getBottom(), 1.0f);

            g.setColour(juce::Colour::fromString("4D64DC32")); // 30%
            g.drawLine(xCoord - 3, drawArea.getY(), xCoord - 3, drawArea.getBottom(), 1.0f);

            g.setColour(juce::Colour::fromString("2664DC32")); // 15%
            g.drawLine(xCoord - 4, drawArea.getY(), xCoord - 4, drawArea.getBottom(), 1.0f);

            g.setColour(juce::Colour::fromString("1964DC32")); // 10%
            g.drawLine(xCoord - 5, drawArea.getY(), xCoord - 5, drawArea.getBottom(), 1.0f);

            g.setColour(juce::Colour::fromString("1164DC32")); // 7%
            g.drawLine(xCoord - 6, drawArea.getY(), xCoord - 6, drawArea.getBottom(), 1.0f);

            g.setColour(juce::Colour::fromString("0A64DC32")); // 4%
            g.drawLine(xCoord - 7, drawArea.getY(), xCoord - 7, drawArea.getBottom(), 1.0f);

            g.setColour(juce::Colour::fromString("0364DC32")); // 1%
            g.drawLine(xCoord - 8, drawArea.getY(), xCoord - 8, drawArea.getBottom(), 1.0f);

        }

        // Draw Chop Markers on audio thumbnail
        int i = 0;
        for (auto chopButton : chopButtons)
        {
            // Get chop color
            juce::Colour activeColor = chopColors[i];

            // Find chop's proportion
            double chopProportion = chopButton->getTiming() / transport.getTotalLength();
            double chopXCoord = drawArea.getX() + chopProportion * drawArea.getWidth();

            // Draw chop
            g.setColour(activeColor);
            g.drawLine(chopXCoord, drawArea.getY(), chopXCoord, drawArea.getBottom(), 1.0f);

            // Increment i for next color
            i++;
        }


    }
        
}

// Function: loadFile
// Purpose: Loads audio file, updates waveforms and chops
void MainComponent::loadFile()
{

    // Choose a file
    juce::FileChooser chooser(
        /* Dialogue window title */
        "Choose a Sample",
        /* Default browser directory */
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
        /* Permitted filetypes */
        "*.wav;*.mp3;*.aiff"
    );

    // If the user chooses a file
    if (chooser.browseForFileToOpen())
    {
        // Store user's selection
        juce::File selectedFile = chooser.getResult();

        // Update isFileLoaded
        isFileLoaded == true;

        // Read the file format
        juce::AudioFormatReader* formatReader = formatManager.createReaderFor(selectedFile);

        // Prepare file for playback
        std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(formatReader, true));

        // Set the transport's source to data from tempSource
        transport.setSource(tempSource.get());

        // Pass prepped file to playSource
        playSource.reset(tempSource.release());

        // Set up chops
        setChops();
        
        // Update fileName
        fileName.setText(selectedFile.getFileNameWithoutExtension(), juce::dontSendNotification);

        // Update filePath
        filePath = selectedFile.getFullPathName();

        // Set thumbnail source
        thumbnail.setSource(new juce::FileInputSource(selectedFile));

        // DEBUG: Log data in console
        DBG(formatReader->getFormatName());
        DBG(transport.getLengthInSeconds());
        DBG(selectedFile.getFileName());
    }

    // Instantiate new file chooser
    fileChooser.reset(new juce::FileChooser("Select a sample...", juce::File(), "*.wav;*.mp3"));

}

// Function: prepareToPlay
// Purpose: Prepares transport to play loaded audio
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transport.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

// Function: getNextAudioBlock
// Purpose: Loads audio into buffer
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    transport.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    transport.releaseResources();
}

// Function: playButtonClicked
// Purpose: Plays a loaded audio file
void MainComponent::playButtonClicked()
{
    transportStateChanged(Starting);
}

// Function: stopButtonClicked
// Purpose: stops a playing audio file
void MainComponent::stopButtonClicked()
{
    transportStateChanged(Stopping);
}

// Function: transportStateChanged
// Purpose: Handle changes in the transport's state
void MainComponent::transportStateChanged(TransportState newState)
{
    // Check if state is changed
    if (newState != state)
    {
        state = newState;

        switch (state)
        {
        case Stopped:
            playButton->setEnabled(true);
            // bring transport to the beginning
            transport.setPosition(0.0);
            break;

        case Starting:
            playButton->setEnabled(false);
            stopButton->setEnabled(true);
            // transport play
            transport.start();
            break;

        case Stopping:
            playButton->setEnabled(true);
            stopButton->setEnabled(false);
            // transport stop
            DBG(transport.getCurrentPosition());
            transport.stop();
            break;
        }
    }
}

// Function: setChops
// Purpose: Sets ALL chop buttons timing according to transport's length
void MainComponent::setChops()
{
    // Store the transport length
    double length = transport.getLengthInSeconds();

    // Loop through our chop buttons
    int i = 1;
    for (auto& chopButton : chopButtons)
    {
;

        // Set this ChopButton's timing
        chopButton->setTiming((length / 10) * i);
        double thisTiming = chopButton->getTiming();

        // Update this ChopButton's visuals
        chopButton->setButtonText(chopButton->getTimingInMinutesSeconds());
        chopButton->setColour(juce::TextButton::buttonColourId, chopColors[i - 1]);
        chopButton->repaint();

        // Set chop button's actions
        chopButton->onClick = [this, chopButton] {

            // Set transport to specific chop location
            transport.setPosition(chopButton->getTiming());

            if (!transport.isPlaying())
            {
                transport.start();
            }
            transportStateChanged(Starting);
        };
        i++;
    }
}

// Function: keyPressed
// Purpose: Detects and handles user keyboard input
bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    // Prohibit repeated key inputs when user holds
    if (key == activeKey)
    {
        return true;
    }
    else if (key == juce::KeyPress::numberPad0)
    {
        stopButton->triggerClick();
    }
    else if (key == juce::KeyPress::spaceKey)
    {
        playButton->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad1)
    {
        chopButtons[0]->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad2)
    {
        chopButtons[1]->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad3)
    {
        chopButtons[2]->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad4)
    {
        chopButtons[3]->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad5)
    {
        chopButtons[4]->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad6)
    {
        chopButtons[5]->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad7)
    {
        chopButtons[6]->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad8)
    {
        chopButtons[7]->triggerClick();
    }
    else if (key == juce::KeyPress::numberPad9)
    {
        chopButtons[8]->triggerClick();
    }

    activeKey = key;

    return true;

}

bool MainComponent::keyStateChanged(bool isKeyDown)
{
    // Stop the transport if key is released and toggleHold is on
    if (!isKeyDown && toggleHold && !juce::KeyPress::isKeyCurrentlyDown(activeKey.getKeyCode()))
    {
        transport.stop();
    }

    // If active key isn't being pressed, reset activekey
    if (!juce::KeyPress::isKeyCurrentlyDown(activeKey.getKeyCode()))
    {
        activeKey = juce::KeyPress();
    }

    // This is insignificant
    return isKeyDown;
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transport)
    {
        transportSourceChanged();
    }
    if (source == &thumbnail)
    {
        thumbnailChanged();
    }
}

void MainComponent::transportSourceChanged()
{
    transportStateChanged(transport.isPlaying() ? Starting : Stopped);
}

void MainComponent::thumbnailChanged()
{
    repaint();
}

void MainComponent::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<float>& thumbnailBounds, const juce::Rectangle<int>& thumbnailBoundsInt)
{
    g.setColour(juce::Colour::fromString("FFF4F7FA"));
    g.fillRoundedRectangle(thumbnailBounds, 15.0f);
    g.setColour(juce::Colour::fromString("FFD6E2EE"));
    g.drawFittedText("No File Loaded", thumbnailBoundsInt, juce::Justification::centred, 1);
}

void MainComponent::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<float>& thumbnailBounds, const juce::Rectangle<int>& thumbnailBoundsInt)
{
    g.setColour(juce::Colour::fromString("FFF4F7FA"));
    g.fillRoundedRectangle(thumbnailBounds, 15.0f);
    g.setColour(juce::Colour::fromString("FFA0AEC0"));
    juce::Rectangle<int> thumbnailBoundsInner(25, 80, getWidth() - 50, 80);

    thumbnail.drawChannel(g,
        thumbnailBoundsInner,
        0.0,
        thumbnail.getTotalLength(),
        1.0f,
        1);


}

void MainComponent::timerCallback()
{
    // Repaint (for audio thumbnail playhead)
    repaint();

    // Change transport state to "Stopping" once it's reached end of file playback
    if (transport.getCurrentPosition() >= transport.getLengthInSeconds())
    {
        transportStateChanged(TransportState::Stopping);
    }
}

void MainComponent::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.flexWrap = juce::FlexBox::Wrap::noWrap;
    fb.justifyContent = juce::FlexBox::JustifyContent::center;
    fb.alignContent = juce::FlexBox::AlignContent::center;

    // ───────────── File Handling (Container Required) ─────────────
    juce::FlexBox fileHandling;
    fileHandling.flexDirection = juce::FlexBox::Direction::row;
    fileHandling.justifyContent = juce::FlexBox::JustifyContent::center;
    fileHandling.alignItems = juce::FlexBox::AlignItems::center;
    fileHandling.items.add(juce::FlexItem(fileName)
        .withMinWidth(368.0f).withMinHeight(50.0f));
    fileHandling.items.add(juce::FlexItem(*loadSampleButton)
        .withMinWidth(174.0f).withMinHeight(50.0f).withMargin(5.0f));

    fb.items.add(juce::FlexItem(fileHandlingContainer)
        .withMinWidth(600.0f).withMinHeight(50.0f).withMargin(0.0f)
        .withAlignSelf(juce::FlexItem::AlignSelf::center));

    // ───────────── Controls (Play/Stop/ToggleHold) ─────────────
    juce::FlexBox controls;
    controls.flexDirection = juce::FlexBox::Direction::row;
    controls.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    controls.items.add(juce::FlexItem(*playButton)
        .withMinWidth(174.0f).withMinHeight(25.0f).withMargin(5.0f));
    controls.items.add(juce::FlexItem(*stopButton)
        .withMinWidth(174.0f).withMinHeight(25.0f).withMargin(5.0f));
    controls.items.add(juce::FlexItem(*toggleHoldButton)
        .withMinWidth(174.0f).withMinHeight(25.0f).withMargin(5.0f));

    fb.items.add(juce::FlexItem(controls)
        .withMinWidth(300.0f).withMinHeight(50.0f).withMargin(5.0f).withMaxWidth(550.0f));

    // ───────────── Chop Grid (3 Rows of 3 Buttons) ─────────────
    juce::FlexBox chopRow1, chopRow2, chopRow3;
    chopRow1.flexDirection = chopRow2.flexDirection = chopRow3.flexDirection = juce::FlexBox::Direction::row;
    chopRow1.justifyContent = chopRow2.justifyContent = chopRow3.justifyContent = juce::FlexBox::JustifyContent::center;

    for (int i = 0; i < 3; ++i)
        chopRow1.items.add(juce::FlexItem(*chopButtons[i])
            .withWidth(174.0f).withHeight(100.0f).withMargin(5.0f));

    for (int i = 3; i < 6; ++i)
        chopRow2.items.add(juce::FlexItem(*chopButtons[i])
            .withWidth(174.0f).withHeight(100.0f).withMargin(5.0f));

    for (int i = 6; i < 9; ++i)
        chopRow3.items.add(juce::FlexItem(*chopButtons[i])
            .withWidth(174.0f).withHeight(100.0f).withMargin(5.0f));

    juce::FlexBox chops;
    chops.flexDirection = juce::FlexBox::Direction::column;

    chops.items.add(juce::FlexItem(chopRow3).withFlex(1.0f));
    chops.items.add(juce::FlexItem(chopRow2).withFlex(1.0f));
    chops.items.add(juce::FlexItem(chopRow1).withFlex(1.0f));

    fb.items.add(juce::FlexItem(chops)
        .withMinWidth(600.0f).withMinHeight(334.0f).withMargin(0.0f));

    // ───────────── Layout ─────────────
    fb.justifyContent = juce::FlexBox::JustifyContent::center;
    fb.performLayout(getLocalBounds());

    // Perform nested layout inside the fileHandlingContainer
    fileHandling.performLayout(fileHandlingContainer.getLocalBounds());
}


