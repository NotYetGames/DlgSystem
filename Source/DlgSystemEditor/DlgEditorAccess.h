// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DlgSystem/IDlgEditorAccess.h"
#include "DlgSystemEditor/DlgEditorUtilities.h"

/**
 * Implementation of the interface for dialogue graph interaction between DlgSystem module <-> DlgSystemEditor module.
 * Set in UDialogueGraph constructor for Each Dialogue
 */
class DLGSYSTEMEDITOR_API FDlgEditorAccess : public IDlgEditorAccess
{
public:
	FDlgEditorAccess() {}
	~FDlgEditorAccess() {}

	void UpdateGraphNodeEdges(UEdGraphNode* GraphNode) override;
	UEdGraph* CreateNewDialogueGraph(UDlgDialogue* Dialogue) const override;
	void CompileDialogueNodesFromGraphNodes(UDlgDialogue* Dialogue) const override;
	void RemoveAllGraphNodes(UDlgDialogue* Dialogue) const override;
	void UpdateDialogueToVersion_UseOnlyOneOutputAndInputPin(UDlgDialogue* Dialogue) const override;
	void SetNewOuterForObjectFromGraphNode(UObject* Object, UEdGraphNode* GraphNode) const override;

	bool AreDialogueNodesInSyncWithGraphNodes(UDlgDialogue* Dialogue) const override
	{
		return FDlgEditorUtilities::AreDialogueNodesInSyncWithGraphNodes(Dialogue);
	}
};
