/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GraphEditorPanel.h"
#include "InternalFilters.h"

//==============================================================================
VstrackAudioProcessorEditor::VstrackAudioProcessorEditor (VstrackAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
	PropertiesFile::Options options;
	options.applicationName = "Juce Audio Plugin Host";
	options.filenameSuffix = "settings";
	options.osxLibrarySubFolder = "Preferences";

	appProperties = new ApplicationProperties();
	appProperties->setStorageParameters(options);
	
	formatManager.addDefaultFormats();
	formatManager.addFormat(new InternalPluginFormat());

	ScopedPointer<XmlElement> savedAudioState(appProperties->getUserSettings()
		->getXmlValue("audioDeviceState"));

	deviceManager.initialise(256, 256, savedAudioState, true);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);

	graphDocumentComponent = new GraphDocumentComponent(formatManager, &deviceManager, p.getGraph());
	graphDocumentComponent->setBounds(0, 0, getWidth(), getHeight());
	addAndMakeVisible(graphDocumentComponent);

	InternalPluginFormat internalFormat;
	internalFormat.getAllTypes(internalTypes);

	ScopedPointer<XmlElement> savedPluginList(appProperties->getUserSettings()->getXmlValue("pluginList"));

	if (savedPluginList != nullptr)
		knownPluginList.recreateFromXml(*savedPluginList);

	pluginSortMethod = (KnownPluginList::SortMethod) appProperties->getUserSettings()
		->getIntValue("pluginSortMethod", KnownPluginList::sortByManufacturer);

	knownPluginList.addChangeListener(this);

	if (auto* filterGraph = getGraphEditor()->graph.get())
		filterGraph->addChangeListener(this);

	commandManager.registerAllCommandsForTarget(this);

	addKeyListener(commandManager.getKeyMappings());

	menuBar = new MenuBar(this);
	menuBar->setBounds(0, 0, getWidth(), getLookAndFeel().getDefaultMenuBarHeight());
	addAndMakeVisible(menuBar);// menu bar
	menuBar->menuItemsChanged();

	commandManager.setFirstCommandTarget(this);
}

VstrackAudioProcessorEditor::~VstrackAudioProcessorEditor()
{
	knownPluginList.removeChangeListener(this);

	if (auto* filterGraph = getGraphEditor()->graph.get())
		filterGraph->removeChangeListener(this);

	appProperties = nullptr;
	pluginListWindow = nullptr;
	graphDocumentComponent = nullptr;
	menuBar = nullptr;

	//appProperties->getUserSettings()->setValue("mainWindowPos", getWindowStateAsString());

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
	//midiVolume.setBounds(40, 30, 20, getHeight() - 60);
}

StringArray MenuBar::getMenuBarNames()
{
	return { "File", "Plugins", "Options", "Windows" };
}

PopupMenu MenuBar::getMenuForIndex(int topLevelMenuIndex, const String& menuName)
{
	PopupMenu menu;

	if (topLevelMenuIndex == 0)
	{
		// "File" menu
		menu.addCommandItem(&editor->commandManager, CommandIDs::newFile);
		menu.addCommandItem(&editor->commandManager, CommandIDs::open);

		RecentlyOpenedFilesList recentFiles;
		recentFiles.restoreFromString(editor->appProperties->getUserSettings()
			->getValue("recentFilterGraphFiles"));

		PopupMenu recentFilesMenu;
		recentFiles.createPopupMenuItems(recentFilesMenu, 100, true, true);
		menu.addSubMenu("Open recent file", recentFilesMenu);

		menu.addCommandItem(&editor->commandManager, CommandIDs::save);
		menu.addCommandItem(&editor->commandManager, CommandIDs::saveAs);
		menu.addSeparator();
		menu.addCommandItem(&editor->commandManager, StandardApplicationCommandIDs::quit);
	}
	else if (topLevelMenuIndex == 1)
	{
		// "Plugins" menu
		PopupMenu pluginsMenu;
		//addPluginsToMenu(pluginsMenu);
		menu.addSubMenu("Create plugin", pluginsMenu);
		menu.addSeparator();
		menu.addItem(250, "Delete all plugins");
	}
	else if (topLevelMenuIndex == 2)
	{
		// "Options" menu

		menu.addCommandItem(&editor->commandManager, CommandIDs::showPluginListEditor);

		PopupMenu sortTypeMenu;
		sortTypeMenu.addItem(200, "List plugins in default order", true, editor->pluginSortMethod == KnownPluginList::defaultOrder);
		sortTypeMenu.addItem(201, "List plugins in alphabetical order", true, editor->pluginSortMethod == KnownPluginList::sortAlphabetically);
		sortTypeMenu.addItem(202, "List plugins by category", true, editor->pluginSortMethod == KnownPluginList::sortByCategory);
		sortTypeMenu.addItem(203, "List plugins by manufacturer", true, editor->pluginSortMethod == KnownPluginList::sortByManufacturer);
		sortTypeMenu.addItem(204, "List plugins based on the directory structure", true, editor->pluginSortMethod == KnownPluginList::sortByFileSystemLocation);
		menu.addSubMenu("Plugin menu type", sortTypeMenu);

		menu.addSeparator();
		menu.addCommandItem(&editor->commandManager, CommandIDs::showAudioSettings);
		menu.addCommandItem(&editor->commandManager, CommandIDs::toggleDoublePrecision);

		menu.addSeparator();
		menu.addCommandItem(&editor->commandManager, CommandIDs::aboutBox);
	}
	else if (topLevelMenuIndex == 3)
	{
		menu.addCommandItem(&editor->commandManager, CommandIDs::allWindowsForward);
	}

	return menu;
}

