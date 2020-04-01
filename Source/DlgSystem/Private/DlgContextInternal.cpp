// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
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
		if (ChildLink.IsValid() && ChildLink.Evaluate(this, {}))
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
		if (ChildLink.IsValid() && ChildLink.Evaluate(this, {}))
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
	FDlgMemory::Get().SetNodeVisited(Dialogue->GetDlgGuid(), ActiveNodeIndex);
	VisitedNodeIndices.Add(ActiveNodeIndex);

	return Node->ReevaluateChildren(this, {});
}
