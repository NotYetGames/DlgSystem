// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgDialogueEditorAccess.h"

#include "DlgDialogue.h"
#include "DialogueEditor/Graph/DialogueGraph.h"
#include "DialogueEditor/Graph/DialogueGraphSchema.h"
#include "DialogueEditorUtilities.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueEditor/DialogueCompiler.h"
#include "Nodes/DlgNode.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgDialogueEditorAccess
void FDlgDialogueEditorAccess::UpdateGraphNodeEdges(UEdGraphNode* GraphNode)
{
	if (UDialogueGraphNode* DialogueGraphNode = Cast<UDialogueGraphNode>(GraphNode))
	{
		DialogueGraphNode->UpdateEdgesFromDialogueNode();
	}
}

UEdGraph* FDlgDialogueEditorAccess::CreateNewDialogueGraph(UDlgDialogue* Dialogue) const
{
	UDialogueGraph* DialogueGraph = CastChecked<UDialogueGraph>(FDialogueEditorUtilities::CreateNewGraph(Dialogue, NAME_None,
										UDialogueGraph::StaticClass(), UDialogueGraphSchema::StaticClass()));
	DialogueGraph->bAllowDeletion = false;

	return CastChecked<UEdGraph>(DialogueGraph);
}

/** Compiles the dialogue nodes from the graph nodes. Meaning it transforms the graph data -> (into) dialogue data. */
void FDlgDialogueEditorAccess::CompileDialogueNodesFromGraphNodes(UDlgDialogue* Dialogue) const
{
	FCompilerResultsLog MessageLog;
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	FDialogueCompilerContext CompilerContext(Dialogue, Settings, MessageLog);
	CompilerContext.Compile();
	//FDialogueEditorUtilities::RefreshDetailsView(Dialogue->GetGraph(), true);
}

/** Removes all nodes from the graph. */
void FDlgDialogueEditorAccess::RemoveAllGraphNodes(UDlgDialogue* Dialogue) const
{
	CastChecked<UDialogueGraph>(Dialogue->GetGraph())->RemoveAllNodes();

	// Clear the references from the Dialogue Nodes
	Dialogue->GetMutableStartNode()->ClearGraphNode();
	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();
	const int32 NodesNum = Nodes.Num();
	for (int32 NodeIndex = 0; NodeIndex < NodesNum; NodeIndex++)
	{
		Nodes[NodeIndex]->ClearGraphNode();
	}
}

/** Updates the Dialogue to match the version UseOnlyOneOutputAndInputPin */
void FDlgDialogueEditorAccess::UpdateDialogueToVersion_UseOnlyOneOutputAndInputPin(UDlgDialogue* Dialogue) const
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
				FDialogueEditorUtilities::SpawnGraphNodeFromTemplate<UDialogueGraphNode_Edge>(
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


void FDlgDialogueEditorAccess::SetNewOuterForObjectFromGraphNode(UObject* Object, UEdGraphNode* GraphNode) const
{
	if (!Object || !GraphNode)
	{
		return;
	}

	UDlgNode* ClosestNode = FDialogueEditorUtilities::GetClosestNodeFromGraphNode(GraphNode);
	if (!ClosestNode)
	{
		return;
	}

	Object->Rename(nullptr, ClosestNode, REN_DontCreateRedirectors);
}
