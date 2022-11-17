// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgCompiler.h"

#include "Algo/Reverse.h"

#include "DlgSystemEditor/DlgSystemEditorModule.h"
#include "DlgSystemEditor/Editor/Graph/DialogueGraph.h"
#include "DlgSystemEditor/DlgEditorUtilities.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Root.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Edge.h"
#include "DlgSystem/Nodes/DlgNode.h"
#include "DlgSystem/DlgDialogue.h"
#include "DlgSystem/DlgHelper.h"

void FDlgCompilerContext::Compile()
{
	check(Dialogue);
	UDialogueGraph* DialogueGraph = CastChecked<UDialogueGraph>(Dialogue->GetGraph());
	DialogueGraphNodes = DialogueGraph->GetAllDialogueGraphNodes();
	if (DialogueGraphNodes.Num() == 0)
	{
		return;
	}

	ResultDialogueNodes.Empty();
	VisitedNodes.Empty();
	Queue.Empty();
	IndicesHistory.Empty();
	NodesPath.Empty();
	NextAvailableIndex = 0;

	// The code below tries to reconstruct the Dialogue Nodes from the Graph Nodes (aka compile).
	// The tricky part of the reconstructing (the Dialogue Nodes) is the node indices, because it takes
	// too much time to find the graph node with index X and add it in position X in the Dialogue.Nodes Array,
	// we simply walk the graph with a breath first search and reassign the indices as they appear.
	// This also has the advantage that close nodes will also have indices close to each other.


	// TODO(vampy): Add checking and output errors for the nodes, like orphans
	// Step 1. Find the roots and set the start nodes
	GraphNodeRoots = DialogueGraph->GetRootGraphNodes();
	check(GraphNodeRoots.Num() > 0);

	OrderRootGraphNodes();
	TArray<UDlgNode*> StartNodes;
	for (UDialogueGraphNode_Root* RootNode : GraphNodeRoots)
	{
		StartNodes.Add(RootNode->GetMutableDialogueNode());
	}
	Dialogue->SetStartNodes(StartNodes);

	// Add initially the children as we do not add the StartNode to the array of DialogueNodes
	NodeDepth = 0;
	NodesNumberUntilDepthIncrease = 1; // root/start node
	NodesNumberNextDepth = 0; // we do not know yet

	for (UDialogueGraphNode_Root* RootNode : GraphNodeRoots)
	{
		VisitedNodes.Add(RootNode);
		CompileGraphNode(RootNode);
	}

	// Step 2. Walk the graph and set the rest of the nodes.
	CompileGraph();

	// Step 3. Set nodes categorization, this ignores isolated nodes
	SetEdgesCategorization();

	// Step 4. Add orphan nodes (nodes / node group with no parents), not connected to the start node
	PruneIsolatedNodes();

	// Step 5. Update the dialogue data.
	Dialogue->EmptyNodesGUIDToIndexMap();
	Dialogue->SetNodes(ResultDialogueNodes);

	// Step 6. Fix old indices and update GUID for the Conditions
	FixBrokenOldIndicesAndUpdateGUID();

	Dialogue->PostEditChange();

	FDlgEditorUtilities::RefreshDialogueEditorForGraph(DialogueGraph);
}

void FDlgCompilerContext::OrderRootGraphNodes()
{
	// order based on position
	GraphNodeRoots.Sort([](const UDialogueGraphNode_Root& First, const UDialogueGraphNode_Root& Second)
	{
		return First.GetPosition().X < Second.GetPosition().X;
	});
}

void FDlgCompilerContext::PreCompileGraphNode(UDialogueGraphNode* GraphNode)
{
	// Sort connections/children so that they're organized the same as user can see in the editor.
	GraphNode->SortChildrenBasedOnXLocation();
	GraphNode->CheckDialogueNodeSyncWithGraphNode();
	GraphNode->SetNodeDepth(NodeDepth);
	NodeUnvisitedChildrenNum = 0;
}

void FDlgCompilerContext::PostCompileGraphNode(UDialogueGraphNode* GraphNode)
{
	GraphNode->ApplyCompilerWarnings();

	// Check symmetry, dialogue node <-> graph node
	GraphNode->CheckDialogueNodeSyncWithGraphNode(true);

	// Ensure the Node has a valid GUID
	UDlgNode* DialogueNode = GraphNode->GetMutableDialogueNode();
	if (!DialogueNode->HasGUID())
	{
		DialogueNode->RegenerateGUID();
	}

	// Update depth
	// BFS has the property that unvisited nodes in the queue all have depths that never decrease,
	// and increase by at most 1.
	--NodesNumberUntilDepthIncrease;

	// Track NodeDepth + 1 number
	NodesNumberNextDepth += NodeUnvisitedChildrenNum;
	if (NodesNumberUntilDepthIncrease == 0)
	{
		// Next depth coming up!
		NodeDepth++;
		NodesNumberUntilDepthIncrease = NodesNumberNextDepth;
		// Reset for the next dept aka NodeDepth + 1
		NodesNumberNextDepth = 0;
	}
}

