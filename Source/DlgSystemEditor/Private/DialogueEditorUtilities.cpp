// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueEditorUtilities.h"

#include "Toolkits/IToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "Templates/Casts.h"
#include "Containers/Queue.h"
#include "EdGraphNode_Comment.h"
#include "FileHelpers.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/SClassPickerDialog.h"

#include "DlgSystemEditorModule.h"
#include "DialogueEditor/IDialogueEditor.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DlgHelper.h"
#include "DlgManager.h"
#include "Factories/DialogueClassViewerFilters.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node_Event.h"

/** Useful for auto positioning */
struct NodeWithParentPosition
{
	NodeWithParentPosition() {}
	NodeWithParentPosition(UDialogueGraphNode* InNode, const int32 InParentNodeX, const int32 InParentNodeY) :
		Node(InNode), ParentNodeX(InParentNodeX), ParentNodeY(InParentNodeY) {}

	UDialogueGraphNode* Node = nullptr;
	int32 ParentNodeX = 0;
	int32 ParentNodeY = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueEditorUtilities
void FDialogueEditorUtilities::LoadAllDialoguesAndCheckGUIDs()
{
	//const int32 NumDialoguesBefore = UDlgManager::GetAllDialoguesFromMemory().Num();
	const int32 NumLoadedDialogues = UDlgManager::LoadAllDialoguesIntoMemory(false);
	//const int32 NumDialoguesAfter = UDlgManager::GetAllDialoguesFromMemory().Num();
	//check(NumDialoguesBefore == NumDialoguesAfter);
	UE_LOG(LogDlgSystemEditor, Log, TEXT("UDlgManager::LoadAllDialoguesIntoMemory loaded %d Dialogues into Memory"), NumLoadedDialogues);

	// Try to fix duplicate GUID
	// Can happen for one of the following reasons:
	// - duplicated files outside of UE
	// - somehow loaded from text files?
	// - the universe hates us? +_+
	for (UDlgDialogue* Dialogue : UDlgManager::GetDialoguesWithDuplicateGUIDs())
	{
		UE_LOG(
			LogDlgSystemEditor,
			Warning,
			TEXT("Dialogue = `%s`, GUID = `%s` has a Duplicate GUID. Regenerating."),
			*Dialogue->GetPathName(), *Dialogue->GetGUID().ToString()
		)
		Dialogue->RegenerateGUID();
		Dialogue->MarkPackageDirty();
	}

	// Give it another try, Give up :((
	// May the math Gods have mercy on us!
	for (const UDlgDialogue* Dialogue : UDlgManager::GetDialoguesWithDuplicateGUIDs())
	{
		// GUID already exists (╯°□°）╯︵ ┻━┻
		// Does this break the universe?
		UE_LOG(
			LogDlgSystemEditor,
			Error,
			TEXT("Dialogue = `%s`, GUID = `%s`"),
			*Dialogue->GetPathName(), *Dialogue->GetGUID().ToString()
		)

		UE_LOG(
			LogDlgSystemEditor,
			Fatal,
			TEXT("(╯°□°）╯︵ ┻━┻ Congrats, you just broke the universe, are you even human? Now please go and proove an NP complete problem."
				"The chance of generating two equal random FGuid (picking 4, uint32 numbers) is p = 9.3132257 * 10^(-10) % (or something like this)")
		)
	}
}

const TSet<UObject*> FDialogueEditorUtilities::GetSelectedNodes(const UEdGraph* Graph)
{
	TSharedPtr<IDialogueEditor> DialogueEditor = GetDialogueEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		return DialogueEditor->GetSelectedNodes();
	}

	return {};
}

bool FDialogueEditorUtilities::GetBoundsForSelectedNodes(const UEdGraph* Graph, class FSlateRect& Rect, float Padding)
{
	TSharedPtr<IDialogueEditor> DialogueEditor = GetDialogueEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		return DialogueEditor->GetBoundsForSelectedNodes(Rect, Padding);
	}

	return false;
}

void FDialogueEditorUtilities::RefreshDetailsView(const UEdGraph* Graph, bool bRestorePreviousSelection)
{
	TSharedPtr<IDialogueEditor> DialogueEditor = GetDialogueEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		DialogueEditor->RefreshDetailsView(bRestorePreviousSelection);
	}
}

void FDialogueEditorUtilities::Refresh(const UEdGraph* Graph, bool bRestorePreviousSelection)
{
	TSharedPtr<IDialogueEditor> DialogueEditor = GetDialogueEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		DialogueEditor->Refresh(bRestorePreviousSelection);
	}
}

UDialogueGraphNode_Edge* FDialogueEditorUtilities::GetLastTargetGraphEdgeBeforeDrag(const UEdGraph* Graph)
{
	TSharedPtr<IDialogueEditor> DialogueEditor = GetDialogueEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		return DialogueEditor->GetLastTargetGraphEdgeBeforeDrag();
	}

	return nullptr;
}

void FDialogueEditorUtilities::SetLastTargetGraphEdgeBeforeDrag(const UEdGraph* Graph, UDialogueGraphNode_Edge* InEdge)
{
	TSharedPtr<IDialogueEditor> DialogueEditor = GetDialogueEditorForGraph(Graph);
	if (DialogueEditor.IsValid())
	{
		DialogueEditor->SetLastTargetGraphEdgeBeforeDrag(InEdge);
	}
}

