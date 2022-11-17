// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgDetailsPanelUtils.h"
#include "DlgSystem/DlgHelper.h"

UDialogueGraphNode_Base* FDlgDetailsPanelUtils::GetGraphNodeBaseFromPropertyHandle(const TSharedRef<IPropertyHandle>& PropertyHandle)
{
	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (UObject* Object : OuterObjects)
	{
		if (UDlgNode* Node = Cast<UDlgNode>(Object))
		{
			return CastChecked<UDialogueGraphNode_Base>(Node->GetGraphNode());
		}

		if (UDialogueGraphNode_Base* Node = Cast<UDialogueGraphNode_Base>(Object))
		{
			return Node;
		}
	}

	return nullptr;
}

UDialogueGraphNode* FDlgDetailsPanelUtils::GetClosestGraphNodeFromPropertyHandle(const TSharedRef<IPropertyHandle>& PropertyHandle)
{
	if (UDialogueGraphNode_Base* BaseGraphNode = GetGraphNodeBaseFromPropertyHandle(PropertyHandle))
	{
		if (UDialogueGraphNode* Node = Cast<UDialogueGraphNode>(BaseGraphNode))
		{
			return Node;
		}
		if (UDialogueGraphNode_Edge* GraphEdge = Cast<UDialogueGraphNode_Edge>(BaseGraphNode))
		{
			if (GraphEdge->HasParentNode())
			{
				return GraphEdge->GetParentNode();
			}
		}
	}

	return nullptr;
}

UDlgDialogue* FDlgDetailsPanelUtils::GetDialogueFromPropertyHandle(const TSharedRef<IPropertyHandle>& PropertyHandle)
{
	UDlgDialogue* Dialogue = nullptr;

	// Check first children objects of property handle, should be a dialogue node or a graph node
	if (UDialogueGraphNode_Base* GraphNode = GetGraphNodeBaseFromPropertyHandle(PropertyHandle))
	{
		Dialogue = GraphNode->GetDialogue();
	}

	// One last try, get to the root of the problem ;)
	if (!IsValid(Dialogue))
	{
		TSharedPtr<IPropertyHandle> ParentHandle = PropertyHandle->GetParentHandle();
		// Find the root property handle
		while (ParentHandle.IsValid() && ParentHandle->GetParentHandle().IsValid())
		{
			ParentHandle = ParentHandle->GetParentHandle();
		}

		// The outer should be a dialogue
		if (ParentHandle.IsValid())
		{
			TArray<UObject*> OuterObjects;
			ParentHandle->GetOuterObjects(OuterObjects);
			for (UObject* Object : OuterObjects)
			{
				if (UDlgDialogue* FoundDialogue = Cast<UDlgDialogue>(Object))
				{
					Dialogue = FoundDialogue;
					break;
				}
			}
		}
	}

	return Dialogue;
}

FName FDlgDetailsPanelUtils::GetParticipantNameFromPropertyHandle(const TSharedRef<IPropertyHandle>& ParticipantNamePropertyHandle)
{
	FName ParticipantName = NAME_None;
	if (ParticipantNamePropertyHandle->GetValue(ParticipantName) != FPropertyAccess::Success)
	{
		return ParticipantName;
	}

	// Try the node that owns this
	if (ParticipantName.IsNone())
	{
		// Possible edge?
		if (UDialogueGraphNode* GraphNode = GetClosestGraphNodeFromPropertyHandle(ParticipantNamePropertyHandle))
		{
			return GraphNode->GetDialogueNode().GetNodeParticipantName();
		}
	}

	return ParticipantName;
}

TArray<FName> FDlgDetailsPanelUtils::GetDialogueSortedParticipantNames(UDlgDialogue* Dialogue)
{
	if (Dialogue == nullptr)
	{
		return {};
	}

	TSet<FName> ParticipantNames = Dialogue->GetParticipantNames();
	FDlgHelper::SortDefault(ParticipantNames);
	return ParticipantNames.Array();
}