void FDlgCompilerContext::CompileGraphNode(UDialogueGraphNode* GraphNode)
{
	PreCompileGraphNode(GraphNode);

	// Get the data as mutable, so what we can modify inplace
	const UDlgNode& DialogueNode = GraphNode->GetDialogueNode();
	const TArray<FDlgEdge>& NodeEdges = DialogueNode.GetNodeChildren();;

	// Walk over direct children
	// NOTE: GraphNode.OutputPin.LinkedTo are kept in sync with the Dialogue.Children
	const TArray<UDlgNode*>& DialogueNodes = Dialogue->GetNodes();
	const TArray<UDialogueGraphNode*> ChildNodes = GraphNode->GetChildNodes();

	for (int32 ChildIndex = 0, ChildrenNum = ChildNodes.Num(); ChildIndex < ChildrenNum; ChildIndex++)
	{
		// Fail on invalid edges
		check(NodeEdges[ChildIndex].IsValid());
		UDialogueGraphNode* ChildNode = ChildNodes[ChildIndex];

		// Sanity check to assume that the child node will have the same edge data from the parent
		// BEFORE TargetIndex reassignment. If this fails it means that the Dialogue Node Children are not in
		// the right order (assumption below fails).
		const int32 ChildNodeTargetIndex = NodeEdges[ChildIndex].TargetIndex;
		check(ChildNode == DialogueNodes[ChildNodeTargetIndex]->GetGraphNode())

		// Prevent double visiting nodes
		if (VisitedNodes.Contains(ChildNode))
		{
			// Node already visited, this means it has already a set index (we found a cycle)
			GraphNode->SetEdgeTargetIndexAt(ChildIndex, ChildNode->GetDialogueNodeIndex());
			continue;
		}

		// Traverse the queue
		verify(Queue.Enqueue(ChildNode));
		VisitedNodes.Add(ChildNode);

		// From This Node => Parent Node
		NodesPath.Add(ChildNode, GraphNode);
		NodeUnvisitedChildrenNum++;

		// Assume they will be added in order.
		// Parent (GraphNode) points to the new assigned ChildNode.NodeIndex
		SetNextAvailableIndexToNode(ChildNode);
		GraphNode->SetEdgeTargetIndexAt(ChildIndex, NextAvailableIndex);
		NextAvailableIndex++;
	}

	// Update the graph node/dialogue node with the new Node data (indices of children)
	PostCompileGraphNode(GraphNode);
}

void FDlgCompilerContext::CompileGraph()
{
	// Complexity O(|V| + |E|)
	// Reassign all the indices in the queue
	while (!Queue.IsEmpty())
	{
		UDialogueGraphNode* GraphNode;
		verify(Queue.Dequeue(GraphNode));

		// See children
		CompileGraphNode(GraphNode);

		// Accumulate for the Dialogue.Nodes array
		const int32 NodeAddedIndex = ResultDialogueNodes.Add(GraphNode->GetMutableDialogueNode());

		// Expect prediction to be true
		check(NodeAddedIndex == GraphNode->GetDialogueNodeIndex());
	}
}

bool FDlgCompilerContext::GetPathToNode(
	const UDialogueGraphNode* SourceNode,
	const UDialogueGraphNode* TargetNode,
	TArray<const UDialogueGraphNode*>& OutPath
)
{
	OutPath.Empty();
	OutPath.Add(TargetNode);

	// backtrack to the source node
	const UDialogueGraphNode* CurrentNode = TargetNode;
	while (CurrentNode != SourceNode)
	{
		// Find the parent of the current node in the path
		const UDialogueGraphNode** ParentNodePtr = NodesPath.Find(CurrentNode);

		// Something went wrong :(
		if (ParentNodePtr == nullptr)
		{
			return false;
		}

		// continue searching
		OutPath.Add(*ParentNodePtr);
		CurrentNode = *ParentNodePtr;
	}

	Algo::Reverse(OutPath);
	return true;
}

bool FDlgCompilerContext::GetPathToNodeAsSet(
	const UDialogueGraphNode* SourceNode,
	const UDialogueGraphNode* TargetNode,
	TSet<const UDialogueGraphNode*>& OutNodesInPath
)
{
	OutNodesInPath.Empty();
	OutNodesInPath.Add(TargetNode);

	const UDialogueGraphNode* CurrentNode = TargetNode;
	while (CurrentNode != SourceNode)
	{
		const UDialogueGraphNode** ParentNodePtr = NodesPath.Find(CurrentNode);
		if (ParentNodePtr == nullptr)
		{
			return false;
		}

		OutNodesInPath.Add(*ParentNodePtr);
		CurrentNode = *ParentNodePtr;
	}

	return true;
}