TSharedPtr<class IDialogueEditor> FDialogueEditorUtilities::GetDialogueEditorForGraph(const UEdGraph* Graph)
{
	// Find the associated Dialogue
	const UDlgDialogue* Dialogue = GetDialogueForGraph(Graph);
	TSharedPtr<IDialogueEditor> DialogueEditor;

	// This Dialogue has already an asset editor opened
	TSharedPtr<IToolkit> FoundAssetEditor = FToolkitManager::Get().FindEditorForAsset(Dialogue);
	if (FoundAssetEditor.IsValid())
	{
		DialogueEditor = StaticCastSharedPtr<IDialogueEditor>(FoundAssetEditor);
	}

	return DialogueEditor;
}

bool FDialogueEditorUtilities::RemoveNode(UEdGraphNode* NodeToRemove)
{
	if (!IsValid(NodeToRemove))
	{
		return false;
	}

	UDialogueGraph* Graph = CastChecked<UDialogueGraph>(NodeToRemove->GetGraph());
	if (!IsValid(Graph))
	{
		return false;
	}

	// Transactions should be declared in the code that calls this method
	if (!Graph->Modify())
	{
		UE_LOG(LogDlgSystemEditor, Fatal, TEXT("FDialogueEditorUtilities::RemoveNode No transaction was declared before calling this method, aborting!"));
		return false;
	}
	if (!NodeToRemove->Modify())
	{
		UE_LOG(LogDlgSystemEditor, Fatal, TEXT("FDialogueEditorUtilities::RemoveNode No transaction was declared before calling this method, aborting!"));
		return false;
	}

	return Graph->RemoveGraphNode(NodeToRemove);
}

UEdGraph* FDialogueEditorUtilities::CreateNewGraph(
	UObject* ParentScope,
	FName GraphName,
	TSubclassOf<UEdGraph> GraphClass,
	TSubclassOf<UEdGraphSchema> SchemaClass
)
{
	// Mostly copied from FBlueprintEditorUtils::CreateNewGraph
	UEdGraph* NewGraph;
	bool bRename = false;

	// Ensure this name isn't already being used for a graph
	if (GraphName != NAME_None)
	{
		UEdGraph* ExistingGraph = FindObject<UEdGraph>(ParentScope, *(GraphName.ToString()));
		ensureMsgf(!ExistingGraph, TEXT("Graph %s already exists: %s"), *GraphName.ToString(), *ExistingGraph->GetFullName());

		// Rename the old graph out of the way; but we have already failed at this point
		if (ExistingGraph)
		{
			ExistingGraph->Rename(nullptr, ExistingGraph->GetOuter(), REN_DoNotDirty | REN_ForceNoResetLoaders);
		}

		// Construct new graph with the supplied name
		NewGraph = NewObject<UEdGraph>(ParentScope, GraphClass, NAME_None, RF_Transactional);
		bRename = true;
	}
	else
	{
		// Construct a new graph with a default name
		NewGraph = NewObject<UEdGraph>(ParentScope, GraphClass, NAME_None, RF_Transactional);
	}

	NewGraph->Schema = SchemaClass;

	// Now move to where we want it to. Workaround to ensure transaction buffer is correctly utilized
	if (bRename)
	{
		NewGraph->Rename(*GraphName.ToString(), ParentScope, REN_DoNotDirty | REN_ForceNoResetLoaders);
	}

	return NewGraph;
}

bool FDialogueEditorUtilities::CheckAndTryToFixDialogue(UDlgDialogue* Dialogue, bool bDisplayWarning)
{
	bool bIsDataValid = true;
#if DO_CHECK
	const TArray<UDlgNode*>& DialogueNodes = Dialogue->GetNodes();
	// Do some additional checks to ensure the data is safe, useful in development
	auto checkIfMultipleEdgesToSameNode = [DialogueNodes, bDisplayWarning](UDlgNode* Node)
	{
		if (!IsValid(Node))
		{
			return true;
		}

		TSet<int32> NodeEdgesFound;
		TSet<int32> EdgesToRemove;
		// Find the duplicate edges
		const TArray<FDlgEdge>& NodeChildren = Node->GetNodeChildren();
		for (int32 EdgeIndex = 0, EdgesNum = NodeChildren.Num(); EdgeIndex < EdgesNum; EdgeIndex++)
		{
			const FDlgEdge& Edge = NodeChildren[EdgeIndex];
			if (Edge.TargetIndex == INDEX_NONE)
			{
				continue;
			}

			if (NodeEdgesFound.Contains(Edge.TargetIndex))
			{
				// Mark for deletion
				EdgesToRemove.Add(EdgeIndex);

				if (!bDisplayWarning)
				{
					continue;
				}

				// Find source and destination
				const int32 IndexToNode = Edge.TargetIndex;
				int32 IndexFromNode = DialogueNodes.Find(Node);
				if (IndexFromNode == INDEX_NONE) // start node
				{
					IndexFromNode = -1;
				}

				const FString Message = FString::Printf(
					TEXT("Node with index = `%d` connects multiple times to destination Node with index = `%d`. One of the Edges will be removed."),
					IndexFromNode, IndexToNode);
				ShowMessageBox(EAppMsgType::Ok, Message, TEXT("Invalid Dialogue data"));
			}
			else
			{
				NodeEdgesFound.Add(Edge.TargetIndex);
			}
		}

		// Remove if any duplicate edges
		for (int32 EdgeIndex : EdgesToRemove)
		{
			Node->RemoveChildAt(EdgeIndex);
		}

		return EdgesToRemove.Num() == 0;
	};

	bIsDataValid = bIsDataValid && checkIfMultipleEdgesToSameNode(Dialogue->GetMutableStartNode());
	for (UDlgNode* Node : DialogueNodes)
	{
		bIsDataValid = bIsDataValid && checkIfMultipleEdgesToSameNode(Node);
	}

#endif

	return bIsDataValid;
}

