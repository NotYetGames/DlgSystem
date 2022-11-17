// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"

#include "DlgSystemEditor/Editor/Graph/DialogueGraphSchema.h"
#include "DlgSystemEditor/Editor/Graph/DialogueGraph.h"

#include "DialogueGraphNode_Base.generated.h"

class UEdGraphPin;
class UEdGraphSchema;
class UDialogueGraphSchema;

/**
 * Represents the base class representation of the dialogue graph nodes.
 * Each dialogue graph node has only one input/output pin. And each pin can be linked to multiple nodes.
 */
UCLASS(Abstract)
class DLGSYSTEMEDITOR_API UDialogueGraphNode_Base : public UEdGraphNode
{
	GENERATED_BODY()

public:
	//~ Begin UObject Interface.
	/**
	 * Do any object-specific cleanup required immediately after loading an object,
	 * and immediately after any undo/redo.
	 */
	void PostLoad() override;

	/**
	 *  Called after duplication & serialization and before PostLoad. Used to e.g. make sure UStaticMesh's UModel gets copied as well.
	 *  Note: NOT called on components on actor duplication (alt-drag or copy-paste).  Use PostEditImport as well to cover that case.
	 */
	void PostDuplicate(bool bDuplicateForPIE) override;

	/**
	 * Called after importing property values for this object (paste, duplicate or .t3d import)
	 * Allow the object to perform any cleanup for properties which shouldn't be duplicated or
	 * are unsupported by the script serialization
	 */
	void PostEditImport() override;

	// UEdGraphNode interface.
	/**
	 * A chance to initialize a new node; called just once when a new node is created, before AutowireNewNode or AllocateDefaultPins is called.
	 * This method is not called when a node is reconstructed, etc...
	 */
	void PostPlacedNewNode() override { RegisterListeners(); }

	/** Allocate default pins for a given node, based only the NodeType, which should already be filled in. */
	void AllocateDefaultPins() override;

	/** Refresh the connectors on a node, preserving as many connections as it can. */
	void ReconstructNode() override;

	/** Whether or not this node can be safely duplicated (via copy/paste, etc...) in the graph */
	bool CanDuplicateNode() const override { return true; }

	/** Whether or not this node can be deleted by user action */
	bool CanUserDeleteNode() const override { return true; }

	/** Perform any steps necessary prior to copying a node into the paste buffer */
	void PrepareForCopying() override { Super::PrepareForCopying(); }

	/** IGNORED. Removes the specified pin from the node, preserving remaining pin ordering. */
	void RemovePinAt(int32 PinIndex, EEdGraphPinDirection PinDirection) override {}

	/** Whether or not struct pins belonging to this node should be allowed to be split or not. */
	bool CanSplitPin(const UEdGraphPin* Pin) const override { return false; }

	/** Determine if this node can be created under the specified schema */
	bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const override
	{
		return Schema->IsA(UDialogueGraphSchema::StaticClass());
	}

	/** Returns the link used for external documentation for the graph node. */
	FString GetDocumentationLink() const override { return TEXT("Shared/DialogueGraphNode"); }

	/** Should we show the Palette Icon for this node on the node title */
	bool ShowPaletteIconOnNode() const override { return true; }

	/** Gets the draw color of a node's title bar. */
	FLinearColor GetNodeTitleColor() const override { return GetNodeBackgroundColor(); }

	/** @return Icon to use in menu or on node */
	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override
	{
		static const FSlateIcon Icon = FSlateIcon(NY_GET_APP_STYLE_NAME(), "Graph.StateNode.Icon");
		OutColor = GetNodeBackgroundColor();
		return Icon;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Begin own functions
	UDialogueGraphNode_Base(const FObjectInitializer& ObjectInitializer);

	/** Perform any fixups (deep copies of associated data, etc...) necessary after a node has been copied in the editor. */
	virtual void PostCopyNode() {}

	/** Checks if this node has a output connection to the TargetNode. */
	virtual bool HasOutputConnectionToNode(const UEdGraphNode* TargetNode) const;

	/** Checks whether an input connection can be added to this node */
	virtual bool CanHaveInputConnections() const { return true; }

	/** Checks whether an output connection can be added from this node */
	virtual bool CanHaveOutputConnections() const { return true; }

	/** Gets the background color of this node. */
	virtual FLinearColor GetNodeBackgroundColor() const { return FLinearColor::Black; }

	/** Performs all checks */
	virtual void CheckAll() const
	{
#if DO_CHECK
		GetInputPin();
		GetOutputPin();
#endif
	}

	/** Gets the position in the Graph canvas of this node. */
	virtual FIntPoint GetPosition() const { return FIntPoint(NodePosX, NodePosY); }

	/** Sets the position in the Graph canvas of this node. */
	virtual void SetPosition(int32 X, int32 Y)
	{
		NodePosX = X;
		NodePosY = Y;
	}

	// Compiler methods
	/** Clears the compiler messages on this node. */
	void ClearCompilerMessage();

	/** Sets a compiler message of type warning. */
	void SetCompilerWarningMessage(FString Message);

	/** Is the Input pin initialized? */
	bool HasInputPin() const
	{
		return Pins.IsValidIndex(INDEX_PIN_Input) && Pins[INDEX_PIN_Input] != nullptr &&
			   Pins[INDEX_PIN_Input]->Direction == EGPD_Input;
	}

	/** Is the Output pin initialized? */
	bool HasOutputPin() const
	{
		return Pins.IsValidIndex(INDEX_PIN_Output) && Pins[INDEX_PIN_Output] != nullptr &&
			   Pins[INDEX_PIN_Output]->Direction == EGPD_Output;
	}

	// @return the input pin for this dialogue Node
	UEdGraphPin* GetInputPin() const
	{
		check(HasInputPin());
		return Pins[INDEX_PIN_Input];
	}

	// @return the output pin for this dialogue Node
	UEdGraphPin* GetOutputPin() const
	{
		check(HasOutputPin());
		return Pins[INDEX_PIN_Output];
	}

	// Helper method to get directly the Dialogue Graph (which is our parent)
	UDialogueGraph* GetDialogueGraph() const { return CastChecked<UDialogueGraph>(GetGraph()); }

	// Helper method to get directly the Dialogue
	UDlgDialogue* GetDialogue() const { return GetDialogueGraph()->GetDialogue(); }

	// TODO fix UEdGraphSchema::BreakSinglePinLink, make it to const
	/** Helper method to get directly the Dialogue Graph Schema */
	const UDialogueGraphSchema* GetDialogueGraphSchema() const { return GetDialogueGraph()->GetDialogueGraphSchema(); }

	/** Widget representing this node if it exists */
	TSharedPtr<SGraphNode> GetNodeWidget() const { return DEPRECATED_NodeWidget.Pin(); }

protected:
	// Begin own functions
	/** Creates the input pin for this node. */
	virtual void CreateInputPin() { unimplemented(); }

	/** Creates the output pin for this node. */
	virtual void CreateOutputPin() { unimplemented(); }

	/** This function is called after one of the properties of the Dialogue are changed.  */
	virtual void OnDialoguePropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent) {}

	/** Registers all the listener this class listens to. */
	virtual void RegisterListeners();

protected:
	// Constants for the location of the input/output pins in the Pins array
	static constexpr int32 INDEX_PIN_Input = 0;
	static constexpr int32 INDEX_PIN_Output = 1;
};
