// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "DlgBrowserTreeNode.h"
#include "DialogueTreeProperties/DlgBrowserTreeParticipantProperties.h"

enum class EDlgBrowserSortOption : uint8
{
	Name = 0,
	DialogueReferences
};

struct FDlgBrowserSortOption
{
public:
	FDlgBrowserSortOption(EDlgBrowserSortOption InOption, FName InName)
		: Option(InOption), Name(InName) {}

	FName GetFName() const { return Name; }
	FText GetFText() const { return FText::FromName(Name); }
	FString GetFString() const { return Name.ToString(); }

	bool IsByName() const { return Option == EDlgBrowserSortOption::Name; }

public:
	EDlgBrowserSortOption Option;

	// The name of the option.
	FName Name;

	// TODO add ascending descending
};

class FDlgBrowserUtilities
{
public:
	// Compare two FDlgBrowserTreeNode
	static bool PredicateCompareDialogueTreeNode(
		const TSharedPtr<FDlgBrowserTreeNode>& FirstNode,
		const TSharedPtr<FDlgBrowserTreeNode> SecondNode
	)
	{
		check(FirstNode.IsValid());
		check(SecondNode.IsValid());
		return *FirstNode == *SecondNode;
	}

	// Predicate that sorts participants by dialogue number references, in descending order.
	static bool PredicateSortByDialoguesNumDescending(
		FName FirstParticipant,
		FName SecondParticipant,
		const TMap<FName, TSharedPtr<FDlgBrowserTreeParticipantProperties>>& ParticipantsProperties
	)
	{
		int32 FirstNum = 0;
		int32 SecondNum = 0;

		const TSharedPtr<FDlgBrowserTreeParticipantProperties>* FirstPtr =
			ParticipantsProperties.Find(FirstParticipant);
		if (FirstPtr)
		{
			FirstNum = (*FirstPtr)->GetDialogues().Num();
		}
		const TSharedPtr<FDlgBrowserTreeParticipantProperties>* SecondPtr =
			ParticipantsProperties.Find(SecondParticipant);
		if (SecondPtr)
		{
			SecondNum = (*SecondPtr)->GetDialogues().Num();
		}

		return FirstNum > SecondNum;
	}
};
