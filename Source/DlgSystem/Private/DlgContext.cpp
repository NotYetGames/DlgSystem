// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgContext.h"

#include "DlgSystemPrivatePCH.h"
#include "Nodes/DlgNode.h"
#include "Nodes/DlgNode_End.h"
#include "DlgDialogueParticipant.h"
#include "DlgMemory.h"
#include "Engine/Texture2D.h"
#include "Logging/DlgLogger.h"

bool UDlgContext::ChooseChild(int32 OptionIndex)
{
	check(Dialogue);
	if (UDlgNode* Node = GetMutableActiveNode())
	{
		if (Node->OptionSelected(OptionIndex, this))
		{
			return true;
		}
	}

	bDialogueEnded = true;
	return false;
}

bool UDlgContext::ChooseChildBasedOnAllOptionIndex(int32 Index)
{
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("ChooseChildBasedOnAllOptionIndex - INVALID given Index = %d"), Index));
		bDialogueEnded = true;
		return false;
	}

	if (!AllChildren[Index].IsSatisfied())
	{
		LogErrorWithContext(FString::Printf(TEXT("ChooseChildBasedOnAllOptionIndex - given Index = %d is an unsatisfied edge"), Index));
		bDialogueEnded = true;
		return false;
	}

	for (int32 i = 0; i < AvailableChildren.Num(); ++i)
	{
		if (AvailableChildren[i] == AllChildren[Index].GetEdge())
		{
			return ChooseChild(i);
		}
	}

	ensure(false);
	bDialogueEnded = true;
	return false;
}

void UDlgContext::ReevaluateChildren()
{
	check(Dialogue);
	UDlgNode* Node = GetMutableActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("ReevaluateChildren - Failed to update dialogue options"));
		return;
	}

	Node->ReevaluateChildren(this, {});
}

const FText& UDlgContext::GetOptionText(int32 OptionIndex) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionText - INVALID given OptionIndex = %d"), OptionIndex));
		return FText::GetEmpty();
	}

	return AvailableChildren[OptionIndex].GetText();
}

FName UDlgContext::GetOptionSpeakerState(int32 OptionIndex) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionSpeakerState - INVALID given OptionIndex = %d"), OptionIndex));
		return NAME_None;
	}

	return AvailableChildren[OptionIndex].SpeakerState;
}

const TArray<FDlgCondition>& UDlgContext::GetOptionEnterConditions(int32 OptionIndex) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionEnterConditions - INVALID given OptionIndex = %d"), OptionIndex));
		static TArray<FDlgCondition> EmptyArray;
		return EmptyArray;
	}

	return AvailableChildren[OptionIndex].Conditions;
}

const FDlgEdge& UDlgContext::GetOption(int32 OptionIndex) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOption - INVALID given OptionIndex = %d"), OptionIndex));
		return FDlgEdge::GetInvalidEdge();
	}

	return AvailableChildren[OptionIndex];
}

const FText& UDlgContext::GetOptionTextFromAll(int32 Index) const
{
	check(Dialogue);
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionTextFromAll - INVALID given Index = %d"), Index));
		return FText::GetEmpty();
	}

	return AllChildren[Index].GetEdge().GetText();
}

bool UDlgContext::IsOptionSatisfied(int32 Index) const
{
	check(Dialogue);
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("IsOptionSatisfied - INVALID given Index = %d"), Index));
		return false;
	}

	return AllChildren[Index].IsSatisfied();
}

FName UDlgContext::GetOptionSpeakerStateFromAll(int32 Index) const
{
	check(Dialogue);
	if (!AllChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionSpeakerStateFromAll - INVALID given Index = %d"), Index));
		return NAME_None;
	}

	return AllChildren[Index].GetEdge().SpeakerState;
}

const FDlgEdgeData& UDlgContext::GetOptionFromAll(int32 Index) const
{
	check(Dialogue);
	if (!AvailableChildren.IsValidIndex(Index))
	{
		LogErrorWithContext(FString::Printf(TEXT("GetOptionFromAll - INVALID given Index = %d"), Index));
		return FDlgEdgeData::GetInvalidEdge();
	}

	return AllChildren[Index];
}

const FText& UDlgContext::GetActiveNodeText() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeText - INVALID Active Node"));
		return FText::GetEmpty();
	}

	return Node->GetNodeText();
}

FName UDlgContext::GetActiveNodeSpeakerState() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeSpeakerState - INVALID Active Node"));
		return NAME_None;
	}

	return Node->GetSpeakerState();
}

USoundWave* UDlgContext::GetActiveNodeVoiceSoundWave() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeVoiceSoundWave - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeVoiceSoundWave();
}

USoundBase* UDlgContext::GetActiveNodeVoiceSoundBase() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeVoiceSoundBase - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeVoiceSoundBase();
}

UDialogueWave* UDlgContext::GetActiveNodeVoiceDialogueWave() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeVoiceDialogueWave - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeVoiceDialogueWave();
}

