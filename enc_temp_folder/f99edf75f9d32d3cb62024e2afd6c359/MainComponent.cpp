#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.


    // Initialize chopButtons
    for (int i = 0; i < 9; i++)
    {
        auto* button = new juce::TextButton("Chop " + juce::String(i + 1));
        button->onClick = [this] { transport.setPosition(10); };
        addAndMakeVisible(button);
        chopButtons.add(button);
    }

    // Initialize loadSampleButton
    loadSampleButton = new juce::TextButton("Load Sample");
    loadSampleButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff797fed));
    loadSampleButton->onClick = [this] { loadFile();  };
    addAndMakeVisible(loadSampleButton);

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

    // Initialize Transport State
    state = Stopped;

    // allow for .wav and .aif files in sample loading process
    formatManager.registerBasicFormats();

    setSize(800, 500);
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
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.fillAll();
    g.setColour(juce::Colours::black);
    g.setFont(juce::FontOptions(30.0f));
    g.drawFittedText("GOATCHOPS", getLocalBounds(), juce::Justification::centred, 1);
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

        // Read the file format
        juce::AudioFormatReader* formatReader = formatManager.createReaderFor(selectedFile);

        // Prepare file for playback
        std::unique_ptr<juce::AudioFormatReaderSource> tempSource(new juce::AudioFormatReaderSource(formatReader, true));

        // Set the transport's source to data from tempSource
        transport.setSource(tempSource.get());

        // Pass prepped file to playSource
        playSource.reset(tempSource.release());

        // Log filetype in console
        DBG(formatReader->getFormatName());
        DBG(transport.getLengthInSeconds());

        // DEV NOTE 6/12/2025:
        // This is where I left off. Next, move on to lesson 37 by The Audio Programmer
    }

    // Instantiate new file chooser
    fileChooser.reset(new juce::FileChooser("Select a sample...", juce::File(), "*.wav;*.mp3"));
    loadSampleButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff77ff77));

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

void MainComponent::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    // Create and set up flexbox
    juce::FlexBox fb;
    fb.flexWrap = juce::FlexBox::Wrap::wrap;
    fb.justifyContent = juce::FlexBox::JustifyContent::center;
    fb.alignContent = juce::FlexBox::AlignContent::center;

    // Add each chop button as a new flex item into flexbox
    for (auto* button : chopButtons)
    {
        fb.items.add(juce::FlexItem(*button).withMinWidth(50.0f).withMinHeight(50.0f));
    }

    fb.items.add(juce::FlexItem(*loadSampleButton).withMinWidth(100.0f).withMinHeight(50.0f));
    fb.items.add(juce::FlexItem(*playButton).withMinWidth(50.0f).withMinHeight(50.0f));
    fb.items.add(juce::FlexItem(*stopButton).withMinWidth(50.0f).withMinHeight(50.0f));

    // Perform the flexbox layout
    fb.performLayout(getLocalBounds());
}
