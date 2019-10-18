// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "IDlgDialogueEditorAccess.h"
#include "DialogueEditor/DialogueEditorUtilities.h"

/**
 * Implementation of the interface for dialogue graph interaction between DlgSystem module <-> DlgSystemEditor module.
 * Set in UDialogueGraph constructor for Each Dialogue
 */
class DLGSYSTEMEDITOR_API FDlgDialogueEditorAccess : public IDlgDialogueEditorAccess
{
public:
	FDlgDialogueEditorAccess() {}
	~FDlgDialogueEditorAccess() {}

	void UpdateGraphNodeEdges(UEdGraphNode* GraphNode);
	UEdGraph* CreateNewDialogueGraph(UDlgDialogue* Dialogue) const override;
	void CompileDialogueNodesFromGraphNodes(UDlgDialogue* Dialogue) const override;
	void RemoveAllGraphNodes(UDlgDialogue* Dialogue) const override;
	void UpdateDialogueToVersion_UseOnlyOneOutputAndInputPin(UDlgDialogue* Dialogue) const override;

	bool AreDialogueNodesInSyncWithGraphNodes(UDlgDialogue* Dialogue) const override
	{
		return FDialogueEditorUtilities::AreDialogueNodesInSyncWithGraphNodes(Dialogue);
	}
};
