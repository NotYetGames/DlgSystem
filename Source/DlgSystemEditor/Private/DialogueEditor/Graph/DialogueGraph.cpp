// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueGraph.h"

#include "GraphEditAction.h"

#include "DlgDialogue.h"
#include "DialogueGraphSchema.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Root.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueEditor/DialogueCompiler.h"
#include "DlgDialogueEditorAccess.h"

UDialogueGraph::UDialogueGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set the static editor module interface used by all the dialogues in the DlgSystem module to communicate with the editor.
	if (!UDlgDialogue::GetDialogueEditorAccess().IsValid())
	{
		UDlgDialogue::SetDialogueEditorAccess(TSharedPtr<IDlgDialogueEditorAccess>(new FDlgDialogueEditorAccess));
	}
}

bool UDialogueGraph::Modify(bool bAlwaysMarkDirty)
{
	if (!CanModify())
	{
		return false;
	}

	bool bWasSaved = Super::Modify(bAlwaysMarkDirty);
	// Transactions do not support arrays?
	// See https://answers.unrealengine.com/questions/674286/how-to-undoredo-a-modification-to-an-array.html?sort=oldest
	// TODO check out why it does not save the arrays, because it uses the standard serializer that also writes to .uasset

	// Mark all nodes for modification
	// question of space (save them all here) or recompile them after every undo
	for (UDialogueGraphNode_Base* BaseNode : GetAllBaseDialogueGraphNodes())
	{
		bWasSaved = bWasSaved && BaseNode->Modify(bAlwaysMarkDirty);
	}

	return bWasSaved;
}

UDialogueGraphNode_Root* UDialogueGraph::GetRootGraphNode() const
{
	TArray<UDialogueGraphNode_Root*> RootNodeList;
	GetNodesOfClass<UDialogueGraphNode_Root>(/*out*/ RootNodeList);
	check(RootNodeList.Num() == 1);
	return RootNodeList[0];
}

TArray<UDialogueGraphNode_Base*> UDialogueGraph::GetAllBaseDialogueGraphNodes() const
{
	TArray<UDialogueGraphNode_Base*> AllBaseDialogueGraphNodes;
	GetNodesOfClass<UDialogueGraphNode_Base>(/*out*/ AllBaseDialogueGraphNodes);
	return AllBaseDialogueGraphNodes;
}

TArray<UDialogueGraphNode*> UDialogueGraph::GetAllDialogueGraphNodes() const
{
	TArray<UDialogueGraphNode*> AllDialogueGraphNodes;
	GetNodesOfClass<UDialogueGraphNode>(/*out*/ AllDialogueGraphNodes);
	return AllDialogueGraphNodes;
}

TArray<UDialogueGraphNode_Edge*> UDialogueGraph::GetAllEdgeDialogueGraphNodes() const
{
	TArray<UDialogueGraphNode_Edge*> AllEdgeDialogueGraphNodes;
	GetNodesOfClass<UDialogueGraphNode_Edge>(/*out*/ AllEdgeDialogueGraphNodes);
	return AllEdgeDialogueGraphNodes;
}

bool UDialogueGraph::RemoveGraphNode(UEdGraphNode* NodeToRemove)
{
	Modify();
	const int32 NumTimesNodeRemoved = Nodes.Remove(NodeToRemove);

	// This will trigger the compile in the UDialogueGraphSchema::BreakNodeLinks
	// NOTE: do not call BreakAllNodeLinks on the node as it does not register properly with the
	// undo system
	GetSchema()->BreakNodeLinks(*NodeToRemove);

	// Notify
	FEdGraphEditAction RemovalAction;
	RemovalAction.Graph = this;
	RemovalAction.Action = GRAPHACTION_RemoveNode;
	RemovalAction.Nodes.Add(NodeToRemove);
	NotifyGraphChanged(RemovalAction);

	return NumTimesNodeRemoved > 0;
}

void UDialogueGraph::CreateGraphNodesFromDialogue()
{
	// Assume empty graph
	check(Nodes.Num() == 0);
	UDlgDialogue* Dialogue = GetDialogue();
	FDialogueEditorUtilities::CheckAndTryToFixDialogue(Dialogue, false);

	// Step 1: Create the root (start) node
	{
		FGraphNodeCreator<UDialogueGraphNode_Root> NodeCreator(*this);
		UDialogueGraphNode_Root* StartGraphNode = NodeCreator.CreateNode();

		// Create two way direction for both Dialogue Node and Graph Node.
		UDlgNode* StartNode = Dialogue->GetMutableStartNode();
		check(StartNode);

		StartGraphNode->SetDialogueNode(StartNode);
		Dialogue->SetStartNode(StartNode);

		// Finalize creation
		StartGraphNode->SetPosition(0, 0);
		NodeCreator.Finalize();
	}

	// Step 2: Create the Graph Nodes for all the Nodes
	const TArray<UDlgNode*>& DialogueNodes = Dialogue->GetNodes();
	const int32 NodesNum = DialogueNodes.Num();
	for (int32 NodeIndex = 0; NodeIndex < NodesNum; NodeIndex++)
	{
		FGraphNodeCreator<UDialogueGraphNode> NodeCreator(*this);
		UDialogueGraphNode* GraphNode = NodeCreator.CreateNode();

		// Create two way direction for both Dialogue Node and Graph Node.
		GraphNode->SetDialogueNodeDataChecked(NodeIndex, DialogueNodes[NodeIndex]);

		// Finalize creation
		GraphNode->SetPosition(0, 0);
		NodeCreator.Finalize();
	}
}

