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
		FDlgLogger::Get().Errorf(TEXT("Invalid index %d in UDlgContext::ChooseChildBasedOnAllOptionIndex!"), Index);
		bDialogueEnded = true;
		return false;
	}

	if (!AllChildren[Index].IsSatisfied())
	{
		FDlgLogger::Get().Errorf(TEXT("Index %d is an unsatisfied edge! (UDlgContext::ChooseChildBasedOnAllOptionIndex!) Call failed!"), Index);
		bDialogueEnded = true;
		return false;
	}

	for (int32 i = 0; i < AvailableChildren.Num(); ++i)
	{
		if (AvailableChildren[i].GetEdge() == AllChildren[Index].GetEdge())
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
		FDlgLogger::Get().Errorf(
			TEXT("Dialogue = `%s` Failed to update dialogue options for - invalid ActiveNodeIndex %d"),
			*Dialogue->GetPathName(), ActiveNodeIndex
		);
		return;
	}

	Node->ReevaluateChildren(this, {});
}

const FText& UDlgContext::GetOptionText(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option = %d in GetOptionText!"), OptionIndex);
		return FText::GetEmpty();
	}

	return AvailableChildren[OptionIndex].GetEdge().GetText();
}

FName UDlgContext::GetOptionSpeakerState(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option = %d in GetOptionSpeakerState!"), OptionIndex);
		return NAME_None;
	}

	return AvailableChildren[OptionIndex].GetEdge().SpeakerState;
}

const FDlgEdge& UDlgContext::GetOption(int32 OptionIndex) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(OptionIndex))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option index %d in GetOption!"), OptionIndex);
		return FDlgEdge::GetInvalidEdge();
	}

	return AvailableChildren[OptionIndex].GetEdge();
}

const FText& UDlgContext::GetOptionTextFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option = %d in GetOptionTextFromAll!"), Index);
		return FText::GetEmpty();
	}

	return AllChildren[Index].GetEdge().GetText();
}

bool UDlgContext::IsOptionSatisfied(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option index %d in IsOptionSatisfied!"), Index);
		return false;
	}

	return AllChildren[Index].IsSatisfied();
}

FName UDlgContext::GetOptionSpeakerStateFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AllChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option = %d in GetOptionSpeakerStateFromAll!"), Index);
		return NAME_None;
	}

	return AllChildren[Index].GetEdge().SpeakerState;
}

const FDlgEdge& UDlgContext::GetOptionFromAll(int32 Index) const
{
	check(Dialogue);

	if (!AvailableChildren.IsValidIndex(Index))
	{
		FDlgLogger::Get().Errorf(TEXT("Invalid option index %d in GetOptionFromAll!"), Index);
		return FDlgEdge::GetInvalidEdge();
	}

	return AllChildren[Index].GetEdge();
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

USoundBase* UDlgContext::GetActiveNodeVoiceSoundBase() const
{
	const UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		return nullptr;
	}

	return Node->GetNodeVoiceSoundBase();
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
		TargetIndex = AvailableChildren[Index].GetEdge().TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			FDlgLogger::Get().Errorf(TEXT("UDlgContext::IsEdgeConnectedToVisitedNode failed - invalid index %d"), Index);
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
		FDlgLogger::Get().Errorf(TEXT("UDlgContext::IsEdgeConnectedToVisitedNode called, but the context does not have a valid dialogue!"), Index);
		return false;
	}

	return FDlgMemory::Get().IsNodeVisited(Dialogue->GetDialogueGUID(), TargetIndex);
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
		TargetIndex = AvailableChildren[Index].GetEdge().TargetIndex;
	}
	else
	{
		if (!AllChildren.IsValidIndex(Index))
		{
			FDlgLogger::Get().Errorf(TEXT("UDlgContext::IsEdgeConnectedToEndNode failed - AllChildren invalid index %d"), Index);
			return false;
		}
		TargetIndex = AllChildren[Index].GetEdge().TargetIndex;
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

bool UDlgContext::EnterNode(int32 NodeIndex, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	check(Dialogue);

	UDlgNode* Node = GetMutableNode(NodeIndex);
	if (!IsValid(Node))
	{
		FDlgLogger::Get().Errorf(
			TEXT("Dialogue = `%s`. Failed to enter dialogue node - invalid node index %d"),
			*Dialogue->GetPathName(), NodeIndex
		);
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

bool UDlgContext::CouldBeStarted(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants) const
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
	if (!InDialogue)
	{
		FDlgLogger::Get().Errorf(TEXT("Failed to start Dialogue because the supplied Dialogue Asset is Invalid (nullptr)"));
		return false;
	}

	Dialogue = InDialogue;
	Participants = InParticipants;

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

	FDlgLogger::Get().Errorf(
        TEXT("Failed to start Dialogue = `%s` - all possible start node condition failed. "
            "Edge conditions and children enter conditions from the start node are not satisfied."),
        *InDialogue->GetPathName()
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
	if (!InDialogue)
	{
		FDlgLogger::Get().Errorf(TEXT("Failed to start Dialogue from Index because the supplied Dialogue Asset is Invalid (nullptr)"));
		return false;
	}

	Dialogue = InDialogue;
	Participants = InParticipants;
	VisitedNodeIndices = VisitedNodes;

	UDlgNode* Node = GetMutableNode(StartIndex);
	if (!IsValid(Node))
	{
		FDlgLogger::Get().Errorf(
            TEXT("Failed to start dialogue = `%s` at index %d - is it invalid index?!"),
            *Dialogue->GetPathName(), StartIndex
        );
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
