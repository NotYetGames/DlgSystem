// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "VariableProperties_DialogueTree.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueTreeVariableProperties
FVariableProperties_DialogueTree::FVariableProperties_DialogueTree(const TSet<TWeakObjectPtr<UDlgDialogue>>& InDialogues) : Dialogues(InDialogues)
{
	// Empty initialize the graph nodes
	for (const TWeakObjectPtr<UDlgDialogue>& Dialogue : InDialogues)
	{
		GraphNodes.Add(Dialogue->GetDlgGuid(), {});
	}

	// Empty initialize the edge nodes
	for (const TWeakObjectPtr<UDlgDialogue>& Dialogue : InDialogues)
	{
		EdgeNodes.Add(Dialogue->GetDlgGuid(), {});
	}
}

void FVariableProperties_DialogueTree::AddDialogue(TWeakObjectPtr<UDlgDialogue> Dialogue)
{
	Dialogues.Add(Dialogue);

	// Initialize the graph nodes
	{
		const FGuid Id = Dialogue->GetDlgGuid();
		auto* SetPtr = GraphNodes.Find(Id);
		if (SetPtr == nullptr)
		{
			// Does not exist, empty initialize.
			GraphNodes.Add(Id, {});
		}
	}

	// Initialize the edge nodes
	{

		const FGuid Id = Dialogue->GetDlgGuid();
		auto* SetPtr = EdgeNodes.Find(Id);
		if (SetPtr == nullptr)
		{
			// Does not exist, empty initialize.
			EdgeNodes.Add(Id, {});
		}
	}
}
