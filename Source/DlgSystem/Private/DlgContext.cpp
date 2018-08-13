// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgContext.h"

#include "DlgSystemPrivatePCH.h"
#include "Nodes/DlgNode.h"
#include "DlgDialogueParticipant.h"
#include "DlgMemory.h"

bool UDlgContext::ChooseChildBasedOnAllOptionIndex(int32 Index)
{
	if (!AllChildren.IsValidIndex(Index))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid index %d in UDlgContext::ChooseChildBasedOnAllOptionIndex!"), Index);
		return false;
	}

	if (!AllChildren[Index].bSatisfied)
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Index %d is an unsatisfied edge! (UDlgContext::ChooseChildBasedOnAllOptionIndex!) Call failed!"), Index);
		return false;
	}

	for (int32 i = 0; i < AvailableChildren.Num(); ++i)
	{
		if (AvailableChildren[i] == AllChildren[Index].EdgePtr)
		{
			return ChooseChild(i);
		}
	}

	ensure(false);
	return false;
}

const FText& UDlgContext::GetOptionText(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid option = %d in GetOptionText!"), OptionIndex);
		return FText::GetEmpty();
	}

	return AvailableChildren[OptionIndex]->GetEdgeText();
}

FName UDlgContext::GetOptionSpeakerState(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid option = %d in GetOptionSpeakerState!"), OptionIndex);
		return NAME_None;
	}

	return AvailableChildren[OptionIndex]->SpeakerState;
}

const FDlgEdge& UDlgContext::GetOption(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid option index %d in GetOption!"), OptionIndex);
		return FDlgEdge::GetInvalidEdge();
	}

	return *AvailableChildren[OptionIndex];
}

const FText& UDlgContext::GetOptionTextFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid option = %d in GetOptionTextFromAll!"), Index);
		return FText::GetEmpty();
	}

	return AllChildren[Index].EdgePtr->GetEdgeText();
}

bool UDlgContext::IsOptionSatisfied(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid option index %d in IsOptionSatisfied!"), Index);
		return false;
	}

	return AllChildren[Index].bSatisfied;
}

FName UDlgContext::GetOptionSpeakerStateFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid option = %d in GetOptionSpeakerStateFromAll!"), Index);
		return NAME_None;
	}

	return AllChildren[Index].EdgePtr->SpeakerState;
}

const FDlgEdge& UDlgContext::GetOptionFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(Index))
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Invalid option index %d in GetOptionFromAll!"), Index);
		return FDlgEdge::GetInvalidEdge();
	}

	return *AllChildren[Index].EdgePtr;
}


const FText& UDlgContext::GetActiveNodeText() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return FText::GetEmpty();
	}

	return Node->GetNodeText();
}

FName UDlgContext::GetActiveSpeakerState() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return NAME_None;
	}

	return Node->GetSpeakerState();
}

USoundWave* UDlgContext::GetActiveNodeVoiceSoundWave() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return nullptr;
	}

	return Node->GetNodeVoiceSoundWave();
}

UDialogueWave* UDlgContext::GetActiveNodeVoiceDialogueWave() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return nullptr;
	}

	return Node->GetNodeVoiceDialogueWave();
}

UTexture2D* UDlgContext::GetActiveParticipantIcon() const
{
	if (!IsValid(Dialogue))
	{
		return nullptr;
	}

	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return nullptr;
	}

	FName SpeakerName = Node->GetNodeParticipantName();
	UObject* const* Item = Participants.Find(SpeakerName);
	if (Item == nullptr || !IsValid(*Item))
	{
		return nullptr;
	}

	return IDlgDialogueParticipant::Execute_GetParticipantIcon(*Item, SpeakerName, Node->GetSpeakerState());
}

UObject* UDlgContext::GetActiveParticipant() const
{
	if (!IsValid(Dialogue))
	{
		return nullptr;
	}

	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return nullptr;
	}

	UObject* const* Item = Participants.Find(Node->GetNodeParticipantName());
	return Item == nullptr ? nullptr : *Item;
}

FName UDlgContext::GetActiveParticipantName() const
{
	if (!IsValid(Dialogue))
	{
		return NAME_None;
	}

	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return NAME_None;
	}

	return Node->GetNodeParticipantName();
}


const UObject* UDlgContext::GetConstParticipant(FName DlgParticipantName) const
{
	const UObject* const* ParticipantPtr = Participants.Find(DlgParticipantName);
	if (ParticipantPtr != nullptr)
	{
		return *ParticipantPtr;
	}

	return nullptr;
}


bool UDlgContext::IsEdgeConnectedToVisitedNode(int32 Index, bool bLocalHistory, bool bIndexSkipsUnsatisfiedEdges) const
{
	int32 TargetIndex = INDEX_NONE;

	if (bIndexSkipsUnsatisfiedEdges)
	{
		if (!AvailableChildren.IsValidIndex(Index))
		{
			UE_LOG(LogDlgSystem, Error, TEXT("UDlgContext::IsEdgeConnectedToVisitedNode failed - invalid index %d"), Index);
			return false;
		}
		TargetIndex = AvailableChildren[Index]->TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			UE_LOG(LogDlgSystem, Error, TEXT("UDlgContext::IsEdgeConnectedToVisitedNode failed - invalid index %d"), Index);
			return false;
		}
		TargetIndex = AllChildren[Index].EdgePtr->TargetIndex;
	}

	if (bLocalHistory)
	{
		return VisitedNodeIndices.Contains(TargetIndex);
	}
	else
	{
		if (Dialogue == nullptr)
		{
			UE_LOG(LogDlgSystem, Error, TEXT("UDlgContext::IsEdgeConnectedToVisitedNode called, but the context does not have a valid dialogue!"));
			return false;
		}

		return FDlgMemory::GetInstance()->IsNodeVisited(Dialogue->GetDlgGuid(), TargetIndex);
	}
}