void FDialogueEditorUtilities::TryToCreateDefaultGraph(UDlgDialogue* Dialogue, bool bPrompt)
{
	// Clear the graph if the number of nodes differ
	if (AreDialogueNodesInSyncWithGraphNodes(Dialogue))
	{
		return;
	}

	// Simply do the operations without any consent
	if (!bPrompt)
	{
		// Always keep in sync with the .dlg (text file).
		Dialogue->InitialSyncWithTextFile();
		CheckAndTryToFixDialogue(Dialogue);
		Dialogue->ClearGraph();
		return;
	}

	// Prompt to the user to initial sync with the text file
	{
		const EAppReturnType::Type Response = ShowMessageBox(EAppMsgType::YesNo,
			FString::Printf(TEXT("Initial sync the Dialogues nodes of `%s` from the text file with the same name?"), *Dialogue->GetName()),
			TEXT("Get Dialogue nodes from the text file"));

		if (Response == EAppReturnType::Yes)
		{
			Dialogue->InitialSyncWithTextFile();
		}
	}
	CheckAndTryToFixDialogue(Dialogue);

	// Prompt the user and only if he answers yes we clear the graph
	{
		const int32 NumGraphNodes = CastChecked<UDialogueGraph>(Dialogue->GetGraph())->GetAllDialogueGraphNodes().Num();
		const int32 NumDialogueNodes = Dialogue->GetNodes().Num() + 1; // (plus the start node)
		const FString Message = FString::Printf(TEXT("Dialogue with name = `%s` has number of graph nodes (%d) != number dialogue nodes (%d)."),
			*Dialogue->GetName(), NumGraphNodes, NumDialogueNodes);
		const EAppReturnType::Type Response = ShowMessageBox(EAppMsgType::YesNo,
			FString::Printf(TEXT("%s%s"), *Message, TEXT("\nWould you like to autogenerate the graph nodes from the dialogue nodes?\n WARNING: Graph nodes will be lost")),
			TEXT("Autogenerate graph nodes from dialogue nodes?"));

		// This will trigger the CreateDefaultNodesForGraph in the the GraphSchema
		if (Response == EAppReturnType::Yes)
		{
			Dialogue->ClearGraph();
		}
	}
}

bool FDialogueEditorUtilities::AreDialogueNodesInSyncWithGraphNodes(const UDlgDialogue* Dialogue)
{
	const int32 NumGraphNodes = CastChecked<UDialogueGraph>(Dialogue->GetGraph())->GetAllDialogueGraphNodes().Num();
	const int32 NumDialogueNodes = Dialogue->GetNodes().Num() + 1; // (plus the start node)
	if (NumGraphNodes == NumDialogueNodes)
	{
		return true;
	}

	return false;
}

UDlgNode* FDialogueEditorUtilities::GetClosestNodeFromGraphNode(UEdGraphNode* GraphNode)
{
	const UDialogueGraphNode_Base* BaseNode = Cast<UDialogueGraphNode_Base>(GraphNode);
	if (!BaseNode)
	{
		return nullptr;
	}

	// Node
	if (const UDialogueGraphNode* Node = Cast<UDialogueGraphNode>(BaseNode))
	{
		return Node->GetMutableDialogueNode();
	}

	// Edge
	if (const UDialogueGraphNode_Edge* EdgeNode = Cast<UDialogueGraphNode_Edge>(BaseNode))
	{
		if (EdgeNode->HasParentNode())
			return EdgeNode->GetParentNode()->GetMutableDialogueNode();
		if (EdgeNode->HasChildNode())
			return EdgeNode->GetChildNode()->GetMutableDialogueNode();
	}

	return nullptr;
}

