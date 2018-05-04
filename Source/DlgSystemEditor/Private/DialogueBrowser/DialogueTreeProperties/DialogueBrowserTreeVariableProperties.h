// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "DlgManager.h"
#include "TreeViewHelpers/DlgTreeViewVariableProperties.h"

class UDialogueGraphNode;
class UDialogueGraphNode_Edge;
class UDlgDialogue;
class FDialogueBrowserTreeVariableProperties;

class FDialogueBrowserTreeVariableProperties : public FDlgTreeViewVariableProperties
{
	typedef FDialogueBrowserTreeVariableProperties Self;
	typedef FDlgTreeViewVariableProperties Super;

public:
	FDialogueBrowserTreeVariableProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>& InDialogues);

	// Dialogues:
	void AddDialogue(TWeakObjectPtr<const UDlgDialogue> Dialogue) override;

	// GraphNodes:
	bool HasGraphNodeSet(const FGuid& DialogueGuid) { return GraphNodes.Find(DialogueGuid) != nullptr; }
	TSet<TWeakObjectPtr<const UDialogueGraphNode>>* GetMutableGraphNodeSet(const FGuid& DialogueGuid)
	{
		return GraphNodes.Find(DialogueGuid);
	}
	const TSet<TWeakObjectPtr<const UDialogueGraphNode>>& GetGraphNodeSet(const FGuid& DialogueGuid) const
	{
		auto* SetPtr = GraphNodes.Find(DialogueGuid);
		check(SetPtr);
		return *SetPtr;
	}

	// EdgeNodes:
	bool HasEdgeNodeSet(const FGuid& DialogueGuid) { return EdgeNodes.Find(DialogueGuid) != nullptr; }
	TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>* GetMutableEdgeNodeSet(const FGuid& DialogueGuid)
	{
		return EdgeNodes.Find(DialogueGuid);
	}
	const TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>& GetEdgeNodeSet(const FGuid& DialogueGuid) const
	{
		auto* SetPtr = EdgeNodes.Find(DialogueGuid);
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
