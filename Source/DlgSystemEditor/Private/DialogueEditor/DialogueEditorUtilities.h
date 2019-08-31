// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "ConnectionDrawingPolicy.h"
#include "PropertyHandle.h"
#include "Toolkits/AssetEditorManager.h"
#include "EdGraphNode_Comment.h"

#include "DialogueEditor/Graph/DialogueGraph.h"
#include "Nodes/DlgNode.h"

//////////////////////////////////////////////////////////////////////////
// FDialogueEditorUtilities

class UDlgDialogue;
class UEdGraphSchema;
struct FDlgNode;
class UEdGraph;

/** Gets the first element from a set. From https://answers.unrealengine.com/questions/332443/how-to-get-the-firstonly-element-in-tset.html */
template <typename SetType>
typename TCopyQualifiersFromTo<SetType, typename SetType::ElementType>::Type* GetFirstSetElement(SetType& Set)
{
	for (auto& Element : Set)
	{
		return &Element;
	}

	return nullptr;
}

namespace Graph
{
	/** Spawns a GraphNode in the specified ParentGraph and at Location. */
	template <typename GraphNodeType>
	static GraphNodeType* SpawnGraphNodeFromTemplate(class UEdGraph* ParentGraph, const FVector2D Location, bool bSelectNewNode = true)
	{
		FGraphNodeCreator<GraphNodeType> NodeCreator(*ParentGraph);
		GraphNodeType* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
		NodeCreator.Finalize(); // Calls on the node: CreateNewGuid, PostPlacedNewNode, AllocateDefaultPins
		GraphNode->NodePosX = Location.X;
		GraphNode->NodePosY = Location.Y;
		GraphNode->SetFlags(RF_Transactional);

		return GraphNode;
	}
}

class FDialogueEditorUtilities
{
public:
	/** Gets the nodes that are currently selected */
	static const TSet<UObject*> GetSelectedNodes(const UEdGraph* Graph);

	/** Get the bounding area for the currently selected nodes
	 *
	 * @param Graph The Graph we are finding bounds for
	 * @param Rect Final output bounding area, including padding
	 * @param Padding An amount of padding to add to all sides of the bounds
	 *
	 * @return false if nothing is selected
	 */
	static bool GetBoundsForSelectedNodes(const UEdGraph* Graph, class FSlateRect& Rect, float Padding = 0.0f);

	/** Refreshes the details panel for the editor of the specified Graph. */
	static void RefreshDetailsView(const UEdGraph* Graph);

	/** Useful for setting the last target edge on drap operations. */
	static UDialogueGraphNode_Edge* GetLastTargetGraphEdgeBeforeDrag(const UEdGraph* Graph);
	static void SetLastTargetGraphEdgeBeforeDrag(const UEdGraph* Graph, UDialogueGraphNode_Edge* InEdge);

	/** Helper function to remove the provided node from it's graph. Returns true on success, false otherwise. */
	static bool RemoveNode(UEdGraphNode* NodeToRemove);

	/**
	 * Creates a new empty graph.
	 *
	 * @param	ParentScope		The outer of the new graph (typically a blueprint).
	 * @param	GraphName		Name of the graph to add.
	 * @param	SchemaClass		Schema to use for the new graph.
	 *
	 * @return	nullptr if it fails, else ther new created graph
	 */
	static UEdGraph* CreateNewGraph(UObject* ParentScope, const FName& GraphName, TSubclassOf<UEdGraph> GraphClass,
								   TSubclassOf<UEdGraphSchema> SchemaClass);

	/** Helper function that checks if the data is valid in the Dialogue/Graph and tries to fix the data. */
	static bool CheckAndTryToFixDialogue(UDlgDialogue* Dialogue, bool bDisplayWarning = true);

	/**
	 * Tries to create the default graph for the Dialogue if the number of nodes differ from the dialogue data and the graph data
	 *
	 * @param Dialogue		The Dialogue we want to create the default graph for.
	 * @param bPrompt		Indicates if we should prompt the user for a response.
	 */
	static void TryToCreateDefaultGraph(UDlgDialogue* Dialogue, bool bPrompt = true);

	/** Tells us if the number of dialogue nodes matches with the number of graph nodes (corresponding to dialogues). */
	static bool AreDialogueNodesInSyncWithGraphNodes(const UDlgDialogue* Dialogue);

	/** Gets the Dialogue from the Graph */
	static UDlgDialogue* GetDialogueForGraph(const UEdGraph* Graph)
	{
		return CastChecked<UDialogueGraph>(Graph)->GetDialogue();
	}