void FDialogueEditorUtilities::AutoPositionGraphNodes(
	UDialogueGraphNode* RootNode,
	const TArray<UDialogueGraphNode*>& GraphNodes,
	int32 OffsetBetweenColumnsX,
	int32 OffsetBetweenRowsY,
	bool bIsDirectionVertical
)
{
	TSet<UDialogueGraphNode*> VisitedNodes;
	VisitedNodes.Add(RootNode);
	TQueue<NodeWithParentPosition> Queue;
	verify(Queue.Enqueue(NodeWithParentPosition(RootNode, 0, 0)));

	// Find first node with children so that we do not get all the graph with orphan nodes
	{
		UDialogueGraphNode* Node = RootNode;
		int32 Index = 0;
		while (Index < GraphNodes.Num() && Node->GetOutputPin()->LinkedTo.Num() == 0)
		{
			Node = GraphNodes[Index];
			Index++;
		}
		if (Node != RootNode)
		{
			NodeWithParentPosition ParentPosition;
			if (bIsDirectionVertical)
			{
				ParentPosition = NodeWithParentPosition(Node, 0, OffsetBetweenRowsY);
			}
			else
			{
				ParentPosition = NodeWithParentPosition(Node, OffsetBetweenColumnsX, 0);
			}

			verify(Queue.Enqueue(ParentPosition));
		}
	}

	// Just some BFS
	while (!Queue.IsEmpty())
	{
		NodeWithParentPosition NodeWithPosition;
		verify(Queue.Dequeue(NodeWithPosition));
		UDialogueGraphNode* Node = NodeWithPosition.Node;

		if (bIsDirectionVertical)
		{
			// Position this node at the same level only one row further (down)
			Node->SetPosition(
				NodeWithPosition.ParentNodeX,
				NodeWithPosition.ParentNodeY + OffsetBetweenRowsY
			);
		}
		else
		{
			// Position this node at the same level only one column further (to the right)
			Node->SetPosition(
				NodeWithPosition.ParentNodeX + OffsetBetweenColumnsX,
				NodeWithPosition.ParentNodeY
			);
		}

		// Gather the list of unvisited child nodes, useful for not drawing weird children
		TArray<UDialogueGraphNode*> ChildNodesUnvisited;
		for (UDialogueGraphNode* ChildNode : Node->GetChildNodes())
		{
			// Prevent double visiting
			if (!VisitedNodes.Contains(ChildNode))
			{
				ChildNodesUnvisited.Add(ChildNode);
			}
		}

		// Adjust
		int32 ChildOffsetPos;
		if (bIsDirectionVertical)
		{
			// Adjust for the number of nodes, so that we are left, down by half
			int32 ChildOffsetPosX = Node->NodePosX;
			if (ChildNodesUnvisited.Num() > 1) // only adjust X position if we have more than one child
			{
				ChildOffsetPosX -= OffsetBetweenColumnsX * ChildNodesUnvisited.Num() / 2;
			}

			ChildOffsetPos = ChildOffsetPosX;
		}
		else
		{
			// Adjust for the number of nodes, so that we are right above (top) by half
			int32 ChildOffsetPosY = Node->NodePosY;
			if (ChildNodesUnvisited.Num() > 1) // only adjust Y position if we have more than one child
			{
				ChildOffsetPosY -= OffsetBetweenRowsY * ChildNodesUnvisited.Num() / 2;
			}

			ChildOffsetPos = ChildOffsetPosY;
		}

		// Position children
		for (int32 ChildIndex = 0, ChildNum = ChildNodesUnvisited.Num(); ChildIndex < ChildNum; ChildIndex++)
		{
			UDialogueGraphNode* ChildNode = ChildNodesUnvisited[ChildIndex];
			if (bIsDirectionVertical)
			{
				ChildNode->SetPosition(ChildOffsetPos, Node->NodePosY);
			}
			else
			{
				ChildNode->SetPosition(Node->NodePosX, ChildOffsetPos);
			}

			VisitedNodes.Add(ChildNode);

			NodeWithParentPosition ParentPosition;
			if (bIsDirectionVertical)
			{
				ParentPosition = NodeWithParentPosition(ChildNode, ChildOffsetPos, Node->NodePosY);
				ChildOffsetPos += OffsetBetweenColumnsX + ChildNode->EstimateNodeWidth();
			}
			else
			{
				// Next child on this level will set X aka columns to Node->NodePosX + OffsetBetweenColumnsX
				// And Y aka row will be the same as this parent node, so it will be ChildOffsetPosY
				ParentPosition = NodeWithParentPosition(ChildNode, Node->NodePosX + ChildNode->EstimateNodeWidth() * 1.5, ChildOffsetPos);
				ChildOffsetPos += OffsetBetweenRowsY;
			}

			Queue.Enqueue(ParentPosition);
		}
	}

	// Fix position of orphans (nodes/node group with no parents)
	if (GraphNodes.Num() != VisitedNodes.Num())
	{
		TSet<UDialogueGraphNode*> NodesSet(GraphNodes);
		// Nodes that are in the graph but not in the visited nodes set
		TSet<UDialogueGraphNode*> OrphanedNodes = NodesSet.Difference(VisitedNodes);
		for (UDialogueGraphNode* Node : OrphanedNodes)
		{
			// Finds the highest bottom left point
			const FVector2D NodePos = Node->GetGraph()->GetGoodPlaceForNewNode();
			if (bIsDirectionVertical)
			{
				Node->SetPosition(
					NodePos.X,
					NodePos.Y + OffsetBetweenRowsY
				);
			}
			else
			{
				Node->SetPosition(
					NodePos.X + OffsetBetweenColumnsX,
					NodePos.Y
				);
			}
		}
	}
}