void MenuBar::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
	if (menuItemID == 250)
	{
		if (auto* graphEditor = editor->getGraphEditor())
			if (auto* filterGraph = graphEditor->graph.get())
				filterGraph->clear();
	}
	else if (menuItemID >= 100 && menuItemID < 200)
	{
		RecentlyOpenedFilesList recentFiles;
		recentFiles.restoreFromString(editor->appProperties->getUserSettings()
			->getValue("recentFilterGraphFiles"));

		if (auto* graphEditor = editor->getGraphEditor())
			if (graphEditor->graph != nullptr && graphEditor->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
				graphEditor->graph->loadFrom(recentFiles.getFile(menuItemID - 100), true);
	}
	else if (menuItemID >= 200 && menuItemID < 210)
	{
		if (menuItemID == 200)     editor->pluginSortMethod = KnownPluginList::defaultOrder;
		else if (menuItemID == 201)     editor->pluginSortMethod = KnownPluginList::sortAlphabetically;
		else if (menuItemID == 202)     editor->pluginSortMethod = KnownPluginList::sortByCategory;
		else if (menuItemID == 203)     editor->pluginSortMethod = KnownPluginList::sortByManufacturer;
		else if (menuItemID == 204)     editor->pluginSortMethod = KnownPluginList::sortByFileSystemLocation;

		editor->appProperties->getUserSettings()->setValue("pluginSortMethod", (int)editor->pluginSortMethod);

		menuItemsChanged();
	}
	else
	{
		editor->createPlugin(editor->getChosenType(menuItemID),
			proportionOfWidth(0.3f + Random::getSystemRandom().nextFloat() * 0.6f),
			proportionOfHeight(0.3f + Random::getSystemRandom().nextFloat() * 0.6f));
	}
}

GraphDocumentComponent* VstrackAudioProcessorEditor::getGraphEditor()
{
	return graphDocumentComponent;
}

const PluginDescription* VstrackAudioProcessorEditor::getChosenType(const int menuID) const
{
	if (menuID >= 1 && menuID < 1 + internalTypes.size())
		return internalTypes[menuID - 1];

	return knownPluginList.getType(knownPluginList.getIndexChosenByMenu(menuID));
}

void VstrackAudioProcessorEditor::createPlugin(const PluginDescription* desc, int x, int y)
{
	if (auto* graphEditor = getGraphEditor())
		graphEditor->createNewPlugin(desc, x, y);
}

void VstrackAudioProcessorEditor::changeListenerCallback(ChangeBroadcaster* changed)
{
	if (changed == &knownPluginList)
	{
		menuBar->menuItemsChanged();

		// save the plugin list every time it gets chnaged, so that if we're scanning
		// and it crashes, we've still saved the previous ones
		ScopedPointer<XmlElement> savedPluginList(knownPluginList.createXml());

		if (savedPluginList != nullptr)
		{
			appProperties->getUserSettings()->setValue("pluginList", savedPluginList);
			appProperties->saveIfNeeded();
		}
	}
	else if (changed == getGraphEditor()->graph)
	{
		String title = JUCEApplication::getInstance()->getApplicationName();

		File f = getGraphEditor()->graph->getFile();

		if (f.existsAsFile())
			title = f.getFileName() + " - " + title;

		setName(title);
	}
}

void VstrackAudioProcessorEditor::addPluginsToMenu(PopupMenu& m)
{
	if (auto* graphEditor = getGraphEditor())
	{
		int i = 0;

		for (auto* t : internalTypes)
			m.addItem(++i, t->name, graphEditor->graph->getNodeForName(t->name) == nullptr);
	}

	m.addSeparator();

	knownPluginList.addToMenu(m, pluginSortMethod);
}

ApplicationCommandTarget* VstrackAudioProcessorEditor::getNextCommandTarget()
{
	return findFirstTargetParentComponent();
}

void VstrackAudioProcessorEditor::getAllCommands(Array<CommandID>& commands)
{
	// this returns the set of all commands that this target can perform..
	const CommandID ids[] = { CommandIDs::newFile,
		CommandIDs::open,
		CommandIDs::save,
		CommandIDs::saveAs,
		CommandIDs::showPluginListEditor,
		CommandIDs::showAudioSettings,
		CommandIDs::toggleDoublePrecision,
		CommandIDs::aboutBox,
		CommandIDs::allWindowsForward
	};

	commands.addArray(ids, numElementsInArray(ids));
}

