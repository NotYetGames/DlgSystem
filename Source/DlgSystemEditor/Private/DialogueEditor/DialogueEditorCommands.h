// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "Framework/Commands/Commands.h"

#include "DialogueStyle.h"

// Add menu commands and stuff, if you want to that is
class FDialogueEditorCommands : public TCommands<FDialogueEditorCommands>
{
public:
	FDialogueEditorCommands()
		: TCommands<FDialogueEditorCommands>(
			TEXT("DlgSystemEditor"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "DlgSystemEditor", "DlgSystem Editor"), // Localized context name for displaying
			NAME_None, // Parent
			FDialogueStyle::Get()->GetStyleSetName() // Icon Style Set
			)
	{
	}

	// TCommand<> interface
	void RegisterCommands() override;
	// End of TCommand<> interface

	/** Reloads the dialogue data from the .dlg text file that match the name of this dialogue. */
	TSharedPtr<FUICommandInfo> DialogueReloadData;

	/** Shows the primary/secondary edges. */
	TSharedPtr<FUICommandInfo> ToggleShowPrimarySecondaryEdges;

	/** Draw the primary edges */
	TSharedPtr<FUICommandInfo> ToggleDrawPrimaryEdges;

	/** Draw the secondary edges */
	TSharedPtr<FUICommandInfo> ToggleDrawSecondaryEdges;

	/** Converts a speech sequence node to a list of speech node. */
	TSharedPtr<FUICommandInfo> ConvertSpeechSequenceNodeToSpeechNodes;

	/** Saves all the dialogues. */
	TSharedPtr<FUICommandInfo> SaveAllDialogues;

	/** Removes all the dialogue text files */
	TSharedPtr<FUICommandInfo> DeleteAllDialoguesTextFiles;

	/** Open find in ALL Dialogues search window */
	TSharedPtr<FUICommandInfo> FindInAllDialogues;

	/** Open find in current Dialogue tab. */
	TSharedPtr<FUICommandInfo> FindInDialogue;

	/** Hide Selected Node */
	TSharedPtr<FUICommandInfo> HideNodes;

	/** UnHide all nodes */
	TSharedPtr<FUICommandInfo> UnHideAllNodes;
};