bool FDialogueEditorUtilities::CanConvertSpeechNodesToSpeechSequence(
	const TSet<UObject*>& SelectedNodes,
	TArray<UDialogueGraphNode*>& OutSelectedGraphNodes
)
{
	OutSelectedGraphNodes.Empty();
	if (SelectedNodes.Num() == 0)
	{
		return false;
	}

	// Helper to return false :(
	const auto returnFailure = [&OutSelectedGraphNodes]() -> bool
	{
		OutSelectedGraphNodes.Empty();
		return false;
	};

	// We must make sure that nodes are valid and are in a linear order
	// Step 1. Check if selected nodes are valid
	for (UObject* Node : SelectedNodes)
	{
		// Ignore edges
		if (Node->IsA(UDialogueGraphNode_Edge::StaticClass()))
		{
			continue;
		}

		// Not a graph node, can't convert
		UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(Node);
		if (!IsValid(GraphNode))
		{
			return returnFailure();
		}

		// Selected the root node, can't convert
		if (GraphNode->IsRootNode())
		{
			return returnFailure();
		}

		// Not a speech node, can't convert
		if (!GraphNode->IsSpeechNode())
		{
			return returnFailure();
		}

		OutSelectedGraphNodes.Add(GraphNode);
	}
	if (OutSelectedGraphNodes.Num() == 0)
	{
		return returnFailure();
	}

	// Step 2. Sort in increasing order by the dialogue index.
	// This will make sure that the left/top (first) most selected node will be at index 0
	OutSelectedGraphNodes.Sort([](const UDialogueGraphNode& LHS, const UDialogueGraphNode& RHS) -> bool
	{
		return LHS.GetDialogueNodeIndex() < RHS.GetDialogueNodeIndex();
	});

	// Step 3. Check that every node in the sequence is ONLY connected to the next
	for (int32 NodeIndex = 0, NodesNum = OutSelectedGraphNodes.Num(); NodeIndex < NodesNum; NodeIndex++)
	{
		const bool bIsFirstNode = NodeIndex == 0;
		const bool bIsLastNode = NodeIndex == NodesNum - 1;
		const UDialogueGraphNode* CurrentGraphNode = OutSelectedGraphNodes[NodeIndex];

		// The first node can have any number of input connections
		if (!bIsFirstNode)
		{
			// Check input connections
			const TArray<UDialogueGraphNode*> ParentNodes = CurrentGraphNode->GetParentNodes();
			if (ParentNodes.Num() > 1)
			{
				return returnFailure();
			}

			// Is not connected to the previous node in the selection
			if (ParentNodes.Num() == 1 &&
				ParentNodes[0] != OutSelectedGraphNodes[NodeIndex - 1])
			{
				return returnFailure();
			}
			// if ParentNodes.Num() == 0, it is valid
		}

		// The last node can have any number of output pins
		if (!bIsLastNode)
		{
			// Check output connections
			const TArray<UDialogueGraphNode*> ChildNodes = CurrentGraphNode->GetChildNodes();
			if (ChildNodes.Num() > 1)
			{
				return returnFailure();
			}

			// Is not connected to the next node in the selection
			if (ChildNodes.Num() == 1 &&
				ChildNodes[0] != OutSelectedGraphNodes[NodeIndex + 1])
			{
				return returnFailure();
			}
			// if ChildNodes.Num() == 0, it is valid
		}
		// We do not care if the node does not have any input/output connections, it is simply an orphan, lets adopt it :)
	}

	return true;
}

bool FDialogueEditorUtilities::CanConvertSpeechSequenceNodeToSpeechNodes(const TSet<UObject*>& SelectedNodes)
{
	// Is the node a speech sequence? and has at least one speech sequence inside it
	if (SelectedNodes.Num() == 1)
	{
		if (UDialogueGraphNode* SelectedNode = Cast<UDialogueGraphNode>(*FDlgHelper::GetFirstSetElement(SelectedNodes)))
		{
			return SelectedNode->IsSpeechSequenceNode() &&
				   SelectedNode->GetDialogueNode<UDlgNode_SpeechSequence>().HasSpeechSequences();
		}
	}

	return false;
}

void FDialogueEditorUtilities::CloseOtherEditors(UObject* Asset, IAssetEditorInstance* OnlyEditor)
{
	if (!IsValid(Asset) || !GEditor)
	{
		return;
	}

#if ENGINE_MINOR_VERSION >= 24
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(Asset, OnlyEditor);
#else
	FAssetEditorManager::Get().CloseOtherEditors(Asset, OnlyEditor);
#endif
}

bool FDialogueEditorUtilities::OpenEditorForAsset(const UObject* Asset)
{
	if (!IsValid(Asset) || !GEditor)
	{
		return false;
	}

#if ENGINE_MINOR_VERSION >= 24
	return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(const_cast<UObject*>(Asset));
#else
	return FAssetEditorManager::Get().OpenEditorForAsset(const_cast<UObject*>(Asset));
#endif
}

IAssetEditorInstance* FDialogueEditorUtilities::FindEditorForAsset(UObject* Asset, bool bFocusIfOpen)
{
	if (!IsValid(Asset) || !GEditor)
	{
		return nullptr;
	}

#if ENGINE_MINOR_VERSION >= 24
	return GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, bFocusIfOpen);
#else
	return FAssetEditorManager::Get().FindEditorForAsset(Asset, bFocusIfOpen);
#endif
}

