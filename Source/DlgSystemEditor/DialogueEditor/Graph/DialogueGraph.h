// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"

#include "DlgSystem/DlgDialogue.h"

#include "DialogueGraph.generated.h"

class UDlgNode;
class UDialogueGraphNode_Base;
class UDialogueGraphNode;
class UDialogueGraphNode_Root;
class UDialogueGraphNode_Edge;
class UDialogueGraphSchema;

UCLASS()
class UDialogueGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	// Begin UObject Interface.
	/**
	 * Note that the object will be modified. If we are currently recording into the
	 * transaction buffer (undo/redo), save a copy of this object into the buffer and
	 * marks the package as needing to be saved.
	 *
	 * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
	 *								currently recording an active undo/redo transaction
	 * @return true if the object was saved to the transaction buffer
	 */
	bool Modify(bool bAlwaysMarkDirty = true) override;

	// Begin UEdGraph
	/** Remove a node from this graph. Variant of UEdGraph::RemoveNode */
	bool RemoveGraphNode(UEdGraphNode* NodeToRemove);

	// Begin Own methods
	/** Gets the DlgDialogue that contains this graph */
	UDlgDialogue* GetDialogue() const
	{
		// Unreal engine magic, get the object that owns this graph, that is our Dialogue.
		return CastChecked<UDlgDialogue>(GetOuter());
	}

	/** Gets the root graph node of this graph */
	TArray<UDialogueGraphNode_Root*> GetRootGraphNodes() const;

	/** Gets all the graph nodes of this  Graph */
	const TArray<UEdGraphNode*>& GetAllGraphNodes() const { return Nodes;  }

	/** Gets the all the dialogue graph nodes (that inherit from UDialogueGraphNode_Base). Includes Root node. */
	TArray<UDialogueGraphNode_Base*> GetAllBaseDialogueGraphNodes() const;

	/** Gets the all the dialogue graph nodes (that inherit from UDialogueGraphNode). Includes Root node. */
	TArray<UDialogueGraphNode*> GetAllDialogueGraphNodes() const;

	/** Gets the all the dialogue graph nodes (that inherit from UDialogueGraphNode_Edge). */
	TArray<UDialogueGraphNode_Edge*> GetAllEdgeDialogueGraphNodes() const;

	/** Creates the graph nodes from the Dialogue that contains this graph */
	void CreateGraphNodesFromDialogue();

	/** Creates all the links between the graph nodes from the Dialogue nodes */
	void LinkGraphNodesFromDialogue() const;

	/** Automatically reposition all the nodes in the graph. */
	void AutoPositionGraphNodes() const;

	/** Remove all nodes from the graph. Without notifying anyone. This operation is atomic to the graph */
	void RemoveAllNodes();

	/** Helper method to get directly the Dialogue Graph Schema */
	const UDialogueGraphSchema* GetDialogueGraphSchema() const;

private:
	UDialogueGraph(const FObjectInitializer& ObjectInitializer);

	// Link the specified node to all it's children
	void LinkGraphNodeToChildren(
		const TArray<UDlgNode*>& NodesDialogue,
		const UDlgNode& NodeDialogue,
		UDialogueGraphNode* NodeGraph
	) const;
};
