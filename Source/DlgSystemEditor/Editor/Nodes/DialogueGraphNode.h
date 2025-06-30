// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreTypes.h"
#include "UObject/ObjectMacros.h"
#include "Runtime/Launch/Resources/Version.h"

#include "DlgSystem/Nodes/DlgNode_End.h"
#include "DlgSystem/Nodes/DlgNode_Speech.h"
#include "DlgSystem/Nodes/DlgNode_Selector.h"
#include "DlgSystem/Nodes/DlgNode_Proxy.h"
#include "DlgSystem/Nodes/DlgNode_Custom.h"
#include "DlgSystem/Nodes/DlgNode_SpeechSequence.h"
#include "DialogueGraphNode_Base.h"
#include "DlgSystem/NYEngineVersionHelpers.h"

#include "DialogueGraphNode.generated.h"

class UEdGraphPin;
class DialogueGraphNode_Edge;
class UToolMenu;
class UGraphNodeContextMenuContext;

/** Result for a single difference between the Dialogue Node Edges and LinkedTo Array of the output pins */
struct DLGSYSTEMEDITOR_API FDiffNodeEdgeLinkedToPinResult
{
	enum EDiffType
	{
		NO_DIFFERENCE = 0,

		// A result of DoesEdgeMatchEdgeIndex
		EDGE_NOT_MATCHING_INDEX,

		// The length of the arrays mismatch, there is one more edge (located at the end) than pin connection
		LENGTH_MISMATCH_ONE_MORE_EDGE,

		// The length of the arrays mismatch, there is one more pin connection (located at the end) than edges
		LENGTH_MISMATCH_ONE_MORE_PIN_CONNECTION,

		// Diff type not supported
		NOT_SUPPORTED
	};

	/** Tells us the type of diff */
	EDiffType Type = EDiffType::NO_DIFFERENCE;

	/**
	 * Depending on the Type of diff this index can mean diferent things:
	 * - EDGE_NOT_MATCHING_INDEX
	 *		- represents the Index number of both arrays that is different
	 * - LENGTH_MISMATCH_ONE_MORE_EDGE
	 *		- represents the index (of the one more edge) that does not exist in the LinkedTo array but exists in the Edges array
	 * - LENGTH_MISMATCH_ONE_MORE_PIN_CONNECTION
	 *		- represents the index (of the one more pin connection) that does not exist in the Edges array but exists in the LinkedTo array
	 */
	int32 Index = INDEX_NONE;

	/** Some error message if any. */
	FString Message;
};

UCLASS()
class DLGSYSTEMEDITOR_API UDialogueGraphNode : public UDialogueGraphNode_Base
{
	GENERATED_BODY()

public:
	//
	// Begin UObject Interface.
	//

	/**
	 * Do any object-specific cleanup required immediately after loading an object,
	 * and immediately after any undo/redo.
	 */
	virtual void PostLoad() override;

	/**
	 * Called after importing property values for this object (paste, duplicate or .t3d import)
	 * Allow the object to perform any cleanup for properties which shouldn't be duplicated or
	 * are unsupported by the script serialization
	 */
	virtual void PostEditImport() override;

	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	 * is located at the tail of the list.  The head of the list of the FStructProperty member variable that contains the property that was modified.
	 */
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;

	/**
	 * Note that the object will be modified.  If we are currently recording into the
	 * transaction buffer (undo/redo), save a copy of this object into the buffer and
	 * marks the package as needing to be saved.
	 *
	 * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
	 *								currently recording an active undo/redo transaction
	 * @return true if the object was saved to the transaction buffer
	 */
	virtual bool Modify(bool bAlwaysMarkDirty = true) override;

	//
	// Begin UEdGraphNode interface
	//

	/** Gets the name of this node, shown in title bar */
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	/** Gets the tooltip to display when over the node */
	virtual FText GetTooltipText() const override;
	virtual FString GetDocumentationExcerptName() const override;

	/** Whether or not this node can be safely duplicated (via copy/paste, etc...) in the graph. */
	virtual bool CanDuplicateNode() const override { return !IsRootNode(); }

	/** Perform any steps necessary prior to copying a node into the paste buffer */
	virtual void PrepareForCopying() override;