bool FDialogueEditorUtilities::OpenEditorAndJumpToGraphNode(const UEdGraphNode* GraphNode, bool bFocusIfOpen /*= false*/)
{
	if (!IsValid(GraphNode))
	{
		return false;
	}

	// Open if not already.
	UDlgDialogue* Dialogue = GetDialogueFromGraphNode(GraphNode);
	if (!OpenEditorForAsset(Dialogue))
	{
		return false;
	}

	// Could still fail focus on the graph node
	if (IAssetEditorInstance* EditorInstance = FindEditorForAsset(Dialogue, bFocusIfOpen))
	{
		EditorInstance->FocusWindow(const_cast<UEdGraphNode*>(GraphNode));
		return true;
	}

	return false;
}

bool FDialogueEditorUtilities::JumpToGraphNode(const UEdGraphNode* GraphNode)
{
	if (!IsValid(GraphNode))
	{
		return false;
	}

	TSharedPtr<IDialogueEditor> DialogueEditor = GetDialogueEditorForGraph(GraphNode->GetGraph());
	if (DialogueEditor.IsValid())
	{
		DialogueEditor->JumpToObject(GraphNode);
		return true;
	}

	return false;
}

bool FDialogueEditorUtilities::JumpToGraphNodeIndex(const UDlgDialogue* Dialogue, int32 NodeIndex)
{
	if (!Dialogue)
	{
		return false;
	}

	if (UDlgNode* Node = Dialogue->GetMutableNodeFromIndex(NodeIndex))
	{
		return JumpToGraphNode(Node->GetGraphNode());
	}

	return false;
}


void FDialogueEditorUtilities::CopyNodeChildren(const UDialogueGraphNode* FromNode, UDialogueGraphNode* ToNode)
{
	check(FromNode != ToNode);
	const UEdGraphSchema* GraphSchema = FromNode->GetSchema();
	const TArray<UDialogueGraphNode*> ChildNodes = FromNode->GetChildNodes();
	UEdGraphPin* ToNodeOutputPin = ToNode->GetOutputPin();

	// Remake connections to children for the ToNode
	// (input pin) FromNode (output pin) -> (input pin) ChildEdgeConnection (output pin) -> (input pin) ChildNode (output pin)
	for (UDialogueGraphNode* ChildNode : ChildNodes)
	{
		verify(GraphSchema->TryCreateConnection(ToNodeOutputPin, ChildNode->GetInputPin()));
	}

	// Copy the dialogue Data
	ToNode->SetEdges(FromNode->GetDialogueNode().GetNodeChildren());
}

void FDialogueEditorUtilities::ReplaceParentConnectionsToNode(const UDialogueGraphNode* OldNode, const UDialogueGraphNode* NewNode)
{
	check(OldNode != NewNode);
	const UEdGraphSchema* GraphSchema = OldNode->GetSchema();
	const TArray<UDialogueGraphNode_Edge*> ParentEdgeNodes = OldNode->GetParentEdgeNodes();
	UEdGraphPin* NewNodeInputPin = NewNode->GetInputPin();

	// (input pin) ParentNode (output pin) -> (input pin) ParentEdgeConnection (output pin) -> (input pin) NewNode (output pin)
	for (UDialogueGraphNode_Edge* ParentEdgeConnection : ParentEdgeNodes)
	{
		// Replace connection from the edge output pin to the new node
		// Reparenting logic handled by UDialogueGraphNode_Edge::PinConnectionListChanged
		verify(GraphSchema->TryCreateConnection(ParentEdgeConnection->GetOutputPin(), NewNodeInputPin));
	}
}

EAppReturnType::Type FDialogueEditorUtilities::ShowMessageBox(EAppMsgType::Type MsgType, const FString& Text, const FString& Caption)
{
	UE_LOG(LogDlgSystemEditor, Warning, TEXT("%s\n%s"), *Caption, *Text);
	return FPlatformMisc::MessageBoxExt(MsgType, *Text, *Caption);
}

