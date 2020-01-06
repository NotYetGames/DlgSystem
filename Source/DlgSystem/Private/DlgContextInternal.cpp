// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgContextInternal.h"

#include "DlgSystemPrivatePCH.h"
#include "Nodes/DlgNode.h"
#include "DlgMemory.h"
#include "Logging/DlgLogger.h"


bool UDlgContextInternal::Initialize(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants)
{
	Dialogue = InDialogue;
	Participants = InParticipants;

	// Evaluate edges/children of the start node
	const UDlgNode& StartNode = Dialogue->GetStartNode();
	for (const FDlgEdge& ChildLink : StartNode.GetNodeChildren())
	{
		if (ChildLink.TargetIndex != INDEX_NONE && ChildLink.Evaluate(this, {}))
		{
			if (EnterNode(ChildLink.TargetIndex, {}))
			{
				return true;
			}
		}
	}

	FDlgLogger::Get().Errorf(
		TEXT("Failed to start Dialogue = `%s`: all possible start node condition failed. "
			"Edge conditions and children enter conditions from the start node are not satisfied."),
		*InDialogue->GetPathName()
	);
	return false;
}


bool UDlgContextInternal::CouldBeInitialized(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants)
{
	Dialogue = InDialogue;
	Participants = InParticipants;

	// Evaluate edges/children of the start node
	const UDlgNode& StartNode = Dialogue->GetStartNode();
	for (const FDlgEdge& ChildLink : StartNode.GetNodeChildren())
	{
		if (ChildLink.TargetIndex != INDEX_NONE && ChildLink.Evaluate(this, {}))
		{
			return true;
		}
	}

	return false;
}


bool UDlgContextInternal::Initialize(UDlgDialogue* InDialogue, const TMap<FName, UObject*>& InParticipants, int32 StartIndex, const TSet<int32>& VisitedNodes, bool bFireEnterEvents)
{
	Dialogue = InDialogue;
	Participants = InParticipants;
	VisitedNodeIndices = VisitedNodes;

	UDlgNode* Node = GetNode(StartIndex);
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
	FDlgMemory::GetInstance()->SetNodeVisited(Dialogue->GetDlgGuid(), ActiveNodeIndex);
	VisitedNodeIndices.Add(ActiveNodeIndex);

	return Node->ReevaluateChildren(this, {});
}


bool UDlgContextInternal::EnterNode(int32 NodeIndex, TSet<const UDlgNode*> NodesEnteredWithThisStep)
{
	check(Dialogue);

	UDlgNode* Node = GetNode(NodeIndex);
	if (!IsValid(Node))
	{
		FDlgLogger::Get().Errorf(
			TEXT("Dialogue = `%s`. Failed to enter dialogue node - invalid node index %d"),
			*Dialogue->GetPathName(), NodeIndex
		);
		return false;
	}

	ActiveNodeIndex = NodeIndex;
	FDlgMemory::GetInstance()->SetNodeVisited(Dialogue->GetDlgGuid(), ActiveNodeIndex);
	VisitedNodeIndices.Add(ActiveNodeIndex);

	return Node->HandleNodeEnter(this, NodesEnteredWithThisStep);
}


bool UDlgContextInternal::ChooseChild(int32 OptionIndex)
{
	check(Dialogue);
	if (UDlgNode* Node = GetActiveNode())
	{
		if (Node->OptionSelected(OptionIndex, this))
		{
			return true;
		}
	}

	bDialogueEnded = true;
	return false;
}


bool UDlgContextInternal::IsNodeEnterable(int32 NodeIndex, TSet<const UDlgNode*> AlreadyVisitedNodes) const
{
	check(Dialogue);
	if (const UDlgNode* Node = GetNode(NodeIndex))
	{
		return Node->CheckNodeEnterConditions(this, AlreadyVisitedNodes);
	}

	return false;
}


void UDlgContextInternal::ReevaluateChildren()
{
	check(Dialogue);
	UDlgNode* Node = GetActiveNode();
	if (!IsValid(Node))
	{
		FDlgLogger::Get().Errorf(
			TEXT("Dialogue = `%s` Failed to update dialogue options for  - invalid ActiveNodeIndex %d"),
			*Dialogue->GetPathName(), ActiveNodeIndex
		);
		return;
	}

	Node->ReevaluateChildren(this, {});
}

const UDlgNode* UDlgContextInternal::GetNode(int32 NodeIndex) const
{
	check(Dialogue);
	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();

	if (!Nodes.IsValidIndex(NodeIndex))
	{
		return nullptr;
	}

	return Nodes[NodeIndex];
}