void VstrackAudioProcessorEditor::getCommandInfo(const CommandID commandID, ApplicationCommandInfo& result)
{
	const String category("General");

	switch (commandID)
	{
	case CommandIDs::newFile:
		result.setInfo("New", "Creates a new filter graph file", category, 0);
		result.defaultKeypresses.add(KeyPress('n', ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::open:
		result.setInfo("Open...", "Opens a filter graph file", category, 0);
		result.defaultKeypresses.add(KeyPress('o', ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::save:
		result.setInfo("Save", "Saves the current graph to a file", category, 0);
		result.defaultKeypresses.add(KeyPress('s', ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::saveAs:
		result.setInfo("Save As...",
			"Saves a copy of the current graph to a file",
			category, 0);
		result.defaultKeypresses.add(KeyPress('s', ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
		break;

	case CommandIDs::showPluginListEditor:
		result.setInfo("Edit the list of available plug-Ins...", String(), category, 0);
		result.addDefaultKeypress('p', ModifierKeys::commandModifier);
		break;

	case CommandIDs::showAudioSettings:
		result.setInfo("Change the audio device settings", String(), category, 0);
		result.addDefaultKeypress('a', ModifierKeys::commandModifier);
		break;

	case CommandIDs::toggleDoublePrecision:
		updatePrecisionMenuItem(result);
		break;

	case CommandIDs::aboutBox:
		result.setInfo("About...", String(), category, 0);
		break;

	case CommandIDs::allWindowsForward:
		result.setInfo("All Windows Forward", "Bring all plug-in windows forward", category, 0);
		result.addDefaultKeypress('w', ModifierKeys::commandModifier);
		break;

	default:
		break;
	}
}

void VstrackAudioProcessorEditor::updatePrecisionMenuItem(ApplicationCommandInfo& info)
{
	info.setInfo("Double floating point precision rendering", String(), "General", 0);
	info.setTicked(isDoublePrecisionProcessing());
}

bool VstrackAudioProcessorEditor::isDoublePrecisionProcessing()
{
	if (auto* props = appProperties->getUserSettings())
		return props->getBoolValue("doublePrecisionProcessing", false);

	return false;
}

bool VstrackAudioProcessorEditor::perform(const InvocationInfo& info)
{
	auto* graphEditor = getGraphEditor();

	switch (info.commandID)
	{
	case CommandIDs::newFile:
		if (graphEditor != nullptr && graphEditor->graph != nullptr && graphEditor->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
			graphEditor->graph->newDocument();
		break;

	case CommandIDs::open:
		if (graphEditor != nullptr && graphEditor->graph != nullptr && graphEditor->graph->saveIfNeededAndUserAgrees() == FileBasedDocument::savedOk)
			graphEditor->graph->loadFromUserSpecifiedFile(true);
		break;

	case CommandIDs::save:
		if (graphEditor != nullptr && graphEditor->graph != nullptr)
			graphEditor->graph->save(true, true);
		break;

	case CommandIDs::saveAs:
		if (graphEditor != nullptr && graphEditor->graph != nullptr)
			graphEditor->graph->saveAs(File(), true, true, true);
		break;

	case CommandIDs::showPluginListEditor:
		if (pluginListWindow == nullptr)
			pluginListWindow = new PluginListWindow(*this, formatManager);

		pluginListWindow->toFront(true);
		break;

	case CommandIDs::showAudioSettings:
		showAudioSettings();
		break;

	case CommandIDs::toggleDoublePrecision:
		if (auto* props = appProperties->getUserSettings())
		{
			bool newIsDoublePrecision = !isDoublePrecisionProcessing();
			props->setValue("doublePrecisionProcessing", var(newIsDoublePrecision));

			{
				ApplicationCommandInfo cmdInfo(info.commandID);
				updatePrecisionMenuItem(cmdInfo);
				menuBar->menuItemsChanged();
			}

			if (graphEditor != nullptr)
				graphEditor->setDoublePrecision(newIsDoublePrecision);
		}
		break;

	case CommandIDs::aboutBox:
		// TODO
		break;

	case CommandIDs::allWindowsForward:
	{
		auto& desktop = Desktop::getInstance();

		for (int i = 0; i < desktop.getNumComponents(); ++i)
			desktop.getComponent(i)->toBehind(this);

		break;
	}

	default:
		return false;
	}

	return true;
}

void VstrackAudioProcessorEditor::showAudioSettings()
{
	AudioDeviceSelectorComponent audioSettingsComp(deviceManager,
		0, 256,
		0, 256,
		true, true, true, false);

	audioSettingsComp.setSize(500, 450);

	DialogWindow::LaunchOptions o;
	o.content.setNonOwned(&audioSettingsComp);
	o.dialogTitle = "Audio Settings";
	o.componentToCentreAround = this;
	o.dialogBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
	o.escapeKeyTriggersCloseButton = true;
	o.useNativeTitleBar = false;
	o.resizable = false;

	o.runModal();

	ScopedPointer<XmlElement> audioState(deviceManager.createStateXml());

	appProperties->getUserSettings()->setValue("audioDeviceState", audioState);
	appProperties->getUserSettings()->saveIfNeeded();

	if (auto* graphEditor = getGraphEditor())
		if (graphEditor->graph != nullptr)
			graphEditor->graph->removeIllegalConnections();
}