void UDialogueGraph::LinkGraphNodesFromDialogue() const
{
	UDialogueGraphNode_Root* StartNodeGraph = GetRootGraphNode();
	UDlgDialogue* Dialogue = GetDialogue();

	// Assume we have all the nodes created (plus the start node)
	check(FDialogueEditorUtilities::AreDialogueNodesInSyncWithGraphNodes(Dialogue));

	const UDlgNode& StartNodeDialogue = Dialogue->GetStartNode();
	const TArray<UDlgNode*>& NodesDialogue = Dialogue->GetNodes();
	// Step 1. Make the root (start) node connections
	LinkGraphNodeToChildren(NodesDialogue, StartNodeDialogue, StartNodeGraph);

	// Step 2: Create all the connections between the rest of the nodes
	for (UDlgNode* DialogueNode : NodesDialogue)
	{
		LinkGraphNodeToChildren(NodesDialogue, *DialogueNode, CastChecked<UDialogueGraphNode>(DialogueNode->GetGraphNode()));
	}
}

void UDialogueGraph::LinkGraphNodeToChildren(
	const TArray<UDlgNode*>& NodesDialogue,
	const UDlgNode& NodeDialogue,
	UDialogueGraphNode* GraphNode
) const
{
	// Assume we are starting from scratch, no output connections
	GraphNode->GetOutputPin()->BreakAllPinLinks();

//	UEdGraphPin* OutputPin = GraphNode->GetOutputPin();
	const TArray<FDlgEdge>& NodeEdges = NodeDialogue.GetNodeChildren();
	TSet<int32> NodeSeenEdges;
	for (int32 ChildIndex = 0, ChildNum = NodeEdges.Num(); ChildIndex < ChildNum; ChildIndex++)
	{
		const int32 TargetIndex = NodeEdges[ChildIndex].TargetIndex;
		// Ignore invalid edges
		if (TargetIndex == INDEX_NONE)
		{
			continue;
		}

		// Prevent two edges to the same node.
		if (NodeSeenEdges.Contains(TargetIndex))
		{
			continue;
		}

		// Get the child node and make sure it has the required number of inputs
		const UDlgNode& ChildNode = *NodesDialogue[TargetIndex];
		UDialogueGraphNode* ChildGraphNode = CastChecked<UDialogueGraphNode>(ChildNode.GetGraphNode());

		// Make connection
		UDialogueGraphNode_Edge* GraphNode_Edge =
			FDialogueEditorUtilities::SpawnGraphNodeFromTemplate<UDialogueGraphNode_Edge>(
				GraphNode->GetGraph(), GraphNode->GetDefaultEdgePosition(), false
			);

		// Create proxy connection from output -> input
		GraphNode_Edge->CreateConnections(GraphNode, ChildGraphNode);
		NodeSeenEdges.Add(TargetIndex);
	}
	GraphNode->CheckAll();
	GraphNode->ApplyCompilerWarnings();
}

void UDialogueGraph::AutoPositionGraphNodes() const
{
	static constexpr bool bIsDirectionVertical = true;
	UDialogueGraphNode_Root* RootNode = GetRootGraphNode();
	const TArray<UDialogueGraphNode*> DialogueGraphNodes = GetAllDialogueGraphNodes();
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();

	// TODO investigate Node->SnapToGrid
	FDialogueEditorUtilities::AutoPositionGraphNodes(
		RootNode,
		DialogueGraphNodes,
		Settings->OffsetBetweenColumnsX,
		Settings->OffsetBetweenRowsY,
		bIsDirectionVertical
	);
}

void UDialogueGraph::RemoveAllNodes()
{
	Modify();

	// Could have used RemoveNode on each node but that is unecessary as that is slow and notifies external objects
	Nodes.Empty();
	check(Nodes.Num() == 0);
}

const UDialogueGraphSchema* UDialogueGraph::GetDialogueGraphSchema() const
{
	return GetDefault<UDialogueGraphSchema>(Schema);
}