	/**
	 * Automatically reposition all the nodes in the graph.
	 *
	 * @param	RootNode				The Node that is considered the node
	 * @param	GraphNodes				The rest of the graph nodes
	 * @param	OffsetBetweenColumnsX   The offset between nodes on the X axis
	 * @param	OffsetBetweenRowsY		The offset between nodes on the Y axis
	 * @param	bIsDirectionVertical	Is direction vertical? If false it is horizontal
	 */
	static void AutoPositionGraphNodes(UDialogueGraphNode* RootNode, const TArray<UDialogueGraphNode*>& GraphNodes,
									int32 OffsetBetweenColumnsX, int32 OffsetBetweenRowsY, bool bIsDirectionVertical);

	/**
	 * Tells us if the selected nodes can be converted to a speech sequence node.
	 *
	 * @param	SelectedNodes			The selected graph nodes we do the checking on.
	 * @param	OutSelectedGraphNodes	The result graph nodes if the we can do the conversion.
	 *
	 * @return bool		If true, the selected graph nodes (filtered and sorted) will be in set in the OutSelectedGraphNodes array.
	 *					If false, the OutSelectedGraphNodes will be empty
	*/
	static bool CanConvertSpeechNodesToSpeechSequence(const TSet<UObject*>& SelectedNodes,
													  TArray<UDialogueGraphNode*>& OutSelectedGraphNodes);

	/**
	 * Tells us if the selected nodes (should be only one) can be converted from a speech sequence to speech nodes
	 *
	 * @param	SelectedNodes	The selected graph nodes we do the checking on.
	 * @return	bool			If we can do the conversion
	 */
	static bool CanConvertSpeechSequenceNodeToSpeechNodes(const TSet<UObject*>& SelectedNodes);

	/**
	 * Tries to open the editor for the specified asset. Returns true if the asset is opened in an editor.
	 * If the file is already open in an editor, it will not create another editor window but instead bring it to front
	 */
	static bool OpenEditorForAsset(const UObject* Asset)
	{
		if (!IsValid(Asset))
		{
			return false;
		}

		return FAssetEditorManager::Get().OpenEditorForAsset(const_cast<UObject*>(Asset));
	}

	/**
	 * Tries to open an Dialogue editor for the GraphNode and jumps to it. Returns true if the asset is opened in an editor.
	 * If the file is already open in an editor, it will not create another editor window but instead bring it to front
	 */
	static bool OpenEditorAndJumpToGraphNode(const UEdGraphNode* GraphNode, bool bFocusIfOpen = false);

	/**
	 * Copy all children of the FromNode to be also the children of ToNode.
	 *
	 * @param	FromNode	Source Node
	 * @param	ToNode		Destination Node
	 */
	static void CopyNodeChildren(const UDialogueGraphNode* FromNode, UDialogueGraphNode* ToNode);

	/**
	 * Replace all connection to the OldNode, so that they point to the NewNode.
	 * Make all parents of the OldNode point to NewNode instead.
	 *
	 * @param	OldNode		The old child node that will have its conections from its parents removed and replaced to point to NewNode.
	 * @param	NewNode		The new child node that will have the connections to the parents of OldNode
	 */
	static void ReplaceParentConnectionsToNode(const UDialogueGraphNode* OldNode, const UDialogueGraphNode* NewNode);

	/**
	 * Wrapper over standard message box that that also logs to the console
	 */
	static EAppReturnType::Type ShowMessageBox(EAppMsgType::Type MsgType, const FString& Text, const FString& Caption);

	/** Returns true if the TestPoint is inside the Geometry. */
	static bool IsPointInsideGeometry(const FVector2D& TestPoint, const FGeometry& Geometry)
	{
		TArray<FVector2D> GeometryPoints;
		FGeometryHelper::ConvertToPoints(Geometry, GeometryPoints);
		return FBox2D(GeometryPoints).IsInside(TestPoint);
	}

	/**
	 * Replaces all references to old Node indices from the provided GraphNodes with new indices.
	 * This can happen inside Conditions of type DlgConditionNodeVisited and DlgConditionHasSatisfiedChild because the NodeIndex is a weak reference.
	 *
	 * @param	GraphNodes			The nodes we are replacing the old references
	 * @param	OldToNewIndexMap	Map that tells us the mapping from old index to new index. Maps from old index -> new index
	 */
	static void ReplaceReferencesToOldIndiciesWithNew(const TArray<UDialogueGraphNode*>& GraphNodes,
													  const TMap<int32, int32>& OldToNewIndexMap);

	/** Gets the Dialogue for the provided UEdGraphNode_Comment  */
	static UDlgDialogue* GetDialogueFromGraphNodeComment(const UEdGraphNode_Comment* CommentNode)
	{
		if (!IsValid(CommentNode))
		{
			return nullptr;
		}

		if (const UDialogueGraph* DialogueGraph = Cast<UDialogueGraph>(CommentNode->GetGraph()))
		{
			return DialogueGraph->GetDialogue();
		}

		return nullptr;
	}

private:
	/** Get the DialogueEditor for given object, if it exists */
	static TSharedPtr<class IDialogueEditor> GetDialogueEditorForGraph(const UEdGraph* Graph);

	FDialogueEditorUtilities() = delete;
};
