// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgBrowserTreeVariableProperties.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueTreeVariableProperties
FDlgBrowserTreeVariableProperties::FDlgBrowserTreeVariableProperties(const TSet<TWeakObjectPtr<const UDlgDialogue>>& InDialogues)
	: Super(InDialogues)
{
	// Empty initialize the graph nodes
	for (TWeakObjectPtr<const UDlgDialogue> Dialogue: InDialogues)
	{
		GraphNodes.Add(Dialogue->GetGUID(), {});
	}

	// Empty initialize the edge nodes
	for (TWeakObjectPtr<const UDlgDialogue> Dialogue : InDialogues)
	{
		EdgeNodes.Add(Dialogue->GetGUID(), {});
	}
}

void FDlgBrowserTreeVariableProperties::AddDialogue(TWeakObjectPtr<const UDlgDialogue> Dialogue)
{
	Super::AddDialogue(Dialogue);

	// Initialize the graph nodes
	{
		const FGuid Id = Dialogue->GetGUID();
		auto* SetPtr = GraphNodes.Find(Id);
		if (SetPtr == nullptr)
		{
			// Does not exist, empty initialize.
			GraphNodes.Add(Id, {});
		}
	}

	// Initialize the edge nodes
	{

		const FGuid ID = Dialogue->GetGUID();
		auto* SetPtr = EdgeNodes.Find(ID);
		if (SetPtr == nullptr)
		{
			// Does not exist, empty initialize.
			EdgeNodes.Add(ID, {});
		}
	}
}
