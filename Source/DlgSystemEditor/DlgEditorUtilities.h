// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "ConnectionDrawingPolicy.h"
#include "EdGraphNode_Comment.h"

#include "Editor/Graph/DialogueGraph.h"
#include "DlgSystem/Nodes/DlgNode.h"

enum class EDlgBlueprintOpenType : uint8
{
	None = 0,
	Function,
	Event
};

//////////////////////////////////////////////////////////////////////////
// FDlgEditorUtilities

class UDlgDialogue;
class UEdGraphSchema;
class UDlgNode;
class UEdGraph;
class FSlateRect;
class UK2Node_Event;

class DLGSYSTEMEDITOR_API FDlgEditorUtilities
{
public:
	/** Spawns a GraphNode in the specified ParentGraph and at Location. */
	template <typename GraphNodeType>
	static GraphNodeType* SpawnGraphNodeFromTemplate(UEdGraph* ParentGraph, const FIntPoint& Location, bool bSelectNewNode = true)
	{
		FGraphNodeCreator<GraphNodeType> NodeCreator(*ParentGraph);
		GraphNodeType* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
		NodeCreator.Finalize(); // Calls on the node: CreateNewGuid, PostPlacedNewNode, AllocateDefaultPins
		GraphNode->NodePosX = Location.X;
		GraphNode->NodePosY = Location.Y;
		GraphNode->SetFlags(RF_Transactional);

		return GraphNode;
	}

	// Loads all dialogues into memory and checks the GUIDs for duplicates
	static void LoadAllDialoguesAndCheckGUIDs();

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
	static bool GetBoundsForSelectedNodes(const UEdGraph* Graph, FSlateRect& Rect, float Padding = 0.0f);

	/** Refreshes the details panel for the editor of the specified Graph. */
	static void RefreshDetailsView(const UEdGraph* Graph, bool bRestorePreviousSelection);

	// Refresh the viewport and property/details pane
	static void Refresh(const UEdGraph* Graph, bool bRestorePreviousSelection);

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
	static UEdGraph* CreateNewGraph(
		UObject* ParentScope,
		FName GraphName,
		TSubclassOf<UEdGraph> GraphClass,
		TSubclassOf<UEdGraphSchema> SchemaClass
	);

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

	// Tries to get the closest UDlgNode for a  UEdGraphNode
	static UDlgNode* GetClosestNodeFromGraphNode(UEdGraphNode* GraphNode);

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
	static void AutoPositionGraphNodes(
		UDialogueGraphNode* RootNode,
		const TArray<UDialogueGraphNode*>& GraphNodes,
		int32 OffsetBetweenColumnsX,
		int32 OffsetBetweenRowsY,
		bool bIsDirectionVertical
	);

	/**
	 * Tells us if the selected nodes can be converted to a speech sequence node.
	 *
	 * @param	SelectedNodes			The selected graph nodes we do the checking on.
	 * @param	OutSelectedGraphNodes	The result graph nodes if the we can do the conversion.
	 *
	 * @return bool		If true, the selected graph nodes (filtered and sorted) will be in set in the OutSelectedGraphNodes array.
	 *					If false, the OutSelectedGraphNodes will be empty
	*/
	static bool CanConvertSpeechNodesToSpeechSequence(
		const TSet<UObject*>& SelectedNodes,
		TArray<UDialogueGraphNode*>& OutSelectedGraphNodes
	);

	static bool CanConvertSpeechNodesToSpeechSequence(const TSet<UObject*>& SelectedNodes)
	{
		TArray<UDialogueGraphNode*> OutSelectedGraphNodes;
		return CanConvertSpeechNodesToSpeechSequence(SelectedNodes, OutSelectedGraphNodes);
	}

	/**
	 * Tells us if the selected nodes (should be only one) can be converted from a speech sequence to speech nodes
	 *
	 * @param	SelectedNodes	The selected graph nodes we do the checking on.
	 * @return	bool			If we can do the conversion
	 */
	static bool CanConvertSpeechSequenceNodeToSpeechNodes(const TSet<UObject*>& SelectedNodes);

	/** Close any editor which is not this one */
	static void CloseOtherEditors(UObject* Asset, IAssetEditorInstance* OnlyEditor);

	/**
	 * Tries to open the editor for the specified asset. Returns true if the asset is opened in an editor.
	 * If the file is already open in an editor, it will not create another editor window but instead bring it to front
	 */
	static bool OpenEditorForAsset(const UObject* Asset);

	/** Returns the primary editor if one is already open for the specified asset.
	 * If there is one open and bFocusIfOpen is true, that editor will be brought to the foreground and focused if possible.
	 */
	static IAssetEditorInstance* FindEditorForAsset(UObject* Asset, bool bFocusIfOpen);

