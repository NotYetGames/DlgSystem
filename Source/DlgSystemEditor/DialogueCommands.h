// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Framework/Commands/Commands.h"

#include "DialogueStyle.h"

// Add menu commands and stuff, if you want to that is
class FDialogueCommands : public TCommands<FDialogueCommands>
{
public:
	FDialogueCommands()
		: TCommands<FDialogueCommands>(
			TEXT("DlgSystemEditor"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "DlgSystemEditor", "DlgSystem Editor"), // Localized context name for displaying
			NAME_None, // Parent
			FDialogueStyle::Get()->GetStyleSetName() // Icon Style Set
			)
	{
	}

	//
	// TCommand<> interface
	//
	void RegisterCommands() override;

public:
	// Reloads the dialogue data from the .dlg text file that match the name of this dialogue
	TSharedPtr<FUICommandInfo> DialogueReloadData;

	// Shows the primary/secondary edges
	TSharedPtr<FUICommandInfo> ToggleShowPrimarySecondaryEdges;

	// Draw the primary edges
	TSharedPtr<FUICommandInfo> ToggleDrawPrimaryEdges;

	// Draw the secondary edges
	TSharedPtr<FUICommandInfo> ToggleDrawSecondaryEdges;

	// Converts a speech sequence node to a list of speech node
	TSharedPtr<FUICommandInfo> ConvertSpeechSequenceNodeToSpeechNodes;

	// Converts  a list of speech nodes a speech sequence node
	TSharedPtr<FUICommandInfo> ConvertSpeechNodesToSpeechSequence;

	// Saves all the dialogues
	TSharedPtr<FUICommandInfo> SaveAllDialogues;

	// Delete all the dialogues text files
	TSharedPtr<FUICommandInfo> DeleteAllDialoguesTextFiles;

	// Delete all the text files for the CURRENT Dialogue
	TSharedPtr<FUICommandInfo> DeleteCurrentDialogueTextFiles;

	// External links
	TSharedPtr<FUICommandInfo> OpenNotYetPlugins;
	TSharedPtr<FUICommandInfo> OpenMarketplace;
	TSharedPtr<FUICommandInfo> OpenWiki;
	TSharedPtr<FUICommandInfo> OpenDiscord;
	TSharedPtr<FUICommandInfo> OpenForum;

	// Open find in ALL Dialogues search window
	TSharedPtr<FUICommandInfo> FindInAllDialogues;

	// Open find in current Dialogue tab
	TSharedPtr<FUICommandInfo> FindInDialogue;

	// Hide Selected Node
	TSharedPtr<FUICommandInfo> HideNodes;

	// UnHide all nodes
	TSharedPtr<FUICommandInfo> UnHideAllNodes;
};
