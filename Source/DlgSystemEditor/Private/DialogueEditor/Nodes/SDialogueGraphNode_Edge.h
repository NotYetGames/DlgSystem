// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SGraphNode.h"

#include "DialogueGraphNode_Edge.h"
#include "SDialogueGraphNode_Base.h"

/**
 * Widget for UDialogueGraphNode_Edge
 */
class SDialogueGraphNode_Edge : public SDialogueGraphNode_Base
{
	typedef SDialogueGraphNode_Base Super;
	typedef SDialogueGraphNode_Edge Self;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialogueGraphNode_Edge* InNode);

	// Begin SWidget interface

	/**
	 * The system will use this event to notify a widget that the cursor has entered it. This event is uses a custom bubble strategy.
	 *
	 * @param MyGeometry The Geometry of the widget receiving the event
	 * @param MouseEvent Information about the input event
	 */
	void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/**
	 * The system will use this event to notify a widget that the cursor has left it. This event is uses a custom bubble strategy.
	 *
	 * @param MouseEvent Information about the input event
	 */
	void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	// End SWidget interface

	// Begin SNodePanel::SNode Interface

	/**
	 * @param NewPosition	The Node should be relocated to this position in the graph panel
	 * @param NodeFilter		Set of nodes to prevent movement on, after moving successfully a node is added to this set.
	 */
	void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter) override;

	/** Returns true if this node is dependent on the location of other nodes (it can only depend on the location of first-pass only nodes) */
	bool RequiresSecondPassLayout() const override { return true; }

	/** Performs second pass layout; only called if RequiresSecondPassLayout returned true */
	void PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& InNodeToWidgetLookup) const override;

	/** Populate the widgets array with any overlay widgets to render */
	TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;
	// End SNodePanel::SNode Interface

	// Begin SGraphNode Interface
	/** Update this GraphNode to match the data that it is observing */
	void UpdateGraphNode() override;
	// End SGraphNode Interface

	// Begin SDialogueGraphNode_Base Interface
	EVisibility GetNodeVisibility() const override
	{
		return DialogueGraphNode_Edge && DialogueGraphNode_Edge->ShouldDrawEdge() ? EVisibility::Visible : EVisibility::Hidden;
	}
	// End SDialogueGraphNode_Base Interface

	// Begin own functions
	/** Calculate position for multiple nodes to be placed between a start and end point, by providing this nodes index and max expected nodes */
	void PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;

	/** Gets the transition color of this node and wire. */
	FSlateColor GetTransitionColor() const { return DialogueGraphNode_Edge->GetEdgeColor(IsHovered()); }

	// End own functions

protected:
	/** Gets the tooltip for the condition overlay. */
	FText GetConditionOverlayTooltipText() const;

	/** Get the visibility of the overlay widgets. */
	EVisibility GetOverlayWidgetVisibility() const;

	/** Get the background color to display for the widget overlay. This changes on hover state of sibling nodes */
	FSlateColor GetOverlayWidgetBackgroundColor(bool bHovered) const
	{
		return DialogueGraphNode_Edge->GetEdgeColor(bHovered);
	}

protected:
	// The dialogue Edge this widget represents
	UDialogueGraphNode_Edge* DialogueGraphNode_Edge = nullptr;

	/** The widget we use to display if the edge has any conditions */
	TSharedPtr<SWidget> ConditionOverlayWidget;
};
