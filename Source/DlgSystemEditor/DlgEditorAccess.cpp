// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgEditorAccess.h"

#include "DlgSystem/DlgDialogue.h"
#include "Editor/Graph/DialogueGraph.h"
#include "Editor/Graph/DialogueGraphSchema.h"
#include "DlgSystemEditor/DlgEditorUtilities.h"
#include "Editor/Nodes/DialogueGraphNode.h"
#include "Editor/Nodes/DialogueGraphNode_Edge.h"
#include "Editor/DlgCompiler.h"
#include "DlgSystem/Nodes/DlgNode.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgEditorAccess
void FDlgEditorAccess::UpdateGraphNodeEdges(UEdGraphNode* GraphNode)
{
	if (UDialogueGraphNode* DialogueGraphNode = Cast<UDialogueGraphNode>(GraphNode))
	{
		DialogueGraphNode->UpdateEdgesFromDialogueNode();
	}
}

UEdGraph* FDlgEditorAccess::CreateNewDialogueGraph(UDlgDialogue* Dialogue) const
{
	UDialogueGraph* DialogueGraph = CastChecked<UDialogueGraph>(FDlgEditorUtilities::CreateNewGraph(Dialogue, NAME_None,
										UDialogueGraph::StaticClass(), UDialogueGraphSchema::StaticClass()));
	DialogueGraph->bAllowDeletion = false;

	return CastChecked<UEdGraph>(DialogueGraph);
}

/** Compiles the dialogue nodes from the graph nodes. Meaning it transforms the graph data -> (into) dialogue data. */
void FDlgEditorAccess::CompileDialogueNodesFromGraphNodes(UDlgDialogue* Dialogue) const
{
	FCompilerResultsLog MessageLog;
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	FDlgCompilerContext CompilerContext(Dialogue, Settings, MessageLog);
	CompilerContext.Compile();
	//FDlgEditorUtilities::RefreshDetailsView(Dialogue->GetGraph(), true);
}

/** Removes all nodes from the graph. */
void FDlgEditorAccess::RemoveAllGraphNodes(UDlgDialogue* Dialogue) const
{
	CastChecked<UDialogueGraph>(Dialogue->GetGraph())->RemoveAllNodes();

	// Clear the references from the Dialogue Nodes
	for (UDlgNode* Node : Dialogue->GetMutableStartNodes())
	{
		Node->ClearGraphNode();
	}
	for (UDlgNode* Node : Dialogue->GetNodes())
	{
		Node->ClearGraphNode();
	}
}

/** Updates the Dialogue to match the version UseOnlyOneOutputAndInputPin */
void FDlgEditorAccess::UpdateDialogueToVersion_UseOnlyOneOutputAndInputPin(UDlgDialogue* Dialogue) const
{
	Dialogue->DisableCompileDialogue();
	UDialogueGraph* DialogueGraph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	const TArray<UDialogueGraphNode*> DialogueGraphNodes = DialogueGraph->GetAllDialogueGraphNodes();

	// Array that maps from Index (in DialogueGraphNodes) => Array of old pins
	TArray<TArray<UEdGraphPin*>> OldPinsMap;

	// Step 1. Copy all of the pins from the graph nodes, and create new input/output pin
	for (int32 GraphNodeIndex = 0, GraphNodesNum = DialogueGraphNodes.Num(); GraphNodeIndex < GraphNodesNum; GraphNodeIndex++)
	{
		UDialogueGraphNode* GraphNode = DialogueGraphNodes[GraphNodeIndex];
		// Assume we do not have any invalid edges
		check(GraphNode->GetDialogueNode().GetNodeOpenChildren_DEPRECATED().Num() == 0)
		OldPinsMap.Add(GraphNode->Pins);
		GraphNode->Pins.Empty();
		// Creates the new input and output pin
		GraphNode->AllocateDefaultPins();

		// Clean comment if it is only a digit, from this version there is a nice widget overlay that shows the index ;)
		if (GraphNode->NodeComment.TrimStartAndEnd().IsNumeric() || GraphNode->IsRootNode())
		{
			GraphNode->NodeComment.Empty();
			GraphNode->bCommentBubbleVisible = false;
		}
	}

	// Step 2. Create new links
	for (int32 GraphNodeIndex = 0, GraphNodesNum = DialogueGraphNodes.Num(); GraphNodeIndex < GraphNodesNum; GraphNodeIndex++)
	{
		UDialogueGraphNode* GraphNode = DialogueGraphNodes[GraphNodeIndex];

		// Convert
		for (UEdGraphPin* OldPin : OldPinsMap[GraphNodeIndex])
		{
			if (OldPin->LinkedTo.Num() == 0 || OldPin->Direction == EGPD_Input)
			{
				continue;
			}

			check(OldPin->LinkedTo.Num() == 1);
			check(OldPin->Direction == EGPD_Output);

			UDialogueGraphNode* ConnectedNode = CastChecked<UDialogueGraphNode>(OldPin->LinkedTo[0]->GetOwningNode());
			// Make a proxy (selectable) edge graph node
			UDialogueGraphNode_Edge* GraphNode_Edge =
				FDlgEditorUtilities::SpawnGraphNodeFromTemplate<UDialogueGraphNode_Edge>(
					DialogueGraph, GraphNode->GetDefaultEdgePosition(), false
				);
			GraphNode_Edge->CreateConnections(GraphNode, ConnectedNode);
		}
		GraphNode->CheckAll();
	}
	DialogueGraph->AutoPositionGraphNodes();

	// Cleanup
	for (TArray<UEdGraphPin*> OldPins : OldPinsMap)
	{
		for (UEdGraphPin* OldPin : OldPins)
		{
			UDialogueGraphNode::DestroyPin(OldPin);
		}
	}

	OldPinsMap.Empty();
	Dialogue->EnableCompileDialogue();
	Dialogue->MarkPackageDirty();
}


void FDlgEditorAccess::SetNewOuterForObjectFromGraphNode(UObject* Object, UEdGraphNode* GraphNode) const
{
	if (!Object || !GraphNode)
	{
		return;
	}

	UDlgNode* ClosestNode = FDlgEditorUtilities::GetClosestNodeFromGraphNode(GraphNode);
	if (!ClosestNode)
	{
		return;
	}

	Object->Rename(nullptr, ClosestNode, REN_DontCreateRedirectors);
}
