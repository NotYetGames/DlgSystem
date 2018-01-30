// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"

#include "DlgManager.h"

class UDlgDialogue;

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

/** Structure that holds the common properties of a Variable. */
class FDlgTreeViewVariableProperties
{
public:
	FDlgTreeViewVariableProperties(const TSet<TWeakObjectPtr<UDlgDialogue>>& InDialogues) : Dialogues(InDialogues) {}
	virtual ~FDlgTreeViewVariableProperties() {}

	// Dialogues:
	virtual void AddDialogue(TWeakObjectPtr<UDlgDialogue> Dialogue) { Dialogues.Add(Dialogue); }
	const TSet<TWeakObjectPtr<UDlgDialogue>>& GetDialogues() const { return Dialogues; }

	/** Sorts all the properties it can */
	virtual void Sort()
	{
		Dialogues.Sort(PredicateSortDialogueWeakPtrAlphabeticallyAscending);
	}

protected:
	/** Dialogues that contain this variable property */
	TSet<TWeakObjectPtr<UDlgDialogue>> Dialogues;
};
