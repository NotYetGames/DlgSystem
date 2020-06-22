// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
class UEdGraph;
class UDlgDialogue;
class UEdGraphNode;
class UDlgNode;

/**
 * Interface for dialogue graph interaction with the DlgSystemEditor module.
 * See DlgDialogueEditorModule.h (in the DlgSystemEditor) for the implementation of this interface.
 */
class DLGSYSTEM_API IDlgDialogueEditorAccess
{
public:
	virtual ~IDlgDialogueEditorAccess() {}

	// Updates the graph node edges data to match the dialogue data
	virtual void UpdateGraphNodeEdges(UEdGraphNode* GraphNode) = 0;

	// Creates a new dialogue graph.
	virtual UEdGraph* CreateNewDialogueGraph(UDlgDialogue* Dialogue) const = 0;

	// Compiles the dialogue nodes from the graph nodes. Meaning it transforms the graph data -> (into) dialogue data.
	virtual void CompileDialogueNodesFromGraphNodes(UDlgDialogue* Dialogue) const = 0;

	// Removes all nodes from the graph.
	virtual void RemoveAllGraphNodes(UDlgDialogue* Dialogue) const = 0;

	// Tells us if the number of dialogue nodes matches with the number of graph nodes (corresponding to dialogues).
	virtual bool AreDialogueNodesInSyncWithGraphNodes(UDlgDialogue* Dialogue) const = 0;

	// Updates the Dialogue to match the version UseOnlyOneOutputAndInputPin
	virtual void UpdateDialogueToVersion_UseOnlyOneOutputAndInputPin(UDlgDialogue* Dialogue) const = 0;

	// Tries to set the new outer for Object to the closes UDlgNode from UEdGraphNode
	virtual void SetNewOuterForObjectFromGraphNode(UObject* Object, UEdGraphNode* GraphNode) const = 0;
};
#endif
