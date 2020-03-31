// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "Nodes/DlgNode_Selector.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgContextInternal.h"
#include "Logging/DlgLogger.h"


bool UDlgNode_Selector::HandleNodeEnter(UDlgContext* Context, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	check(Context != nullptr);
	FireNodeEnterEvents(Context);

	if (NodesEnteredWithThisStep.Contains(this))
	{
		FDlgLogger::Get().Warning(
			TEXT("Failed to enter selector node: it was entered multiple times in a single step."
					"Theoretically with some condition magic it could make sense, but chances are that it is an endless loop,"
					"thus entering the same selector twice with a single step is not supported. Dialogue is terminated!")
		);

		return false;
	}
	NodesEnteredWithThisStep.Add(this);

	switch (SelectorType)
	{
	case EDlgNodeSelectorType::First:
		{
			// Find first child with satisfies conditions
			for (const FDlgEdge& Edge : Children)
				if (Edge.Evaluate(Context, {this}))
					return Context->EnterNode(Edge.TargetIndex, NodesEnteredWithThisStep);

			break;
		}

	case EDlgNodeSelectorType::Random:
		{
			// Build the list of all valid children
			TArray<int32> Candidates;
			for (int32 EdgeIndex = 0; EdgeIndex < Children.Num(); ++EdgeIndex)
				if (Children[EdgeIndex].Evaluate(Context, { this }))
					Candidates.Add(EdgeIndex);

			// No candidates :(
			if (Candidates.Num() == 0)
				break;

			// Select Random
			const int32 SelectedIndex = FMath::RandHelper(Candidates.Num());
			const int32 TargetNodeIndex = Children[Candidates[SelectedIndex]].TargetIndex;
			return Context->EnterNode(TargetNodeIndex, NodesEnteredWithThisStep);
		}
	default:
		checkNoEntry();
	}

	FDlgLogger::Get().Warningf(TEXT("Dialogue = %s got stuck: selector node entered, no satisfied child!"), *Context->GetDialoguePathName());
	return false;
}
