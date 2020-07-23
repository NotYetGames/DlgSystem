// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreTypes.h"

#include "DialogueGraphNode_Base.h"
#include "DialogueGraphNode.h"

#include "DialogueGraphNode_Edge.generated.h"


/**
 * Represents the graph node for edges, corresponds to the runtime FDlgEdge.
 * Allows us to have selectable edges.
 * This class has an additional constraint over the base class UDialogueGraphNode_Base such that the input and output pin
 * have only one connection (aka Pin.LinkedTo.Num() == 1 at all times) otherwise this does not make sense to simulate an Edge.
 */
UCLASS()
class UDialogueGraphNode_Edge : public UDialogueGraphNode_Base
{
	GENERATED_BODY()

public:
	// Begin UObject Interface.
	/**
	 * Called when a property on this object has been modified externally
	 *
	 * @param PropertyChangedEvent the property that was modified
	 */
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	/**
	 * Note that the object will be modified. If we are currently recording into the
	 * transaction buffer (undo/redo), save a copy of this object into the buffer and
	 * marks the package as needing to be saved.
	 *
	 * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
	 *								currently recording an active undo/redo transaction
	 * @return true if the object was saved to the transaction buffer
	 */
	bool Modify(bool bAlwaysMarkDirty = true) override;

	/**
	 * Same as the above method but it only calls the base class.
	 * Useful to not make an infinite loop when Modify is called from the ParentNode
	 */
	bool SuperModify(bool bAlwaysMarkDirty = true) { return Super::Modify(bAlwaysMarkDirty); }

	// UEdGraphNode interface.
	/** Allocate default pins for a given node, based only the NodeType, which should already be filled in. */
	void AllocateDefaultPins() override;

	/** Gets the name of this node, shown in title bar */
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	/** Gets the tooltip to display when over the node */
	FText GetTooltipText() const override;

	/** Called when the connection list of one of the pins of this node is changed in the editor */
	void PinConnectionListChanged(UEdGraphPin* Pin) override;

	/** Perform any fixups (deep copies of associated data, etc...) necessary after a node has been pasted in the editor */
	void PostPasteNode() override;

	/** @return Icon to use in menu or on node */
	FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override
	{
		static const FSlateIcon Icon = FSlateIcon(FEditorStyle::GetStyleSetName(), "Graph.TransitionNode.Icon");
		OutColor = GetNodeBackgroundColor();
		return Icon;
	}

	// Begin UDialogueGraphNode_Base interface
	/** Gets the background color of this node. */
	FLinearColor GetNodeBackgroundColor() const override { return FColorList::White; }

	// Begin own functions
	/** Does this edge have a parent node? */
	bool HasParentNode() const
	{
		if (HasInputPin())
		{
			UEdGraphPin* InputPin = GetInputPin();
			return InputPin->LinkedTo.Num() == 1 && InputPin->LinkedTo[0] != nullptr &&
				   InputPin->LinkedTo[0]->GetOwningNodeUnchecked() != nullptr;
		}

		return false;
	}

	/** Does this edge has a child edge? */
	bool HasChildNode() const
	{
		if (HasOutputPin())
		{
			UEdGraphPin* OutputPin = GetOutputPin();
			return OutputPin->LinkedTo.Num() == 1 && OutputPin->LinkedTo[0] != nullptr &&
				   OutputPin->LinkedTo[0]->GetOwningNodeUnchecked() != nullptr;
		}

		return false;
	}

	/** Gets the parent node that is connected to the input pin. */
	UDialogueGraphNode* GetParentNode() const
	{
		check(HasParentNode());
		return CastChecked<UDialogueGraphNode>(GetInputPin()->LinkedTo[0]->GetOwningNode());
	}

	/** Gets the child node that is connected from the output pin. */
	UDialogueGraphNode* GetChildNode() const
	{
		check(HasChildNode());
		return CastChecked<UDialogueGraphNode>(GetOutputPin()->LinkedTo[0]->GetOwningNode());
	}

	/** Creates a connection between the two provided nodes */
	void CreateConnections(UDialogueGraphNode* ParentNode, UDialogueGraphNode* ChildNode);

	// Begin own function
	/** Gets the corresponding Dialogue Edge of this Node as a const. */
	const FDlgEdge& GetDialogueEdge() const { return DialogueEdge; }
	FDlgEdge& GetDialogueEdge() { return DialogueEdge; }

	/** Sets the corresponding Dialogue Edge of this Node. */
	void SetDialogueEdge(const FDlgEdge& InEdge) { DialogueEdge = InEdge; }

	/** Sets the DialogueEdge.TargetIndex */
	void SetDialogueEdgeTargetIndex(int32 InIndex) { DialogueEdge.TargetIndex = InIndex; }

	/** Sets the DialogueEdge.Text */
	void SetDialogueEdgeText(const FText& InText)
	{
		DialogueEdge.SetText(InText);
	}

	/** Tells us if this edge has any conditions set. */
	bool HasConditions() const { return DialogueEdge.Conditions.Num() > 0; }

	/** Gets the current edge color. */
	FLinearColor GetEdgeColor(bool bIsHovered) const;

	/** Is this node a primary edge? */
	bool IsPrimaryEdge() const { return bIsPrimaryEdge; }

	/** Sets the primary edge status of this Node. */
	void SetIsPrimaryEdge(bool bValue) { bIsPrimaryEdge = bValue; }

	/** Should this edge be drawn? */
	bool ShouldDrawEdge() const
	{
		// If The parent or the child is hidden, we also hide this edge no matter the settings
		if (HasInputPin() && HasOutputPin())
		{
			if (!GetParentNode()->ShouldDrawNode() || !GetChildNode()->ShouldDrawNode())
			{
				return false;
			}
		}

		const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
		if (Settings->bShowPrimarySecondaryEdges)
		{
			return IsPrimaryEdge() ? Settings->bDrawPrimaryEdges : Settings->bDrawSecondaryEdges;
		}

		return true;
	}
	// End own functions

protected:
	// Begin UDialogueGraphNode_Base interface
	/** Creates the input pin for this node. */
	void CreateInputPin() override
	{
		static const FName PinName(TEXT("Input"));
		static const FName CategoryName(TEXT("Transition"));
		FCreatePinParams PinParams;
		PinParams.Index = INDEX_PIN_Input;
		CreatePin(EGPD_Input, CategoryName, PinName, PinParams);
	}

	/** Creates the output pin for this node. */
	void CreateOutputPin() override
	{
		static const FName PinName(TEXT("Output"));
		static const FName CategoryName(TEXT("Transition"));
		FCreatePinParams PinParams;
		PinParams.Index = INDEX_PIN_Output;
		CreatePin(EGPD_Output, CategoryName, PinName, PinParams);
	}

private:
	// Begin own functions
	/** Gets the corresponding FDlgEdge that this Node actually represents from the ParentNode */
	FDlgEdge* GetMutableDialogueEdgeFromParentNode() const;

private:
	/** The copy Dialogue Edge corresponding to this graph node. This belongs to the the Node of the Input Pin (GetParentNode) */
	UPROPERTY(EditAnywhere, Category = DialogueGraphNode, Meta = (ShowOnlyInnerProperties))
	FDlgEdge DialogueEdge;

	/**
	 * Is this a primary edge? Aka does this edge lead to a unique path to the ChildNode.
	 * This is only set after the graph is compiled.
	 */
	UPROPERTY()
	bool bIsPrimaryEdge = true;
};