	/**
	 * Called when something external to this node has changed the connection list of any of the pins in the node
	 *   - Different from PinConnectionListChanged as this is called outside of any loops iterating over our pins allowing
	 *	 us to do things like reconstruct the node safely without trashing pins we are already iterating on
	 *   - Typically called after a user induced action like making a pin connection or a pin break
	 */
	virtual void NodeConnectionListChanged() override
	{
		CheckDialogueNodeSyncWithGraphNode(true);
		ApplyCompilerWarnings();
	}

	/** Called when the connection list of one of the pins of this node is changed in the editor */
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	/** Gets a list of actions that can be done to this particular node */
#if NY_ENGINE_VERSION >= 424
	virtual void GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const override;
#else
	virtual void GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const override;
#endif

	/**
	 * Autowire a newly created node.
	 *
	 * @param FromPin	The source pin that caused the new node to be created (typically a drag-release context menu creation).
	 */
	virtual void AutowireNewNode(UEdGraphPin* FromPin) override;

	// Begin UDialogueGraphNode_Base interface

	/** Checks whether an input connection can be added to this node */
	virtual bool CanHaveInputConnections() const override;

	/** Checks whether an output connection can be added from this node */
	virtual bool CanHaveOutputConnections() const override;

	/** Checks if this node has a output connection to the TargetNode. */
	virtual bool HasOutputConnectionToNode(const UEdGraphNode* TargetNode) const override;

	/** Gets the background color of this node. */
	virtual FLinearColor GetNodeBackgroundColor() const override;

	/** Perform any fixups (deep copies of associated data, etc...) necessary after a node has been copied in the editor. */
	virtual void PostCopyNode() override;

	/** Perform all checks */
	virtual void CheckAll() const override
	{
#if DO_CHECK
		Super::CheckAll();
		check(IsDialogueNodeSet());
		CheckDialogueNodeIndexMatchesNode();
		CheckDialogueNodeSyncWithGraphNode(true);
#endif
	}

	/** Is this the undeletable root node */
	virtual bool IsRootNode() const { return false; }

	//
	// Begin own functions
	//

	/** Is this an End Node? */
	bool IsEndNode() const { return DialogueNode->IsA<UDlgNode_End>(); }

	/** Is this a Speech Node? */
	bool IsSpeechNode() const { return DialogueNode->IsA<UDlgNode_Speech>(); }

	/** Is this a virtual parent Node? */
	bool IsVirtualParentNode() const
	{
		if (const UDlgNode_Speech* Node = Cast<UDlgNode_Speech>(DialogueNode))
		{
			return Node->IsVirtualParent();
		}

		return false;
	}

	/** Is this a selector Node? */
	bool IsSelectorNode() const { return DialogueNode->IsA<UDlgNode_Selector>(); }

	/** Is this a selector Node? */
	bool IsProxyNode() const { return DialogueNode->IsA<UDlgNode_Proxy>(); }

	/** Is custom Node? */
	bool IsCustomNode() const { return DialogueNode->IsA<UDlgNode_Custom>(); }

	/** Is this a selector First Node? */
	bool IsSelectorFirstNode() const
	{
		if (const UDlgNode_Selector* Node = Cast<UDlgNode_Selector>(DialogueNode))
		{
			return Node->GetSelectorType() == EDlgNodeSelectorType::First;
		}

		return false;
	}

	/** Is this a selector Random Node? */
	bool IsSelectorRandomNode() const
	{
		if (const UDlgNode_Selector* Node = Cast<UDlgNode_Selector>(DialogueNode))
		{
			return Node->GetSelectorType() == EDlgNodeSelectorType::Random;
		}

		return false;
	}

	/** Is this a Speech Sequence Node? */
	bool IsSpeechSequenceNode() const { return DialogueNode->IsA<UDlgNode_SpeechSequence>(); }

	/** Does this node has any enter conditions? */
	bool HasEnterConditions() const
	{
		return DialogueNode ? DialogueNode->HasAnyEnterConditions() : false;
	}

	/** Does this node has any enter events? */
	bool HasEnterEvents() const
	{
		return DialogueNode ? DialogueNode->HasAnyEnterEvents() : false;
	}

	/** Does this node has any voice properties set? */
	bool HasVoicePropertiesSet() const;

	bool IsProxyNodeLeadingToIt() const;

	/** Checks if it is normal for the node to not have a parent */
	bool CanBeOrphan() const;

	/** Does this node has any voice properties set? */
	bool HasGenericDataSet() const;

	/** Gets the node depth in the graph. */
	int32 GetNodeDepth() const { return NodeDepth; }

