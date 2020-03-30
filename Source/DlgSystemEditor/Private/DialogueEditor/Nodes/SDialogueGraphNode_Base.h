// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"

#include "DialogueGraphNode.h"
#include "DlgSystemSettings.h"

class SVerticalBox;

/**
 * Widget for UDialogueGraphNode_Base
 */
class SDialogueGraphNode_Base : public SGraphNode
{
	typedef SGraphNode Super;
	typedef SDialogueGraphNode_Base Self;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialogueGraphNode_Base* InNode);

	// Begin SGraphNode Interface
	/** Create the widgets for pins on the node */
	void CreatePinWidgets() override;

	/** Create a single pin widget */
	void CreateStandardPinWidget(UEdGraphPin* Pin) override;

	/** Update this GraphNode to match the data that it is observing */
	void UpdateGraphNode() override;

	/** @param OwnerPanel  The GraphPanel that this node belongs to */
	void SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel) override;
	// End SGraphNode Interface

	// Begin own methods

	/** Is the current node visible? */
	virtual EVisibility GetNodeVisibility() const { return EVisibility::Visible; }

protected:
	// SGraphNode Interface
	/**
	 * Add a new pin to this graph node. The pin must be newly created.
	 *
	 * @param PinToAdd   A new pin to add to this GraphNode.
	 */
	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

	/** Hook that allows derived classes to supply their own SGraphPin derivatives for any pin. Used by CreateStandardPinWidget. */
	TSharedPtr<SGraphPin> CreatePinWidget(UEdGraphPin* Pin) const override;

private:
	/** Tells us if the provided pin is valid.  */
	bool IsValidPin(UEdGraphPin* Pin)
	{
		return ensureMsgf(Pin->GetOuter() == GraphNode,
			TEXT("Graph node ('%s' - %s) has an invalid %s pin: '%s'; (with a bad %s outer: '%s'); skiping creation of a widget for this pin."),
			*GraphNode->GetNodeTitle(ENodeTitleType::ListView).ToString(),
			*GraphNode->GetPathName(),
			Pin->Direction == EEdGraphPinDirection::EGPD_Input ? TEXT("input") : TEXT("output"),
			Pin->PinFriendlyName.IsEmpty() ? *Pin->PinName.ToString() : *Pin->PinFriendlyName.ToString(),
			Pin->GetOuter() ? *Pin->GetOuter()->GetClass()->GetName() : TEXT("UNKNOWN"),
			Pin->GetOuter() ? *Pin->GetOuter()->GetPathName() : TEXT("NULL"));
	}

protected:
	// The Base dialogue node this widget represents
	UDialogueGraphNode_Base* DialogueGraphNode_Base = nullptr;

	// Cache the Dialogue settings
	const UDlgSystemSettings* Settings = nullptr;

	/** The area where output/input pins reside */
	TSharedPtr<SVerticalBox> PinsNodeBox;
};