void FDialogueEditorUtilities::RemapOldIndicesWithNewAndUpdateGUID(
	const TArray<UDialogueGraphNode*>& GraphNodes,
	const TMap<int32, int32>& OldToNewIndexMap
)
{
	if (GraphNodes.Num() == 0)
	{
		return;
	}

	const UDlgDialogue* Dialogue = GraphNodes[0]->GetDialogue();
	const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();

	// helper function to set the new IntValue on the condition if it exists in the history and it is different
	auto UpdateConditionIndex = [&OldToNewIndexMap](FDlgCondition* ModifiedCondition) -> bool
	{
		const int32* NewIndex = OldToNewIndexMap.Find(ModifiedCondition->IntValue);
		// Ignore invalid node indices any case
		if (NewIndex == nullptr)
		{
			return false;
		}

		if (ModifiedCondition->IntValue != *NewIndex)
		{
			ModifiedCondition->IntValue = *NewIndex;
			return true;
		}

		return false;
	};

	// Fix the weak references in the FDlgCondition::IntValue if it is of type WasNodeVisited
	for (UDialogueGraphNode* GraphNode : GraphNodes)
	{
		UDlgNode* DialogueNode = GraphNode->GetMutableDialogueNode();
		const TArray<UDialogueGraphNode_Edge*> ChildEdgeNodes = GraphNode->GetChildEdgeNodes();

		// Update Enter condition
		for (int32 ConditionIndex = 0, ConditionNum = DialogueNode->GetNodeEnterConditions().Num(); ConditionIndex < ConditionNum; ConditionIndex++)
		{
			FDlgCondition* EnterCondition = DialogueNode->GetMutableEnterConditionAt(ConditionIndex);
			if (FDlgCondition::HasNodeIndex(EnterCondition->ConditionType))
			{
				UpdateConditionIndex(EnterCondition);
				EnterCondition->GUID = Nodes[EnterCondition->IntValue]->GetGUID();
			}
		}

		// Update Edges
		for (int32 EdgeIndex = 0, EdgesNum = DialogueNode->GetNodeChildren().Num(); EdgeIndex < EdgesNum; EdgeIndex++)
		{
			FDlgEdge* DialogueEdge = DialogueNode->GetSafeMutableNodeChildAt(EdgeIndex);

			for (FDlgCondition& Condition : DialogueEdge->Conditions)
			{
				if (FDlgCondition::HasNodeIndex(Condition.ConditionType))
				{
					UpdateConditionIndex(&Condition);
					Condition.GUID = Nodes[Condition.IntValue]->GetGUID();
				}
			}

			// Update graph node edge
			ChildEdgeNodes[EdgeIndex]->SetDialogueEdge(*DialogueEdge);
		}

		GraphNode->CheckDialogueNodeSyncWithGraphNode(true);
	}
}

UDlgDialogue* FDialogueEditorUtilities::GetDialogueFromGraphNode(const UEdGraphNode* GraphNode)
{
	if (const UDialogueGraphNode_Base* DialogueBaseNode = Cast<UDialogueGraphNode_Base>(GraphNode))
	{
		return DialogueBaseNode->GetDialogue();
	}

	// Last change
	if (const UDialogueGraph* DialogueGraph = Cast<UDialogueGraph>(GraphNode->GetGraph()))
	{
		return DialogueGraph->GetDialogue();
	}

	return nullptr;
}

bool FDialogueEditorUtilities::SaveAllDialogues()
{
	const TArray<UDlgDialogue*> Dialogues = UDlgManager::GetAllDialoguesFromMemory();
	TArray<UPackage*> PackagesToSave;
	const bool bBatchOnlyInGameDialogues = GetDefault<UDlgSystemSettings>()->bBatchOnlyInGameDialogues;

	for (UDlgDialogue* Dialogue : Dialogues)
	{
		// Ignore, not in game directory
		if (bBatchOnlyInGameDialogues && !Dialogue->IsInProjectDirectory())
		{
			continue;
		}

		Dialogue->MarkPackageDirty();
		PackagesToSave.Add(Dialogue->GetOutermost());
	}

	static constexpr bool bCheckDirty = false;
	static constexpr bool bPromptToSave = false;
	return FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, bCheckDirty, bPromptToSave) == FEditorFileUtils::EPromptReturnCode::PR_Success;
}

bool FDialogueEditorUtilities::DeleteAllDialoguesTextFiles()
{
	const TArray<UDlgDialogue*> Dialogues = UDlgManager::GetAllDialoguesFromMemory();
	const bool bBatchOnlyInGameDialogues = GetDefault<UDlgSystemSettings>()->bBatchOnlyInGameDialogues;
	for (const UDlgDialogue* Dialogue : Dialogues)
	{
		// Ignore, not in game directory
		if (bBatchOnlyInGameDialogues && !Dialogue->IsInProjectDirectory())
		{
			continue;
		}

		Dialogue->DeleteAllTextFiles();
	}

	return true;
}

bool FDialogueEditorUtilities::PickChildrenOfClass(const FText& TitleText, UClass*& OutChosenClass, UClass* Class)
{
	// Create filter
	TSharedPtr<FDialogueChildrenOfClassFilterViewer> Filter = MakeShareable(new FDialogueChildrenOfClassFilterViewer);
	Filter->AllowedChildrenOfClasses.Add(Class);

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;

	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	Options.DisplayMode = Settings->GetUnrealClassPickerDisplayMode();
	Options.ClassFilter = Filter;
	Options.bShowUnloadedBlueprints = true;
	Options.bExpandRootNodes = true;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;

	return SClassPickerDialog::PickClass(TitleText, Options, OutChosenClass, Class);
}

