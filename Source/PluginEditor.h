/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "GraphEditorPanel.h"
#include "FilterGraph.h"

namespace CommandIDs
{
	static const int open = 0x30000;
	static const int save = 0x30001;
	static const int saveAs = 0x30002;
	static const int newFile = 0x30003;
	static const int showPluginListEditor = 0x30100;
	static const int showAudioSettings = 0x30200;
	static const int aboutBox = 0x30300;
	static const int allWindowsForward = 0x30400;
	static const int toggleDoublePrecision = 0x30500;
}

class MenuBar;
class PluginListWindow;

//==============================================================================
/**
*/
class VstrackAudioProcessorEditor : public AudioProcessorEditor, public ChangeListener, public ApplicationCommandTarget
{
public:
	VstrackAudioProcessorEditor(VstrackAudioProcessor&);
	~VstrackAudioProcessorEditor();

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	// override ChangeListener
	void changeListenerCallback(ChangeBroadcaster* source) override;

	// override ApplicationCommandTarget
	ApplicationCommandTarget* getNextCommandTarget();
	void getAllCommands(Array<CommandID>& commands);
	void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result);
	bool perform(const InvocationInfo& info);

	// copy from MainHostWindow.cpp
	GraphDocumentComponent* getGraphEditor();
	const PluginDescription* getChosenType(const int menuID) const;
	
	void createPlugin(const PluginDescription* desc, int x, int y);

	void addPluginsToMenu(PopupMenu&);

	void updatePrecisionMenuItem(ApplicationCommandInfo& info);
	bool isDoublePrecisionProcessing();

	void showAudioSettings();

	// This reference is provided as a quick way for your editor to
	// access the processor object that created it.
	VstrackAudioProcessor& processor;

	AudioDeviceManager deviceManager;
	AudioPluginFormatManager formatManager;

	OwnedArray<PluginDescription> internalTypes;
	KnownPluginList::SortMethod pluginSortMethod;
	KnownPluginList knownPluginList;

	ScopedPointer<PluginListWindow> pluginListWindow;

	ApplicationCommandManager commandManager;
	ScopedPointer<ApplicationProperties> appProperties;

	ScopedPointer<GraphDocumentComponent> graphDocumentComponent;
	ScopedPointer<MenuBar> menuBar;

	Slider midiVolume;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VstrackAudioProcessorEditor)
};



class MenuBar : public MenuBarComponent, public MenuBarModel
{
public:
	MenuBar(VstrackAudioProcessorEditor *editor)
	{
		this->editor = editor;
		setModel(this);
	};

	~MenuBar()
	{
		setModel(nullptr);
	}

	// override MenuBarModel
	StringArray getMenuBarNames() override;
	PopupMenu getMenuForIndex(int topLevelMenuIndex, const String& menuName) override;
	void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

	VstrackAudioProcessorEditor *editor;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MenuBar)
};



class PluginListWindow: public DocumentWindow
{
public:
	PluginListWindow(VstrackAudioProcessorEditor& owner_, AudioPluginFormatManager& pluginFormatManager)
		: DocumentWindow("Available Plugins",
			LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
			DocumentWindow::minimiseButton | DocumentWindow::closeButton),
		owner(owner_)
	{
		const File deadMansPedalFile(owner.appProperties->getUserSettings()
			->getFile().getSiblingFile("RecentlyCrashedPluginsList"));

		setContentOwned(new PluginListComponent(pluginFormatManager,
			owner.knownPluginList,
			deadMansPedalFile,
			owner.appProperties->getUserSettings(), true), true);

		setResizable(true, false);
		setResizeLimits(300, 400, 800, 1500);
		setTopLeftPosition(60, 60);

		restoreWindowStateFromString(owner.appProperties->getUserSettings()->getValue("listWindowPos"));
		setVisible(true);
	}

	~PluginListWindow()
	{
		owner.appProperties->getUserSettings()->setValue("listWindowPos", getWindowStateAsString());

		clearContentComponent();
	}

	void closeButtonPressed()
	{
		owner.pluginListWindow = nullptr;
	}

private:
	VstrackAudioProcessorEditor& owner;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListWindow)
};