void FDlgCompilerContext::SetEdgesCategorization()
{
	// If there is an unique path from any root node to the child node (the node the edge points to) of this edge it means the
	// edge is primary, otherwise it is secondary.
	for (UDialogueGraphNode* GraphNode : VisitedNodes)
	{
		// Ignore the root nodes
		if (GraphNode->IsRootNode())
		{
			continue;
		}

		// Get existing path from all root nodes, multiple root node could lead to the same node
		TArray<TSet<const UDialogueGraphNode*>> PathListToThisNodeSet;
		for (int32 i = 0; i < GraphNodeRoots.Num(); ++i)
		{
			TSet<const UDialogueGraphNode*> Path;
			if (GetPathToNodeAsSet(GraphNodeRoots[0], GraphNode, Path))
			{
				PathListToThisNodeSet.Add(Path);
			}
		}

		// not a single root node reaches the node -> skip
		if (PathListToThisNodeSet.Num() == 0)
		{
			UE_LOG(LogDlgSystemEditor, Warning, TEXT("Can't find a path from the root node to the node with index = %d"), GraphNode->GetDialogueNodeIndex());
			continue;
		}

		// (input pin) GraphNode (output pin) -> (input pin) ChildEdgeNode (output pin) -> (input pin) ChildNode (output pin)
		for (UDialogueGraphNode_Edge* ChildEdgeNode : GraphNode->GetChildEdgeNodes())
		{
			// Unique path is determined by:
			// If the path to the parent Node of this Edge (aka PathToThisNode) does not contain the ChildNode
			// it means this path is unique (primary edge) to the ChildNode

			// if any path is unique the edge is considered to be primary
			bool bHasPrimaryPath = false;
			for (int32 i = 0; i < PathListToThisNodeSet.Num(); ++i)
			{
				if (!PathListToThisNodeSet[i].Contains(ChildEdgeNode->GetChildNode()))
				{
					bHasPrimaryPath = true;
					break;
				}
			}

			ChildEdgeNode->SetIsPrimaryEdge(bHasPrimaryPath);
		}
	}
}

void FDlgCompilerContext::PruneIsolatedNodes()
{
	// No orphans/isolated nodes
	if (DialogueGraphNodes.Num() == VisitedNodes.Num())
	{
		return;
	}

	const TSet<UDialogueGraphNode*> NodesSet(DialogueGraphNodes);
	// Get every orphan
	while (NodesSet.Num() != VisitedNodes.Num())
	{
		// Nodes that are in the graph but not in the visited nodes set
		const TSet<UDialogueGraphNode*> OrphanedNodes = NodesSet.Difference(VisitedNodes);

		// Try to find the root orphan (the one with 0 inputs)
		// This will fail  if the orphan subgraph is cyclic.
		UDialogueGraphNode* RootOrphan = nullptr;
		for (UDialogueGraphNode* GraphNode : OrphanedNodes)
		{
			if (GraphNode->GetInputPin()->LinkedTo.Num() == 0)
			{
				RootOrphan = GraphNode;
				break;
			}
		}
		// Cyclic orphan subgraph found, choose first node
		if (!IsValid(RootOrphan))
		{
			RootOrphan = CastChecked<UDialogueGraphNode>(*FDlgHelper::GetFirstSetElement(OrphanedNodes));
		}

		// Queue and assign node
		SetNextAvailableIndexToNode(RootOrphan);
		VisitedNodes.Add(RootOrphan);
		Queue.Empty();
		verify(Queue.Enqueue(RootOrphan));
		NextAvailableIndex++;
		CompileGraph();
	}
}

void FDlgCompilerContext::FixBrokenOldIndicesAndUpdateGUID()
{
	// Check if we have any modified indices
	// bool bHistoryModified = false;
	// for (auto& Elem : IndicesHistory)
	// {
	// 	if (Elem.Key != Elem.Value)
	// 	{
	// 		bHistoryModified = true;
	// 		break;
	// 	}
	// }
	// if (!bHistoryModified)
	// {
	// 	return;
	// }

	FDlgEditorUtilities::RemapOldIndicesWithNewAndUpdateGUID(DialogueGraphNodes, IndicesHistory);
}

void FDlgCompilerContext::SetNextAvailableIndexToNode(UDialogueGraphNode* GraphNode)
{
	// History is important.
	IndicesHistory.Add(GraphNode->GetDialogueNodeIndex(), NextAvailableIndex);
	GraphNode->SetDialogueNodeIndex(NextAvailableIndex);
}
