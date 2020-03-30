// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueGraphNode_Edge.h"

#include "DialogueGraphNode.h"

#define LOCTEXT_NAMESPACE "DialogueGraphNode_Edge"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UObject interface
void UDialogueGraphNode_Edge::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Special case when redoing
	if (HasParentNode() && HasChildNode())
	{
		// Always keep in sync with the Edge of the Dialogue Node
		FDlgEdge* ParentNodeDialogueEdge = GetMutableDialogueEdgeFromParentNode();

		// This happens when we copy and paste sometimes, the parent/child nodes does not have this edge
		// Most likely caused by duplicate PinIds
		if ((ParentNodeDialogueEdge == nullptr || DialogueEdge.TargetIndex != ParentNodeDialogueEdge->TargetIndex) &&
			(!GetParentNode()->HasChildEdgeNode(this) || !GetChildNode()->HasParentEdgeNode(this)))
		{
			return;
		}

		// Node is correct but the data isn't? :O
		check(ParentNodeDialogueEdge);
		check(DialogueEdge.TargetIndex == ParentNodeDialogueEdge->TargetIndex);
		*ParentNodeDialogueEdge = DialogueEdge;
	}
}

bool UDialogueGraphNode_Edge::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	bool bWasModified = Super::Modify(bAlwaysMarkDirty);
	// Notify the Parent of this Change
	if (HasParentNode())
	{
		bWasModified = bWasModified && GetParentNode()->Modify(bAlwaysMarkDirty);
	}

	return bWasModified;
}
// End UObject interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin UEdGraphNode interface
void UDialogueGraphNode_Edge::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	GetInputPin()->bHidden = true;
	GetOutputPin()->bHidden = true;
}

FText UDialogueGraphNode_Edge::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("GetNodeTitle", "Edge Node Type");
}

FText UDialogueGraphNode_Edge::GetTooltipText() const
{
	return FText::Format(LOCTEXT("EdegeXToY", "Edge from node {0} to {1}\nText = {2}"),
						GetParentNode()->GetDialogueNodeIndex(), GetChildNode()->GetDialogueNodeIndex(),
						DialogueEdge.GetUnformattedText());
}

void UDialogueGraphNode_Edge::PinConnectionListChanged(UEdGraphPin* Pin)
{
	if (Pin->LinkedTo.Num() == 0)
	{
		// (input pin) ParentNode (output pin) -> (EdgeInputPin) ThisNode (EdgeOutputPin) -> (input pin) ChildNode (output pin)
		if (Pin->Direction == EGPD_Output)
		{
			// The other pin is input
			UEdGraphPin* InputPin = GetInputPin();

			// Try to remove the reference from the parent to this node if there is still a link.
			if (InputPin && InputPin->LinkedTo.Num() > 0)
			{
				UDialogueGraphNode* ParentNode = GetParentNode();
				const TArray<UDialogueGraphNode_Edge*> ParentChildEdgeNodes = ParentNode->GetChildEdgeNodes();
				const int32 ParentThisEdgeIndex = ParentChildEdgeNodes.Find(this);
				check(ParentThisEdgeIndex != INDEX_NONE);
				ParentNode->GetMutableDialogueNode()->RemoveChildAt(ParentThisEdgeIndex);
			}
		}

		// Commit suicide; transitions must always have an input and output connection
		Modify();
		DestroyNode();
		return;
	}

	// Modifed one of the pins, most likely a result of CONNECT_RESPONSE_BREAK_OTHERS
	const int32 NewTargetIndex = GetChildNode()->GetDialogueNodeIndex();
	if (NewTargetIndex != DialogueEdge.TargetIndex)
	{
		// (input pin) Parent Node (output pin) -> (input pin) ThisEdge Node (output pin) -> (input pin) New ChildNode (output pin)
		// Find ThisEdge node index, in the array of child edge nodes of the Parent node.
		// This matches the Edge Index of the Dialogue Edges array we must modify the Target Index of
		UDialogueGraphNode* ParentNode = GetParentNode();
		const TArray<UDialogueGraphNode_Edge*> ParentChildEdgeNodes = ParentNode->GetChildEdgeNodes();
		const int32 ParentThisEdgeIndex = ParentChildEdgeNodes.Find(this);
		check(ParentThisEdgeIndex != INDEX_NONE);

		// Change the Parent Edge Target Index
		UDlgNode* ParentNodeDialogue = ParentNode->GetMutableDialogueNode();
		check(ParentNodeDialogue->GetNodeChildren()[ParentThisEdgeIndex].TargetIndex == DialogueEdge.TargetIndex);
		ParentNodeDialogue->GetSafeMutableNodeChildAt(ParentThisEdgeIndex)->TargetIndex = NewTargetIndex;
		DialogueEdge.TargetIndex = NewTargetIndex;
	}
}

