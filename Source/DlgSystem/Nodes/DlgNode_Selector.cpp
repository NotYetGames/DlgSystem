// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgNode_Selector.h"

#include "DlgSystem/DlgContext.h"
#include "DlgSystem/Logging/DlgLogger.h"

const FText& UDlgNode_Selector::GetNodeText() const
{
	static const FText SelectFirstText = FText::FromString("First Satisfied");
	switch (SelectorType)
	{
		case EDlgNodeSelectorType::First:
		{
			return SelectFirstText;
		}

		case EDlgNodeSelectorType::Random:
		{
			FString SelectRandomString = TEXT("Random Satisfied");

			if (bCycleThroughSatisfiedOptionsWithoutRepetition)
			{
				SelectRandomString += TEXT("\nCycle options");
			}

			if (bAvoidPickingSameOptionTwiceInARow)
			{
				SelectRandomString += TEXT("\nAvoid repetition");
			}
			DynamicDisplayText = FText::FromString(SelectRandomString);
			return DynamicDisplayText;
		}

		default:
		{
			return FText::GetEmpty();
		}
	}
}

bool UDlgNode_Selector::HandleNodeEnter(UDlgContext& Context, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	FireNodeEnterEvents(Context);

	if (NodesEnteredWithThisStep.Contains(this))
	{
		FDlgLogger::Get().Errorf(
			TEXT("SelectorNode::HandleNodeEnter - Failed to enter selector node, it was entered multiple times in a single step."
					"Theoretically with some condition magic it could make sense, but chances are that it is an endless loop,"
					"thus entering the same selector twice with a single step is not supported. Dialogue is terminated.\nContext:\n\t%s"),
			*Context.GetContextString()
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
			{
				if (Edge.Evaluate(Context, { this }))
				{
					return Context.EnterNode(Edge.TargetIndex, NodesEnteredWithThisStep);
				}
			}
			break;
		}

		case EDlgNodeSelectorType::Random:
		{
			// Select Random
			const int32 ChildNodeIndex = GetRandomChildNodeIndex(Context);
			if (ChildNodeIndex != INDEX_NONE)
			{
				return Context.EnterNode(ChildNodeIndex, NodesEnteredWithThisStep);
			}
		}

		default:
			checkNoEntry();
	}

	FDlgLogger::Get().Errorf(
		TEXT("HandleNodeEnter - selector node entered, no satisfied child.\nContext:\n\t%s"),
		*Context.GetContextString()
	);
	return false;
}

int32 UDlgNode_Selector::GetRandomChildNodeIndex(UDlgContext& Context)
{
	FDlgNodeSavedData& SavedData = Context.GetNodeSavedData(NodeGUID);

	// The list of all valid children (ones with satisfied condition)
	TArray<int32> Candidates;

	// List of possible candidates if we want to avoid repetition based on the booleans
	TArray<int32> CandidatesLimited;

	for (int32 EdgeIndex = 0; EdgeIndex < Children.Num(); ++EdgeIndex)
	{
		if (Children[EdgeIndex].Evaluate(Context, { this }))
		{
			Candidates.Add(EdgeIndex);

			const FGuid ChildNodeGUID = Context.GetNodeGUIDForIndex(Children[EdgeIndex].TargetIndex);
			if (!SavedData.GUIDList.Contains(ChildNodeGUID))
			{
				CandidatesLimited.Add(EdgeIndex);
			}
		}
	}

	// No candidates :(
	if (Candidates.Num() == 0)
	{
		return INDEX_NONE;
	}

	// Option cycle is over or something is wrong with the setup
	if (CandidatesLimited.Num() == 0)
	{
		// Only allow to preserve last option in list if it is needed and we are sure that
		// a valid option can be picked even if it stays there
		const bool bTempBlockLast = bAvoidPickingSameOptionTwiceInARow && Candidates.Num() > 1;
		if (bTempBlockLast)
		{
			const FGuid TempBlockedEntry = SavedData.GUIDList.Last();
			SavedData.GUIDList.Empty();
			SavedData.GUIDList.Add(TempBlockedEntry);
			const int32 RandomChildNodeIndex = GetRandomChildNodeIndex(Context);
			SavedData.GUIDList.Remove(TempBlockedEntry);
			return RandomChildNodeIndex;
		}
		else
		{
			SavedData.GUIDList.Empty();
			return GetRandomChildNodeIndex(Context);
		}
	}

	// Select Random
	const int32 SelectedIndex = FMath::RandHelper(CandidatesLimited.Num());
	const int32 TargetNodeIndex = Children[CandidatesLimited[SelectedIndex]].TargetIndex;
	const FGuid TargetNodeGUID = Context.GetNodeGUIDForIndex(TargetNodeIndex);

	// if we cycle through everything the list of picked nodes is needed
	if (bCycleThroughSatisfiedOptionsWithoutRepetition)
	{
		// add the currently picked node to the disallow list, it will be cleared on selection if all valid options are added
		SavedData.GUIDList.Add(TargetNodeGUID);
	}
	else if (bAvoidPickingSameOptionTwiceInARow)
	{
		// only disallow the currently picked node for the next selection
		SavedData.GUIDList = { TargetNodeGUID };
	}

	return TargetNodeIndex;
}