bool FDialogueEditorUtilities::OpenBlueprintEditor(
    UBlueprint* Blueprint,
    EDialogueBlueprintOpenType OpenType,
    FName FunctionNameToOpen,
    bool bForceFullEditor,
	bool bAddBlueprintFunctionIfItDoesNotExist
)
{
	if (!Blueprint)
	{
		return false;
	}

	Blueprint->bForceFullEditor = bForceFullEditor;

	// Find Function Graph
	UObject* ObjectToFocusOn = nullptr;
	if (OpenType != EDialogueBlueprintOpenType::None && FunctionNameToOpen != NAME_None)
	{
		UClass* Class = Blueprint->GeneratedClass;
		check(Class);

		if (OpenType == EDialogueBlueprintOpenType::Function)
		{
			ObjectToFocusOn = bAddBlueprintFunctionIfItDoesNotExist
                ? BlueprintGetOrAddFunction(Blueprint, FunctionNameToOpen, Class)
                : BlueprintGetFunction(Blueprint, FunctionNameToOpen, Class);
		}
		else if (OpenType == EDialogueBlueprintOpenType::Event)
		{
			ObjectToFocusOn = bAddBlueprintFunctionIfItDoesNotExist
                ? BlueprintGetOrAddEvent(Blueprint, FunctionNameToOpen, Class)
                : BlueprintGetEvent(Blueprint, FunctionNameToOpen, Class);
		}
	}

	// Default to the last uber graph
	if (ObjectToFocusOn == nullptr)
	{
		ObjectToFocusOn = Blueprint->GetLastEditedUberGraph();
	}
	if (ObjectToFocusOn)
	{
		FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(ObjectToFocusOn);
		return true;
	}

	return OpenEditorForAsset(Blueprint);
}

UEdGraph* FDialogueEditorUtilities::BlueprintGetOrAddFunction(UBlueprint* Blueprint, FName FunctionName, UClass* FunctionClassSignature)
{
	if (!Blueprint || Blueprint->BlueprintType != BPTYPE_Normal)
	{
		return nullptr;
	}

	// Find existing function
	if (UEdGraph* GraphFunction = BlueprintGetFunction(Blueprint, FunctionName, FunctionClassSignature))
	{
		return GraphFunction;
	}

	// Create a new function
	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, FunctionName, UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddFunctionGraph(Blueprint, NewGraph, /*bIsUserCreated=*/ false, FunctionClassSignature);
	Blueprint->LastEditedDocuments.Add(NewGraph);
	return NewGraph;
}

UEdGraph* FDialogueEditorUtilities::BlueprintGetFunction(UBlueprint* Blueprint, FName FunctionName, UClass* FunctionClassSignature)
{
	if (!Blueprint || Blueprint->BlueprintType != BPTYPE_Normal)
	{
		return nullptr;
	}

	// Find existing function
	for (UEdGraph* GraphFunction : Blueprint->FunctionGraphs)
	{
		if (FunctionName == GraphFunction->GetFName())
		{
			return GraphFunction;
		}
	}

	// Find in the implemented Interfaces Graphs
	for (const FBPInterfaceDescription& Interface : Blueprint->ImplementedInterfaces)
	{
		for (UEdGraph* GraphFunction : Interface.Graphs)
		{
			if (FunctionName == GraphFunction->GetFName())
			{
				return GraphFunction;
			}
		}
	}

	return nullptr;
}

UK2Node_Event* FDialogueEditorUtilities::BlueprintGetOrAddEvent(UBlueprint* Blueprint, FName EventName, UClass* EventClassSignature)
{
	if (!Blueprint || Blueprint->BlueprintType != BPTYPE_Normal)
	{
		return nullptr;
	}

	// Find existing event
	if (UK2Node_Event* EventNode = BlueprintGetEvent(Blueprint, EventName, EventClassSignature))
	{
		return EventNode;
	}

	// Create a New Event
	if (Blueprint->UbergraphPages.Num())
	{
		int32 NodePositionY = 0;
		UK2Node_Event* NodeEvent = FKismetEditorUtilities::AddDefaultEventNode(
			Blueprint,
			Blueprint->UbergraphPages[0],
			EventName,
			EventClassSignature,
			NodePositionY
		);
		NodeEvent->SetEnabledState(ENodeEnabledState::Enabled);
		NodeEvent->NodeComment = "";
		NodeEvent->bCommentBubbleVisible = false;
		return NodeEvent;
	}

	return nullptr;
}

UK2Node_Event* FDialogueEditorUtilities::BlueprintGetEvent(UBlueprint* Blueprint, FName EventName, UClass* EventClassSignature)
{
	if (!Blueprint || Blueprint->BlueprintType != BPTYPE_Normal)
	{
		return nullptr;
	}

	TArray<UK2Node_Event*> AllEvents;
	FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_Event>(Blueprint, AllEvents);
	for (UK2Node_Event* EventNode : AllEvents)
	{
		if (EventNode->bOverrideFunction && EventNode->EventReference.GetMemberName() == EventName)
		{
			return EventNode;
		}
	}

	return nullptr;
}

UEdGraphNode_Comment* FDialogueEditorUtilities::BlueprintAddComment(UBlueprint* Blueprint, const FString& CommentString, FVector2D Location)
{
	if (!Blueprint || Blueprint->BlueprintType != BPTYPE_Normal || Blueprint->UbergraphPages.Num() == 0)
	{
		return nullptr;
	}

	UEdGraph* Graph = Blueprint->UbergraphPages[0];
	TSharedPtr<FEdGraphSchemaAction> Action = Graph->GetSchema()->GetCreateCommentAction();
	if (!Action.IsValid())
	{
		return nullptr;
	}

	UEdGraphNode* GraphNode = Action->PerformAction(Graph, nullptr, Location);
	if (UEdGraphNode_Comment* CommentNode = Cast<UEdGraphNode_Comment>(GraphNode))
	{
		CommentNode->NodeComment = CommentString;
		return CommentNode;
	}

	return nullptr;
}
