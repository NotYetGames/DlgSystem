// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "SGraphNode.h"
#include "SGraphPanel.h"

#include "DialogueGraphNode.h"
#include "SDialogueGraphNode_Base.h"

class SVerticalBox;

/**
 * Widget for UDialogueGraphNode
 */
class SDialogueGraphNode : public SDialogueGraphNode_Base
{
	typedef SDialogueGraphNode_Base Super;
	typedef SDialogueGraphNode Self;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDialogueGraphNode* InNode);

	// Begin SWidget interface
	void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		Super::OnDragEnter(MyGeometry, DragDropEvent);
	}
	void OnDragLeave(const FDragDropEvent& DragDropEvent) override
	{
		Super::OnDragLeave(DragDropEvent);
	}

	FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		return Super::OnDragOver(MyGeometry, DragDropEvent);
	}
	FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		return Super::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
	}
	FReply OnMouseMove(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent) override
	{
		return Super::OnMouseMove(SenderGeometry, MouseEvent);
	}
	// End of SWidget interface

	// Begin SNodePanel::SNode Interface

	/** Populate the widgets array with any overlay widgets to render */
	TArray<FOverlayWidgetInfo> GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const override;

	/** Populate the brushes array with any overlay brushes to render */
	void GetOverlayBrushes(bool bSelected, const FVector2D WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const override;
	// End SNodePanel::SNode Interface

	// Begin SGraphNode Interface

	/** Update this GraphNode to match the data that it is observing */
	void UpdateGraphNode() override;
	// End SGraphNode Interface

	// Begin SDialogueGraphNode_Base Interface
	EVisibility GetNodeVisibility() const override
	{
		return DialogueGraphNode && DialogueGraphNode->ShouldDrawNode() ? EVisibility::Visible : EVisibility::Hidden;
	}
	// End SDialogueGraphNode_Base Interface

protected:
	//
	// SGraphNode Interface
	//

	/** Override this to provide support for an 'expensive' tooltip widget that is only built on demand */
	TSharedPtr<SToolTip> GetComplexTooltip() override { return Super::GetComplexTooltip(); }

	/** Should we use low-detail node titles? Used by UpdateGraphNode() */
	bool UseLowDetailNodeTitles() const override
	{
		if (const SGraphPanel* MyOwnerPanel = GetOwnerPanel().Get())
		{
			return MyOwnerPanel->GetCurrentLOD() <= EGraphRenderingLOD::LowestDetail;
		}

		return false;
	}

	/** Return the desired comment bubble color */
	FSlateColor GetCommentColor() const override { return DialogueGraphNode->GetNodeBackgroundColor(); }

	/* Populate a meta data tag with information about this graph node. sed by UpdateGraphNode() */
	void PopulateMetaTag(class FGraphNodeMetaData* TagMeta) const override { Super::PopulateMetaTag(TagMeta); }

	//
	// Begin own functions
	//

	/** Gets/Creates the inner node content area. Used by UpdateGraphNode() */
	TSharedRef<SWidget> GetNodeBodyWidget();

	/** Gets the actual title widget to display */
	TSharedRef<SWidget> GetTitleWidget();

	/** Gets the actual description widget to display */
	TSharedRef<SWidget> GetDescriptionWidget();

	// Gets the background color for a node
	FSlateColor GetBackgroundColor() const { return DialogueGraphNode->GetNodeBackgroundColor(); }

	// Gets the main description of this Node.
	FText GetDescription() const
	{
		if (DialogueGraphNode && DialogueGraphNode->IsDialogueNodeSet())
		{
			return DialogueGraphNode->GetDialogueNode().GetNodeUnformattedText();
		}

		return FText::GetEmpty();
	}

	// Gets all speech sequence entries for the Node of type Speech Sequence
	const TArray<FDlgSpeechSequenceEntry>& GetSpeechSequenceEntries() const
	{
		return DialogueGraphNode->GetDialogueNode<UDlgNode_SpeechSequence>().GetNodeSpeechSequence();
	}

	// Gets the speech sequence description for the entry at index SpeechEntryIndex
	FText GetDescriptionForSpeechSequenceEntryAt(int32 SpeechEntryIndex) const
	{
		if (DialogueGraphNode && DialogueGraphNode->IsSpeechSequenceNode())
		{
			return GetSpeechSequenceEntries()[SpeechEntryIndex].Text;
		}

		return FText::GetEmpty();
	}

	// Gets the speaker (owner) of speech sequence for the entry at index SpeechEntryIndex
	FText GetSpeakerForSpeechSequenceEntryAt(int32 SpeechEntryIndex) const
	{
		if (DialogueGraphNode && DialogueGraphNode->IsSpeechSequenceNode())
		{
			return FText::FromName(GetSpeechSequenceEntries()[SpeechEntryIndex].Speaker);
		}

		return FText::GetEmpty();
	}

	// Gets the visibility of the Description
	EVisibility GetDescriptionVisibility() const
	{
		// LOD this out once things get too small
		TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
		return !MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail ? EVisibility::Visible : EVisibility::Collapsed;
	}

	/** Gets the text to display in the index overlay */
	FText GetIndexText() const { return FText::AsNumber(DialogueGraphNode->GetDialogueNodeIndex()); }

	/** Gets the tooltip for the index overlay */
	FText GetIndexOverlayTooltipText() const;

	/** Gets the tooltip for the condition overelay. */
	FText GetConditionOverlayTooltipText() const;

	/** Gets the tooltip for the event overlay. */
	FText GetEventOverlayTooltipText() const;

	/** Gets the tooltip for the voice overlay. */
	FText GetVoiceOverlayTooltipText() const;

	/** Gets the tooltip for the voice overlay. */
	FText GetGenericOverlayTooltipText() const;

	/** Get the visibility of the overlay widgets. */
	EVisibility GetOverlayWidgetVisibility() const;

	/** Get the background color to display for the widget overlay. This changes on hover state of sibling nodes */
	FSlateColor GetOverlayWidgetBackgroundColor(bool bHovered) const
	{
		// Hovered is gray
		return bHovered ? Settings->BorderHoveredBackgroundColor : Settings->BorderBackgroundColor;
	}

	/** Handle hover state changing for the index widget - we use this to highlight sibling nodes */
	void OnIndexHoverStateChanged(bool bHovered);
	// End own functions

protected:
	// The dialogue this view represents
	UDialogueGraphNode* DialogueGraphNode = nullptr;

	/** The node body widget, cached here so we can determine its size when we want ot position our overlays */
	TSharedPtr<SBorder> NodeBodyWidget;

	/** The widget that holds the title section */
	TSharedPtr<SWidget> TitleWidget;

	/** The widget that hold the description text */
	TSharedPtr<SWidget> DescriptionWidget;

	/** The widget we use to display the index of the node */
	TSharedPtr<SWidget> IndexOverlayWidget;

	/** The widget we use to display if the node has any enter conditions */
	TSharedPtr<SWidget> ConditionOverlayWidget;

	/** The widget we use to display if the node has any enter events */
	TSharedPtr<SWidget> EventOverlayWidget;

	/** The widget we use to display if the node has voice/sound variables set */
	TSharedPtr<SWidget> VoiceOverlayWidget;

	/** The widget we use to display if the node has the GenericData variable set */
	TSharedPtr<SWidget> GenericOverlayWidget;
};