	/** Sets the new node depth. */
	void SetNodeDepth(int32 NewNodeDepth) { NodeDepth = NewNodeDepth; }

	/** Sets the Dialogue Node. */
	virtual void SetDialogueNode(UDlgNode* InNode)
	{
		DialogueNode = InNode;
		DialogueNode->SetFlags(RF_Transactional);
		DialogueNode->SetGraphNode(this);
		RegisterListeners();
	}

	/** Sets the Dialogue node index number, this represents the index from the DlgDialogue.Nodes Array */
	virtual void SetDialogueNodeIndex(int32 InIndex)
	{
		check(InIndex > INDEX_NONE);
		NodeIndex = InIndex;
	}

	// Where should the edges pointing to this node be positioned at
	// NOTE: we use this because otherwise the edges don't get rendered
	FIntPoint GetDefaultEdgePosition() const { return GetPosition() + FIntPoint(5, 5); }


	/**
	 * The same SetDialogueNodeIndex and SetDialogueNode only that it sets them both at once and it does some sanity checking
	 * such as verifying the index is valid in the Dialogue node and that the index corresponds to this InNode.
	 */
	void SetDialogueNodeDataChecked(int32 InIndex, UDlgNode* InNode);

	/** Gets the copy of the DlgNode stored by this graph node */
	template <typename DlgNodeType>
	const DlgNodeType& GetDialogueNode() const { return *CastChecked<DlgNodeType>(DialogueNode); }

	/** Gets the copy of the DlgNode stored by this graph node as a mutable pointer */
	template <typename DlgNodeType>
	DlgNodeType* GetMutableDialogueNode() { return CastChecked<DlgNodeType>(DialogueNode); }

	// Specialization for the methods above  (by overloading) for the base type UDlgNode type so that we do not need to cast
	const UDlgNode& GetDialogueNode() const { return *DialogueNode; }
	UDlgNode* GetMutableDialogueNode() const { return DialogueNode; }

	/** Tells us if the Dialogue Node is valid non null. */
	bool IsDialogueNodeSet() const { return DialogueNode != nullptr; }

	/** Gets the Dialogue node index number for the DlgDialogue.Nodes Array */
	virtual int32 GetDialogueNodeIndex() const { return NodeIndex; }

	/** Gets the edge inside fromGraphNodeEdges for the ChildNodeIndex  */
	int32 GetChildEdgeIndexForChildNodeIndex(int32 ChildNodeIndex) const;

	/** Sets a new TargetIndex for the Edge at location EdgeIndex.  */
	void SetEdgeTargetIndexAt(int32 EdgeIndex, int32 NewTargetIndex);

	/** Sets a new Text for the Edge at location EdgeIndex.  */
	void SetEdgeTextAt(int32 EdgeIndex, const FText& NewText);

	// Sets all the node children (edges).
	// NOTE: USE WITH CAUTION
	void SetEdges(const TArray<FDlgEdge>& InEdges);

	// Updates the edges data from the DialogueNode
	// NOTE: USE WITH CAUTION
	void UpdateEdgesFromDialogueNode();

	/** Checks the node for warnings and applies the compiler warnings messages */
	void ApplyCompilerWarnings();

	/** Estimate the width of this Node from the length of its content */
	int32 EstimateNodeWidth() const;

	/** Checks Dialogue.Nodes[NodeIndex] == DialogueNode */
	void CheckDialogueNodeIndexMatchesNode() const;

	/** Helper function to check if the DialogueNode.Children matches with the Pins of the graph node */
	void CheckDialogueNodeSyncWithGraphNode(bool bStrictCheck = false) const;

	/** Gets the parent nodes that are connected to the input pin. This handles the proxy connection to the UDialogueGraphNode_Edge.  */
	TArray<UDialogueGraphNode*> GetParentNodes() const;

	/** Gets the child nodes that are connected from the output pin. This handles the proxy connection to the UDialogueGraphNode_Edge.  */
	TArray<UDialogueGraphNode*> GetChildNodes() const;

	/** Gets the parent node edges that are connected from the input pin. This returns the proxy edge nodes. To surpass the proxy connection use GetParentNodes. */
	TArray<UDialogueGraphNode_Edge*> GetParentEdgeNodes(bool bCheckChild = true) const;

