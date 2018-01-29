// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "DlgManager.h"

class UDialogueGraphNode;
class UDialogueGraphNode_Edge;
class UDlgDialogue;
class FVariableProperties_DialogueTree;

typedef TSharedPtr<FVariableProperties_DialogueTree> FDlgTreeVariablePropertiesPtr;

static bool PredicateSortDialogueWeakPtrAlphabeticallyAscending(const TWeakObjectPtr<UDlgDialogue>& First,
																const TWeakObjectPtr<UDlgDialogue>& Second)
{
	if (!First.IsValid())
	{
		return false;
	}
	if (!Second.IsValid())
	{
		return true;
	}

	return PredicateSortFNameAlphabeticallyAscending(First->GetFName(), Second->GetFName());
}

class FVariableProperties_DialogueTree
{
	typedef FVariableProperties_DialogueTree Self;

public:
	FVariableProperties_DialogueTree(const TSet<TWeakObjectPtr<UDlgDialogue>>& InDialogues);
	virtual ~FVariableProperties_DialogueTree() {}

	// Dialogues:
	void AddDialogue(TWeakObjectPtr<UDlgDialogue> Dialogue);
	const TSet<TWeakObjectPtr<UDlgDialogue>>& GetDialogues() const { return Dialogues; }

	// GraphNodes:
	bool HasGraphNodeSet(const FGuid& DialogueGuid) { return GraphNodes.Find(DialogueGuid) != nullptr; }
	TSet<TWeakObjectPtr<UDialogueGraphNode>>* GetMutableGraphNodeSet(const FGuid& DialogueGuid)
	{
		return GraphNodes.Find(DialogueGuid);
	}
	const TSet<TWeakObjectPtr<UDialogueGraphNode>>& GetGraphNodeSet(const FGuid& DialogueGuid) const
	{
		auto* SetPtr = GraphNodes.Find(DialogueGuid);
		check(SetPtr);
		return *SetPtr;
	}

	// EdgeNodes:
	bool HasEdgeNodeSet(const FGuid& DialogueGuid) { return EdgeNodes.Find(DialogueGuid) != nullptr; }
	TSet<TWeakObjectPtr<UDialogueGraphNode_Edge>>* GetMutableEdgeNodeSet(const FGuid& DialogueGuid)
	{
		return EdgeNodes.Find(DialogueGuid);
	}
	const TSet<TWeakObjectPtr<UDialogueGraphNode_Edge>>& GetEdgeNodeSet(const FGuid& DialogueGuid) const
	{
		auto* SetPtr = EdgeNodes.Find(DialogueGuid);
		check(SetPtr);
		return *SetPtr;
	}

	/** Sorts all the properties it can */
	void Sort()
	{
		Dialogues.Sort(PredicateSortDialogueWeakPtrAlphabeticallyAscending);
	}

protected:
	/** Dialogues that contain this variable property */
	TSet<TWeakObjectPtr<UDlgDialogue>> Dialogues;

	/**
	 * All the nodes that contain this variable property
	 * Key: The unique identifier for the Dialogue
	 * Value: All nodes in the Dialogue that contain this variable name.
	 */
	TMap<FGuid, TSet<TWeakObjectPtr<UDialogueGraphNode>>> GraphNodes;

	/**
	 * All the edge nodes that contain this variable property
	 * Key: The unique identifier for the Dialogue
	 * Value: All edge in the Dialogue that contain this condition.
	 */
	TMap<FGuid, TSet<TWeakObjectPtr<UDialogueGraphNode_Edge>>> EdgeNodes;
};
