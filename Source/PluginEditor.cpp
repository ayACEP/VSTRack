/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GraphEditorPanel.h"
#include "InternalFilters.h"


//==============================================================================
VstrackAudioProcessorEditor::VstrackAudioProcessorEditor (VstrackAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
	
	midiVolume.setSliderStyle(Slider::LinearBarVertical);
	midiVolume.setRange(0.0, 127.0, 1.0);
	midiVolume.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
	midiVolume.setPopupDisplayEnabled(true, this);
	midiVolume.setTextValueSuffix(" Volume");
	midiVolume.setValue(1.0);

	//addAndMakeVisible(&midiVolume);

	PropertiesFile::Options options;
	options.applicationName = "Juce Audio Plugin Host";
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Preferences";

	appProperties = new ApplicationProperties();
	appProperties->setStorageParameters(options);

	ScopedPointer<XmlElement> savedAudioState(appProperties->getUserSettings()
		->getXmlValue("audioDeviceState"));

	deviceManager.initialise(256, 256, savedAudioState, true);
	formatManager.addDefaultFormats();
	formatManager.addFormat(new InternalPluginFormat());

	auto mainComponent = new GraphDocumentComponent(formatManager, &deviceManager);
	mainComponent->setBounds(0, 0, 400, 300);
	addAndMakeVisible(mainComponent);

}

VstrackAudioProcessorEditor::~VstrackAudioProcessorEditor()
{
	
	appProperties = nullptr;
}

//==============================================================================
void VstrackAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void VstrackAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	midiVolume.setBounds(40, 30, 20, getHeight() - 60);
}