	/** Gets the child node edges that are connected from the output pin. This returns the proxy edge nodes. To surpass the proxy connection use GetChildNodes. */
	TArray<UDialogueGraphNode_Edge*> GetChildEdgeNodes(bool bCheckParent = true) const;

	/** Does this node have the child edge ChildEdgeToFind? */
	bool HasChildEdgeNode(const UDialogueGraphNode_Edge* ChildEdgeToFind) const;

	/** Does this node has the  parent edge ParentEdgeToFind?  */
	bool HasParentEdgeNode(const UDialogueGraphNode_Edge* ParentEdgeToFind) const;

	/** Rearranges the children (edges, output pin, connections) based on the X location on the graph. */
	void SortChildrenBasedOnXLocation();

	/** Should we force hide this node? */
	bool GetForceHideNode() const { return bForceHideNode; }

	/** Forcefully hide/show this node and all edges that are coming into it. */
	void SetForceHideNode(bool bHide) { bForceHideNode = bHide; }

	/** Should this node be drawn? */
	bool ShouldDrawNode() const { return !bForceHideNode; }

	bool ShouldUseBorderHighlight() const { return bUseBorderHighlight; }
	void SetUseBorderHighlight(bool bUse) { bUseBorderHighlight = bUse; }

	/** Helper constants to get the names of some properties. Used by the DlgSystemEditor module. */
	static FName GetMemberNameDialogueNode() { return GET_MEMBER_NAME_CHECKED(UDialogueGraphNode, DialogueNode); }
	static FName GetMemberNameNodeIndex() { return GET_MEMBER_NAME_CHECKED(UDialogueGraphNode, NodeIndex); }

protected:
	// Begin UDialogueGraphNode_Base interface
	/** Creates the input pin for this node. */
	virtual void CreateInputPin()
	{
		static const FName PinName(TEXT("Dlg Input Pin"));
		FCreatePinParams PinParams;
		PinParams.Index = INDEX_PIN_Input;
		CreatePin(EGPD_Input, UDialogueGraphSchema::PIN_CATEGORY_Input, PinName, PinParams);
	}

	/** Creates the output pin for this node. */
	virtual void CreateOutputPin()
	{
		static const FName PinName(TEXT("Dlg Output Pin"));
		FCreatePinParams PinParams;
		PinParams.Index = INDEX_PIN_Output;
		CreatePin(EGPD_Output, UDialogueGraphSchema::PIN_CATEGORY_Output, PinName, PinParams);

		// This enables or disables dragging of the pin from the Node, see SGraphPin::OnPinMouseDown for details
		GetOutputPin()->bNotConnectable = !CanHaveOutputConnections();
	}

	/** Registers all the listener this class listens to. */
	void RegisterListeners() override;

	//
	// Begin own functions
	//

	/** This function is called after one of the properties of the DialogueNode are changed.  */
	void OnDialogueNodePropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent, int32 EdgeIndexChanged);

	/** Make sure the DialogueNode is owned by the Dialogue */
	void ResetDialogueNodeOwner();

private:
	/** Returns the different Edge Index in the DialogueNode.Children that differs from its corresponding OutputPin.LinkedTo. */
	const FDiffNodeEdgeLinkedToPinResult FindDifferenceBetweenNodeEdgesAndLinkedToPins() const;

	/** Tells us if the Edge matches the Graph Output Connections at the same Index in the OutputPin. They point to the same TargetIndex */
	bool DoesEdgeMatchEdgeIndex(const FDlgEdge& Edge, int32 EdgeIndex, FString& OutMessage) const;

protected:
	/** The Dialogue Node this graph node references.  */
	UPROPERTY(EditAnywhere, Instanced, Category = DialogueGraphNode, Meta = (ShowOnlyInnerProperties))
	UDlgNode* DialogueNode;

	/** The Dialogue Node index in the Dialogue (array) this represents. This is not relevant for the StartNode. */
	UPROPERTY(VisibleAnywhere, Category = DialogueGraphNode)
	int32 NodeIndex = INDEX_NONE;

	/** Indicates the distance from the start node. This is only set after the graph is compiled. */
	UPROPERTY()
	int32 NodeDepth = INDEX_NONE;

	/** Used to highlight the node if the currently selected node is a proxy targeting it */
	UPROPERTY(Transient)
	bool bUseBorderHighlight = false;

	/** Forcefully hide this node from the graph. */
	bool bForceHideNode = false;
};