	/**
	 * Tries to open an Dialogue editor for the GraphNode and jumps to it. Returns true if the asset is opened in an editor.
	 * If the file is already open in an editor, it will not create another editor window but instead bring it to front
	 */
	static bool OpenEditorAndJumpToGraphNode(const UEdGraphNode* GraphNode, bool bFocusIfOpen = false);

	// Just jumps to that graph node without trying to open any Dialogue Editor
	// If you want that just call OpenEditorAndJumpToGraphNode
	static bool JumpToGraphNode(const UEdGraphNode* GraphNode);
	static bool JumpToGraphNodeIndex(const UDlgDialogue* Dialogue, int32 NodeIndex);

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

	// Wrapper over standard message box that that also logs to the console
	static EAppReturnType::Type ShowMessageBox(EAppMsgType::Type MsgType, const FString& Text, const FString& Caption);

	// Returns true if the TestPoint is inside the Geometry.
	static bool IsPointInsideGeometry(const FVector2D& TestPoint, const FGeometry& Geometry)
	{
		TArray<FVector2D> GeometryPoints;
		FGeometryHelper::ConvertToPoints(Geometry, GeometryPoints);
		return FBox2D(GeometryPoints).IsInside(TestPoint);
	}

	/**
	 * Replaces all references to old Node indices from the provided GraphNodes with new indices.
	 * This can happen inside Conditions of type WasNodeVisited and HasSatisfiedChild because the NodeIndex is a weak reference.
	 * Also sets the correct GUID to nodes that reference the GUID
	 *
	 * @param	GraphNodes			The nodes we are replacing the old references
	 * @param	OldToNewIndexMap	Map that tells us the mapping from old index to new index. Maps from old index -> new index
	 */
	static void RemapOldIndicesWithNewAndUpdateGUID(
		const TArray<UDialogueGraphNode*>& GraphNodes,
		const TMap<int32, int32>& OldToNewIndexMap
	);

	// Gets the Dialogue for the provided UEdGraphNode
	static UDlgDialogue* GetDialogueFromGraphNode(const UEdGraphNode* GraphNode);

	// Save all the dialogues.
	// @return True on success or false on failure.
	static bool SaveAllDialogues();

	// Deletes all teh dialogues text files
	// @return True on success or false on failure.
	static bool DeleteAllDialoguesTextFiles();

	/***
	* Pops up a class picker dialog to choose the class that is a child of the Classprovided.
	*
	* @param	TitleText		The title of the class picker dialog
	* @param	OutChosenClass  The class chosen (if this function returns false, this will be null) by the the user
	* @param	Class			The children of this class we are displaying and prompting the user to choose from.
	*
	* @return true if OK was pressed, false otherwise
	*/
	static bool PickChildrenOfClass(const FText& TitleText, UClass*& OutChosenClass, UClass* Class);

	// Opens the specified Blueprint at the last edited graph by default
	// or if the OpenType is set to Function or Event it opens that with the FunctionNameToOpen
	static bool OpenBlueprintEditor(
		UBlueprint* Blueprint,
		EDlgBlueprintOpenType OpenType = EDlgBlueprintOpenType::None,
		FName FunctionNameToOpen = NAME_None,
		bool bForceFullEditor = true,
		bool bAddBlueprintFunctionIfItDoesNotExist = false
	);

	// Adds the function if it does not exist
	// Return the Function Graph of the existing function or the newly created one
	static UEdGraph* BlueprintGetOrAddFunction(UBlueprint* Blueprint, FName FunctionName, UClass* FunctionClassSignature);

	// Same as BlueprintGetOrAddFunction but does not add it
	static UEdGraph* BlueprintGetFunction(UBlueprint* Blueprint, FName FunctionName, UClass* FunctionClassSignature);

	// Same as BlueprintGetOrAddFunction but only for an overriden event
	static UK2Node_Event* BlueprintGetOrAddEvent(UBlueprint* Blueprint, FName EventName, UClass* EventClassSignature);

	// Same as BlueprintGetOrAddEvent but does not add it
	static UK2Node_Event* BlueprintGetEvent(UBlueprint* Blueprint, FName EventName, UClass* EventClassSignature);

	// Adds a comment to the Blueprint
	static UEdGraphNode_Comment* BlueprintAddComment(UBlueprint* Blueprint, const FString& CommentString, FVector2D Location = FVector2D::ZeroVector);

	static void RefreshDialogueEditorForGraph(const UEdGraph* Graph);

private:
	// Get the DialogueEditor for given object, if it exists
	static TSharedPtr<class IDlgEditor> GetDialogueEditorForGraph(const UEdGraph* Graph);

	FDlgEditorUtilities() = delete;
};
