// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "DlgSystem/DlgManager.h"
#include "DlgSystem/TreeViewHelpers/DlgTreeViewVariableProperties.h"

class UDialogueGraphNode;
class UDialogueGraphNode_Edge;
class UDlgDialogue;
class FDlgBrowserTreeVariableProperties;

class FDlgBrowserTreeVariableProperties : public FDlgTreeViewVariableProperties
{
	typedef FDlgBrowserTreeVariableProperties Self;
	typedef FDlgTreeViewVariableProperties Super;

public:
	FDlgBrowserTreeVariableProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>& InDialogues);

	// Dialogues:
	void AddDialogue(TWeakObjectPtr<const UDlgDialogue> Dialogue) override;

	// GraphNodes:
	bool HasGraphNodeSet(const FGuid& DialogueGUID) { return GraphNodes.Find(DialogueGUID) != nullptr; }
	TSet<TWeakObjectPtr<const UDialogueGraphNode>>* GetMutableGraphNodeSet(const FGuid& DialogueGUID)
	{
		return GraphNodes.Find(DialogueGUID);
	}
	const TSet<TWeakObjectPtr<const UDialogueGraphNode>>& GetGraphNodeSet(const FGuid& DialogueGUID) const
	{
		auto* SetPtr = GraphNodes.Find(DialogueGUID);
		check(SetPtr);
		return *SetPtr;
	}

	// EdgeNodes:
	bool HasEdgeNodeSet(const FGuid& DialogueGUID) { return EdgeNodes.Find(DialogueGUID) != nullptr; }
	TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>* GetMutableEdgeNodeSet(const FGuid& DialogueGUID)
	{
		return EdgeNodes.Find(DialogueGUID);
	}
	const TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>& GetEdgeNodeSet(const FGuid& DialogueGUID) const
	{
		auto* SetPtr = EdgeNodes.Find(DialogueGUID);
		check(SetPtr);
		return *SetPtr;
	}

protected:
	/**
	 * All the nodes that contain this variable property
	 * Key: The unique identifier for the Dialogue
	 * Value: All nodes in the Dialogue that contain this variable name.
	 */
	TMap<FGuid, TSet<TWeakObjectPtr<const UDialogueGraphNode>>> GraphNodes;

	/**
	 * All the edge nodes that contain this variable property
	 * Key: The unique identifier for the Dialogue
	 * Value: All edge in the Dialogue that contain this condition.
	 */
	TMap<FGuid, TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>> EdgeNodes;
};
