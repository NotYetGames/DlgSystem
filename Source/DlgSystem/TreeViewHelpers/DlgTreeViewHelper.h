// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STreeView.h"

#include "DlgSystem/DlgHelper.h"

class DLGSYSTEM_API FDlgTreeViewHelper
{
public:
	static bool PredicateSortDialogueWeakPtrAlphabeticallyAscending(
		const TWeakObjectPtr<const UDlgDialogue>& First,
		const TWeakObjectPtr<const UDlgDialogue>& Second
	)
	{
		if (!First.IsValid())
		{
			return false;
		}
		if (!Second.IsValid())
		{
			return true;
		}

		return FDlgHelper::PredicateSortFNameAlphabeticallyAscending(First->GetFName(), Second->GetFName());
	}

	/** Restore the expansion state of the InTree with the ItemSource provided (usually the flattened tree) */
	template<typename ItemType, typename ComparisonType>
	static void RestoreTreeExpansionState(
		const TSharedPtr<STreeView<ItemType>>& InTree,
		const TArray<ItemType>& ItemSource,
		const TSet<ItemType>& OldExpansionState, ComparisonType ComparisonFunction
	)
	{
		check(InTree.IsValid());

		// Iterate over new tree items
		for (int32 ItemIdx = 0; ItemIdx < ItemSource.Num(); ItemIdx++)
		{
			ItemType NewItem = ItemSource[ItemIdx];

			// Look through old expansion state
			for (const ItemType OldItem : OldExpansionState)
			{
				// See if this matches this new item
				if (ComparisonFunction(OldItem, NewItem))
				{
					// It does, so expand it
					InTree->SetItemExpansion(NewItem, true);
				}
			}
		}
	}
};