void UDialogueGraphNode_Edge::PostPasteNode()
{
	Super::PostPasteNode();
	// We don't want to paste nodes in that aren't fully linked (edges nodes have fixed pins as they
	// really describe the connection between two other nodes). If we find one missing link, get rid of the node.
	for (UEdGraphPin* Pin : Pins)
	{
		if (Pin->LinkedTo.Num() == 0)
		{
			DestroyNode();
			break;
		}
	}
}
// End UEdGraphNode interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin own functions
void UDialogueGraphNode_Edge::CreateConnections(UDialogueGraphNode* ParentNode, UDialogueGraphNode* ChildNode)
{
	check(ParentNode != ChildNode);

	// (input pin) ParentNode (output pin) -> (input pin) EdgeNode aka ThisNode (output pin) -> (input pin) ChildNode (output pin)
	UEdGraphPin* ThisInputPin = GetInputPin();
	UEdGraphPin* ThisOutputPin = GetOutputPin();
	ThisInputPin->Modify();
	ThisInputPin->LinkedTo.Empty();
	ThisOutputPin->Modify();
	ThisOutputPin->LinkedTo.Empty();

	// Previous (ParentNode) to ThisNode
	ParentNode->GetOutputPin()->MakeLinkTo(ThisInputPin);

	// ThisNode to Next (ChildNode)
	ThisOutputPin->MakeLinkTo(ChildNode->GetInputPin());

	check(ThisInputPin->LinkedTo.Num() == 1);
	check(ThisOutputPin->LinkedTo.Num() == 1);

	// Set the Edge
	const FDlgEdge* ParentNodeDialogueEdge = GetMutableDialogueEdgeFromParentNode();
	if (ParentNodeDialogueEdge != nullptr)
	{
		// Already exists
		DialogueEdge = *ParentNodeDialogueEdge;
	}
	else
	{
		// Add one Edge at the end of the array
		const FDlgEdge EdgeToAdd = FDlgEdge(ChildNode->GetDialogueNodeIndex());
		ParentNode->GetMutableDialogueNode()->AddNodeChild(EdgeToAdd);
		DialogueEdge = EdgeToAdd;
	}
}

FLinearColor UDialogueGraphNode_Edge::GetEdgeColor(bool bIsHovered) const
{
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	if (bIsHovered)
	{
		return Settings->WireHoveredColor;
	}

	// Use color scheme for primary/secondary edges
	if (Settings->bShowPrimarySecondaryEdges)
	{
		return bIsPrimaryEdge ? Settings->WirePrimaryEdgeColor : Settings->WireSecondaryEdgeColor;
	}

	// Wire has conditions
	if (HasConditions() && Settings->bShowDifferentColorForConditionWires)
	{
		return Settings->WireWithConditionsColor;
	}

	// Normal coloring
	return Settings->WireBaseColor;
}

FDlgEdge* UDialogueGraphNode_Edge::GetMutableDialogueEdgeFromParentNode() const
{
	UDialogueGraphNode* ParentNode = GetParentNode();
	const UDialogueGraphNode* ChildNode = GetChildNode();
	return ParentNode->GetMutableDialogueNode()->GetMutableNodeChildForTargetIndex(ChildNode->GetDialogueNodeIndex());
}
// End own functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