UObject* UDlgContext::GetActiveNodeGenericData() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeGenericData - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeGenericData();
}

UDlgNodeData* UDlgContext::GetActiveNodeData() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeData - INVALID Active Node"));
		return nullptr;
	}

	return Node->GetNodeData();
}

UTexture2D* UDlgContext::GetActiveNodeParticipantIcon() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeParticipantIcon - INVALID Active Node"));
		return nullptr;
	}

	const FName SpeakerName = Node->GetNodeParticipantName();
	auto* ObjectPtr = Participants.Find(SpeakerName);
	if (ObjectPtr == nullptr || !IsValid(*ObjectPtr))
	{
		LogErrorWithContext(FString::Printf(
			TEXT("GetActiveNodeParticipantIcon - The ParticipantName = `%s` from the Active Node does NOT exist in the current Participants"),
			*SpeakerName.ToString()
		));
		return nullptr;
	}

	return IDlgDialogueParticipant::Execute_GetParticipantIcon(*ObjectPtr, SpeakerName, Node->GetSpeakerState());
}

UObject* UDlgContext::GetActiveNodeParticipant() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeParticipant - INVALID Active Node"));
		return nullptr;
	}

	const FName SpeakerName = Node->GetNodeParticipantName();
	auto* ObjectPtr = Participants.Find(Node->GetNodeParticipantName());
	if (ObjectPtr == nullptr || !IsValid(*ObjectPtr))
	{
		LogErrorWithContext(FString::Printf(
            TEXT("GetActiveNodeParticipant - The ParticipantName = `%s` from the Active Node does NOT exist in the current Participants"),
            *SpeakerName.ToString()
        ));
		return nullptr;
	}

	return *ObjectPtr;
}

FName UDlgContext::GetActiveNodeParticipantName() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		LogErrorWithContext(TEXT("GetActiveNodeParticipantName - INVALID Active Node"));
		return NAME_None;
	}

	return Node->GetNodeParticipantName();
}

UObject* UDlgContext::GetMutableParticipant(FName ParticipantName) const
{
	auto* ParticipantPtr = Participants.Find(ParticipantName);
	if (ParticipantPtr != nullptr && IsValid(*ParticipantPtr))
	{
		return *ParticipantPtr;
	}

	return nullptr;
}

const UObject* UDlgContext::GetParticipant(FName ParticipantName) const
{
	auto* ParticipantPtr = Participants.Find(ParticipantName);
	if (ParticipantPtr != nullptr && IsValid(*ParticipantPtr))
	{
		return *ParticipantPtr;
	}

	return nullptr;
}

bool UDlgContext::IsOptionConnectedToVisitedNode(int32 Index, bool bLocalHistory, bool bIndexSkipsUnsatisfiedEdges) const
{
	int32 TargetIndex = INDEX_NONE;

	if (bIndexSkipsUnsatisfiedEdges)
	{
		if (!AvailableChildren.IsValidIndex(Index))
		{
			LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToVisitedNode - INVALID Index = %d for AvailableChildren"), Index));
			return false;
		}
		TargetIndex = AvailableChildren[Index].TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToVisitedNode - INVALID Index = %d for AllChildren"), Index));
			return false;
		}
		TargetIndex = AllChildren[Index].GetEdge().TargetIndex;
	}

	if (bLocalHistory)
	{
		return VisitedNodeIndices.Contains(TargetIndex);
	}

	if (Dialogue == nullptr)
	{
		LogErrorWithContext(TEXT("IsOptionConnectedToVisitedNode - This Context does not have a valid Dialogue"));
		return false;
	}

	return FDlgMemory::Get().IsNodeVisited(Dialogue->GetDialogueGUID(), TargetIndex);
}

bool UDlgContext::IsOptionConnectedToEndNode(int32 Index, bool bIndexSkipsUnsatisfiedEdges) const
{
	int32 TargetIndex = INDEX_NONE;

	if (bIndexSkipsUnsatisfiedEdges)
	{
		if (!AvailableChildren.IsValidIndex(Index))
		{
			LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToEndNode - INVALID Index = %d for AvailableChildren"), Index));
			return false;
		}
		TargetIndex = AvailableChildren[Index].TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToEndNode - INVALID Index = %d for AllChildren"), Index));
			return false;
		}
		TargetIndex = AllChildren[Index].GetEdge().TargetIndex;
	}

	if (Dialogue == nullptr)
	{
		LogErrorWithContext(TEXT("IsOptionConnectedToEndNode - This Context does not have a valid Dialogue"));
		return false;
	}

	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();
	if (Nodes.IsValidIndex(TargetIndex))
	{
		return Nodes[TargetIndex]->IsA<UDlgNode_End>();
	}

	LogErrorWithContext(FString::Printf(TEXT("IsOptionConnectedToEndNode - The examined Edge/Option at Index = %d does not point to a valid node"), Index));
	return false;
}

