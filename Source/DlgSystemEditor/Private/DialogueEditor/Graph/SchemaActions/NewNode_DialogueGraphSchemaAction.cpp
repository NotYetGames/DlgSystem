// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "NewNode_DialogueGraphSchemaAction.h"

#include "ScopedTransaction.h"

#include "DlgDialogue.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"

#define LOCTEXT_NAMESPACE "NewNode_DialogueGraphSchemaAction"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FNewNode_DialogueGraphSchemaAction
UEdGraphNode* FNewNode_DialogueGraphSchemaAction::PerformAction(
	UEdGraph* ParentGraph,
	UEdGraphPin* FromPin,
	const FVector2D Location,
	bool bSelectNewNode/* = true*/
)
{
	const FScopedTransaction Transaction(LOCTEXT("DialogueditorNewDialgueNode", "Dialogue Editor: New Dialogue Node"));
	UDlgDialogue* Dialogue = CastChecked<UDialogueGraph>(ParentGraph)->GetDialogue();

	// Mark for modification
	verify(ParentGraph->Modify());
	if (FromPin)
	{
		verify(FromPin->Modify());
	}
	verify(Dialogue->Modify());

	// Create node, without needing to compile it
	UEdGraphNode* GraphNode = CreateNode(Dialogue, ParentGraph, FromPin, Location, bSelectNewNode);
	Dialogue->PostEditChange();
	Dialogue->MarkPackageDirty();
	ParentGraph->NotifyGraphChanged();

	return GraphNode;
}

UEdGraphNode* FNewNode_DialogueGraphSchemaAction::CreateNode(
	UDlgDialogue* Dialogue,
	UEdGraph* ParentGraph,
	UEdGraphPin* FromPin,
	FVector2D Location,
	bool bSelectNewNode
)
{
	// Maximum distance a drag can be off a node edge to require 'push off' from node
	static constexpr int32 NodeDistance = 60;

	// Create the dialogue node
	auto DialogueNode = Dialogue->ConstructDialogueNode<UDlgNode>(CreateNodeType);

	// Set the Participant Name
	if (FromPin)
	{
		if (UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(FromPin->GetOwningNode()))
		{
			if (GraphNode->IsRootNode())
			{
				// Root Node use the first node from the Array
				if (Dialogue->GetNodes().Num() > 0)
				{
					DialogueNode->SetNodeParticipantName(Dialogue->GetNodes()[0]->GetNodeParticipantName());
				}
			}
			else
			{
				// Use the node from which we spawned this
				DialogueNode->SetNodeParticipantName(GraphNode->GetDialogueNode().GetNodeParticipantName());
			}
		}
	}
	else
	{
		// Use first Node from the Array
		if (Dialogue->GetNodes().Num() > 0)
		{
			DialogueNode->SetNodeParticipantName(Dialogue->GetNodes()[0]->GetNodeParticipantName());
		}
	}

	// Create the graph node
	FGraphNodeCreator<UDialogueGraphNode> NodeCreator(*ParentGraph);
	UDialogueGraphNode* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);

	// Link dialogue node <-> graph node
	DialogueNode->SetGraphNode(GraphNode);
	const int32 DialogueNodeIndex = Dialogue->AddNode(DialogueNode);
	GraphNode->SetDialogueNodeDataChecked(DialogueNodeIndex, DialogueNode);

	// Finalize graph node creation
	NodeCreator.Finalize(); // Calls on the node: CreateNewGuid, PostPlacedNewNode, AllocateDefaultPins
	GraphNode->AutowireNewNode(FromPin);

	// Position graph node
	// For input pins, new node will generally overlap node being dragged off
	// Work out if we want to visually push away from connected node
	int32 XLocation = Location.X;
	if (FromPin && FromPin->Direction == EGPD_Input)
	{
		UEdGraphNode* PinNode = FromPin->GetOwningNode();
		const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);

		if (XDelta < NodeDistance)
		{
			// Set location to edge of current node minus the max move distance
			// to force node to push off from connect node enough to give selection handle
			XLocation = PinNode->NodePosX - NodeDistance;
		}
	}

	GraphNode->SetPosition(XLocation, Location.Y);
	//ResultNode->SnapToGrid(SNAP_GRID);

	return CastChecked<UEdGraphNode>(GraphNode);
}

#undef LOCTEXT_NAMESPACE
