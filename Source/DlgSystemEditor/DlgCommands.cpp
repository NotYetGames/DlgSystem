// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgCommands.h"

//////////////////////////////////////////////////////////////////////////
// DlgEditorCommands
#define LOCTEXT_NAMESPACE "DialogueCommands"

void FDlgCommands::RegisterCommands()
{
	UI_COMMAND(
		DialogueReloadData,
		"Reload Data",
		"Reloads the Dialogue data from the .dlg text file with the same name in the folder",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		ToggleShowPrimarySecondaryEdges,
		"Show primary/secondary edges",
		"Toggles the viewing of the primary/secondary edges.",
		EUserInterfaceActionType::ToggleButton, FInputChord()
	);

	UI_COMMAND(
		ToggleDrawPrimaryEdges,
		"Draws the primary edges",
		"Toggles the drawing of the primary edges",
		EUserInterfaceActionType::ToggleButton, FInputChord()
	);

	UI_COMMAND(
		ToggleDrawSecondaryEdges,
		"Draws the secondary edges",
		"Toggles the drawing of the secondary edges",
		EUserInterfaceActionType::ToggleButton, FInputChord()
	);

	UI_COMMAND(
		ConvertSpeechSequenceNodeToSpeechNodes,
		"Convert to speech nodes",
		"Converts/breaks the speech sequence node to a list of speech node.",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		ConvertSpeechNodesToSpeechSequence,
		"Converts selected Speech node(s) to a Speech Sequence Node",
		"Converts selected (compresses) linear Speech node(s) to a Speech Sequence Node",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		SaveAllDialogues,
		"Save All Dialogues...",
		"Saves all dialogues to the disk",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		DeleteAllDialoguesTextFiles,
		"Delete All Dialogues Text Files...",
		"Delete all dialogues text files on the disk from all existing known text formats and from the Settings AdditionalTextFormatFileExtensionsToLookFor",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		DeleteCurrentDialogueTextFiles,
		"Delete Current Dialogue Text Files...",
		"Delete all text files of the CURRENT Dialogue on the disk from all existing known text formats and from the Settings AdditionalTextFormatFileExtensionsToLookFor",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		OpenNotYetPlugins,
		"Not Yet: Plugins...",
		"Navigates to the Not Yet Plugins page",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		OpenMarketplace,
		"Marketplace...",
		"Navigates to the Dialogue Unreal Marketplace page",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		OpenWiki,
		"Wiki/Documentation...",
		"Navigates to the Dialogue System Wiki/Documentation page",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		OpenDiscord,
		"Discord...",
		"Navigates to the Dialogue System Discord server",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		OpenForum,
		"Forum...",
		"Navigates to the Dialogue System Forum",
		EUserInterfaceActionType::Button, FInputChord()
	);

	UI_COMMAND(
		FindInAllDialogues,
		"Find in All Dialogues",
		"Find references to descriptions, events, condition and variables in ALL Dialogue",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::F)
	);

	UI_COMMAND(
		FindInDialogue,
		"Find",
		"Find references to descriptions, events, condition and variables in the current Dialogue (use Ctrl+Shift+F to search in all Dialogues)",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::F)
	);

	UI_COMMAND(
		HideNodes,
		"HideNodes",
		"Hide selected nodes",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::H)
	);

	UI_COMMAND(
		UnHideAllNodes,
		"UnHideAllNodes",
		"UnHide all nodes",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::H)
	);
}

#undef LOCTEXT_NAMESPACE
