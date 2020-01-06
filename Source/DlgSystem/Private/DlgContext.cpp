// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgContext.h"

#include "DlgSystemPrivatePCH.h"
#include "Nodes/DlgNode.h"
#include "Nodes/DlgNode_End.h"
#include "DlgDialogueParticipant.h"
#include "DlgMemory.h"
#include "Engine/Texture2D.h"
#include "Logging/DlgLogger.h"

bool UDlgContext::ChooseChildBasedOnAllOptionIndex(int32 Index)
{
	if (!AllChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid index %d in UDlgContext::ChooseChildBasedOnAllOptionIndex!"), Index);
		bDialogueEnded = true;
		return false;
	}

	if (!AllChildren[Index].bSatisfied)
	{
		FDlgLogger::Get().Errorf(TEXT("Index %d is an unsatisfied edge! (UDlgContext::ChooseChildBasedOnAllOptionIndex!) Call failed!"), Index);
		bDialogueEnded = true;
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
	bDialogueEnded = true;
	return false;
}

const FText& UDlgContext::GetOptionText(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option = %d in GetOptionText!"), OptionIndex);
		return FText::GetEmpty();
	}

	return AvailableChildren[OptionIndex]->GetText();
}

FName UDlgContext::GetOptionSpeakerState(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option = %d in GetOptionSpeakerState!"), OptionIndex);
		return NAME_None;
	}

	return AvailableChildren[OptionIndex]->SpeakerState;
}

const FDlgEdge& UDlgContext::GetOption(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option index %d in GetOption!"), OptionIndex);
		return FDlgEdge::GetInvalidEdge();
	}

	return *AvailableChildren[OptionIndex];
}

const FText& UDlgContext::GetOptionTextFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option = %d in GetOptionTextFromAll!"), Index);
		return FText::GetEmpty();
	}

	return AllChildren[Index].EdgePtr->GetText();
}

bool UDlgContext::IsOptionSatisfied(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option index %d in IsOptionSatisfied!"), Index);
		return false;
	}

	return AllChildren[Index].bSatisfied;
}

FName UDlgContext::GetOptionSpeakerStateFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option = %d in GetOptionSpeakerStateFromAll!"), Index);
		return NAME_None;
	}

	return AllChildren[Index].EdgePtr->SpeakerState;
}

const FDlgEdge& UDlgContext::GetOptionFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option index %d in GetOptionFromAll!"), Index);
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

FName UDlgContext::GetActiveNodeSpeakerState() const
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

UObject* UDlgContext::GetActiveNodeGenericData() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return nullptr;
	}

	return Node->GetGenericData();
}

UDlgNodeData* UDlgContext::GetActiveNodeData() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return nullptr;
	}

	return Node->GetNodeData();
}

UTexture2D* UDlgContext::GetActiveNodeParticipantIcon() const
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

	const FName SpeakerName = Node->GetNodeParticipantName();
	UObject* const* Item = Participants.Find(SpeakerName);
	if (Item == nullptr || !IsValid(*Item))
	{
		return nullptr;
	}

	return IDlgDialogueParticipant::Execute_GetParticipantIcon(*Item, SpeakerName, Node->GetSpeakerState());
}

UObject* UDlgContext::GetActiveNodeParticipant() const
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

FName UDlgContext::GetActiveNodeParticipantName() const
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
	auto* ParticipantPtr = Participants.Find(DlgParticipantName);
	if (ParticipantPtr != nullptr && IsValid(*ParticipantPtr))
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
			FDlgLogger::Get().Errorf(TEXT("UDlgContext::IsEdgeConnectedToVisitedNode failed - invalid index %d"), Index);
			return false;
		}
		TargetIndex = AvailableChildren[Index]->TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			FDlgLogger::Get().Errorf(TEXT("UDlgContext::IsEdgeConnectedToVisitedNode failed - invalid index %d"), Index);
			return false;
		}
		TargetIndex = AllChildren[Index].EdgePtr->TargetIndex;
	}

	if (bLocalHistory)
	{
		return VisitedNodeIndices.Contains(TargetIndex);
	}
	
	if (Dialogue == nullptr)
	{
		FDlgLogger::Get().Errorf(TEXT("UDlgContext::IsEdgeConnectedToVisitedNode called, but the context does not have a valid dialogue!"), Index);
		return false;
	}

	return FDlgMemory::GetInstance()->IsNodeVisited(Dialogue->GetDlgGuid(), TargetIndex);
}


bool UDlgContext::IsEdgeConnectedToEndNode(int32 Index, bool bIndexSkipsUnsatisfiedEdges) const
{
	int32 TargetIndex = INDEX_NONE;

	if (bIndexSkipsUnsatisfiedEdges)
	{
		if (!AvailableChildren.IsValidIndex(Index))
		{
			FDlgLogger::Get().Errorf(TEXT("UDlgContext::IsEdgeConnectedToEndNode failed - AvailableChildren invalid index %d"), Index);
			return false;
		}
		TargetIndex = AvailableChildren[Index]->TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			FDlgLogger::Get().Errorf(TEXT("UDlgContext::IsEdgeConnectedToEndNode failed - AllChildren invalid index %d"), Index);
			return false;
		}
		TargetIndex = AllChildren[Index].EdgePtr->TargetIndex;
	}

	if (Dialogue == nullptr)
	{
		FDlgLogger::Get().Error(TEXT("UDlgContext::IsEdgeConnectedToEndNode called, but the context does not have a valid dialogue!"));
		return false;
	}

	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();

	if (Nodes.IsValidIndex(TargetIndex))
	{
		return Cast<UDlgNode_End>(Nodes[TargetIndex]) != nullptr;
	}

	FDlgLogger::Get().Error(TEXT("UDlgContext::IsEdgeConnectedToEndNode called, but the examined edge does not point to a valid node!"));
	return false;
}

