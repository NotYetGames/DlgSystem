// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgContextInternal.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgNode.h"
#include "DlgMemory.h"


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
				return true;
		}
	}

	UE_LOG(LogDlgSystem, Error, TEXT("Failed to start Dialogue = `%s`: all possible start node condition failed. "
									 "Edge conditions and children enter conditions from the start node are not satisfied."), *InDialogue->GetPathName());
	return false;
}


bool UDlgContextInternal::EnterNode(int32 NodeIndex, TSet<UDlgNode*> NodesEnteredWithThisStep)
{
	check(Dialogue);

	UDlgNode* Node = GetNode(NodeIndex);
	if (Node == nullptr)
	{
		UE_LOG(LogDlgSystem, Warning, TEXT("Failed to enter dialouge node - invalid node index %d"), NodeIndex);
		return false;
	}

	ActiveNodeIndex = NodeIndex;
	DlgMemory::GetInstance()->SetNodeVisited(Dialogue->GetDlgGuid(), ActiveNodeIndex);
	VisitedNodeIndices.Add(ActiveNodeIndex);

	return Node->HandleNodeEnter(this, NodesEnteredWithThisStep);
}


bool UDlgContextInternal::ChooseChild(int32 OptionIndex)
{
	check(Dialogue);
	UDlgNode* Node = GetActiveNode();
	if (Node != nullptr)
	{
		return Node->OptionSelected(OptionIndex, this);
	}

	return false;
}


bool UDlgContextInternal::IsNodeEnterable(int32 NodeIndex, TSet<UDlgNode*> AlreadyVisitedNodes)
{
	check(Dialogue);

	UDlgNode* Node = GetNode(NodeIndex);
	if (Node != nullptr)
	{
		return Node->CheckNodeEnterConditions(this, AlreadyVisitedNodes);
	}

	return false;
}


void UDlgContextInternal::ReevaluateChildren()
{
	check(Dialogue);
	UDlgNode* Node = GetActiveNode();
	if (Node == nullptr)
	{
		UE_LOG(LogDlgSystem, Warning, TEXT("Failed to update dialogue options - invalid ActiveNodeIndex %d"), ActiveNodeIndex);
		return;
	}

	Node->ReevaluateChildren(this, {});
}


UDlgNode* UDlgContextInternal::GetNode(int32 NodeIndex)
{
	check(Dialogue);
	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();

	if (!Nodes.IsValidIndex(NodeIndex))
	{
		return nullptr;
	}

	return Nodes[NodeIndex];
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