bool UDlgContext::EnterNode(int32 NodeIndex, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	check(Dialogue);
	UDlgNode* Node = GetMutableNode(NodeIndex);
	if (!IsValid(Node))
	{
		LogErrorWithContext(FString::Printf(TEXT("EnterNode - FAILED because of INVALID NodeIndex = %d"), NodeIndex));
		return false;
	}

	ActiveNodeIndex = NodeIndex;
	FDlgMemory::Get().SetNodeVisited(Dialogue->GetDialogueGUID(), ActiveNodeIndex);
	VisitedNodeIndices.Add(ActiveNodeIndex);

	return Node->HandleNodeEnter(this, NodesEnteredWithThisStep);
}

UDlgNode* UDlgContext::GetMutableNode(int32 NodeIndex) const
{
	check(Dialogue);
	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();
	if (!Nodes.IsValidIndex(NodeIndex))
	{
		return nullptr;
	}

	return Nodes[NodeIndex];
}

const UDlgNode* UDlgContext::GetNode(int32 NodeIndex) const
{
	check(Dialogue);
	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();
	if (!Nodes.IsValidIndex(NodeIndex))
	{
		return nullptr;
	}

	return Nodes[NodeIndex];
}

bool UDlgContext::IsNodeEnterable(int32 NodeIndex, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	check(Dialogue);
	if (const UDlgNode* Node = GetNode(NodeIndex))
	{
		return Node->CheckNodeEnterConditions(this, AlreadyVisitedNodes);
	}

	return false;
}


FString UDlgContext::GetContextString() const
{
	FString ContextParticipants;
	TSet<FString> ParticipantsNames;
	for (const auto& KeyValue : Participants)
	{
		ParticipantsNames.Add(KeyValue.Key.ToString());
	}

	return FString::Printf(
		TEXT("Dialogue = `%s`, ActiveNodeIndex = %d, Participants Names = `%s`"),
		Dialogue ? *Dialogue->GetPathName() : TEXT("INVALID"),
		ActiveNodeIndex,
		*FString::Join(ParticipantsNames, TEXT(", "))
	);
}

void UDlgContext::LogErrorWithContext(const FString& ErrorMessage) const
{
	FDlgLogger::Get().Errorf(TEXT("%s.\nContext:\n\t%s"), *ErrorMessage, *GetContextString());
}

bool UDlgContext::CanBeStarted(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants) const
{
	if (!InDialogue)
	{
		return false;
	}

	// Evaluate edges/children of the start node
	const UDlgNode& StartNode = InDialogue->GetStartNode();
	for (const FDlgEdge& ChildLink : StartNode.GetNodeChildren())
	{
		if (ChildLink.IsValid() && ChildLink.Evaluate(this, {}))
		{
			return true;
		}
	}

	return false;
}

bool UDlgContext::Start(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants)
{
	Dialogue = InDialogue;
	Participants = InParticipants;

	if (!Dialogue)
	{
		LogErrorWithContext(TEXT("Start - FAILED because the supplied Dialogue Asset is INVALID (nullptr)"));
		return false;
	}

	// Evaluate edges/children of the start node
	const UDlgNode& StartNode = Dialogue->GetStartNode();
	for (const FDlgEdge& ChildLink : StartNode.GetNodeChildren())
	{
		if (ChildLink.IsValid() && ChildLink.Evaluate(this, {}))
		{
			if (EnterNode(ChildLink.TargetIndex, {}))
			{
				return true;
			}
		}
	}

	LogErrorWithContext(
		TEXT("Start - FAILED because all possible start node condition failed. Edge conditions and children enter conditions from the start node are not satisfied")
	);
	return false;
}

bool UDlgContext::StartFromIndex(
	UDlgDialogue* InDialogue,
	const TMap<FName, UObject*>& InParticipants,
	int32 StartIndex,
	const TSet<int32>& VisitedNodes,
	bool bFireEnterEvents
)
{
	Dialogue = InDialogue;
	Participants = InParticipants;
	VisitedNodeIndices = VisitedNodes;

	if (!Dialogue)
	{
		LogErrorWithContext(TEXT("StartFromIndex - FAILED because the supplied Dialogue Asset is INVALID (nullptr)"));
		return false;
	}

	UDlgNode* Node = GetMutableNode(StartIndex);
	if (!IsValid(Node))
	{
		LogErrorWithContext(FString::Printf(
			TEXT("StartFromIndex - FAILED because StartIndex = %d is an INVALID index"),
			StartIndex
		));
		return false;
	}

	if (bFireEnterEvents)
	{
		return EnterNode(StartIndex, {});
	}

	ActiveNodeIndex = StartIndex;
	FDlgMemory::Get().SetNodeVisited(Dialogue->GetDialogueGUID(), ActiveNodeIndex);
	VisitedNodeIndices.Add(ActiveNodeIndex);

	return Node->ReevaluateChildren(this, {});
}
