// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgGraphNode.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SToolTip.h"
#include "GraphEditorSettings.h"
#include "SCommentBubble.h"
#include "SLevelOfDetailBranchNode.h"
#include "IDocumentation.h"
#include "GraphEditorDragDropAction.h"

#include "SDlgNodeOverlayWidget.h"
#include "SDlgGraphPin.h"
#include "DlgSystemEditor/DlgStyle.h"

#define LOCTEXT_NAMESPACE "DialogueEditor"


/////////////////////////////////////////////////////
// SDlgGraphNode
void SDlgGraphNode::Construct(const FArguments& InArgs, UDialogueGraphNode* InNode)
{
	Super::Construct(Super::FArguments(), InNode);
	DialogueGraphNode = InNode;

	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

FReply SDlgGraphNode::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
//	const bool bReadOnly = OwnerGraphPanelPtr.IsValid() ? !OwnerGraphPanelPtr.Pin()->IsGraphEditable() : false;
//	const TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
//	if (!Operation.IsValid() || bReadOnly)
//		return FReply::Unhandled();

	// Is someone dropping a connection onto this node?
//	if (Operation->IsOfType<FGraphEditorDragDropAction>())
//	{
//		TSharedPtr<FGraphEditorDragDropAction> DragConnectionOp = StaticCastSharedPtr<FGraphEditorDragDropAction>(Operation);
//	}

	return Super::OnDrop(MyGeometry, DragDropEvent);
}

FReply SDlgGraphNode::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (DialogueGraphNode == nullptr)
	{
		return Super::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
	}

	const UDlgNode_Proxy* Proxy = Cast<UDlgNode_Proxy>(&DialogueGraphNode->GetDialogueNode());
	if (Proxy == nullptr)
	{
		return Super::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
	}

	const int32 TargetIndex = Proxy->GetTargetNodeIndex();
	const TArray<UDlgNode*>& Nodes = DialogueGraphNode->GetDialogue()->GetNodes();
	if (Nodes.IsValidIndex(TargetIndex))
	{
		FDlgEditorUtilities::JumpToGraphNode(Nodes[TargetIndex]->GetGraphNode());
		return FReply::Handled();
	}

	return Super::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin SNodePanel::SNode Interface
TArray<FOverlayWidgetInfo> SDlgGraphNode::GetOverlayWidgets(bool bSelected, const FVector2D& WidgetSize) const
{
	check(IndexOverlayWidget.IsValid());
	check(ConditionOverlayWidget.IsValid());
	check(EventOverlayWidget.IsValid());
	check(VoiceOverlayWidget.IsValid());
	check(GenericOverlayWidget.IsValid());

	TArray<FOverlayWidgetInfo> Widgets;
	static constexpr float DistanceBetweenWidgetsY = 1.5f;
	FVector2D OriginRightSide(0.0f, 0.0f);
	FVector2D OriginLeftSide(0.0f, 0.0f);

	// Add Index overlay
	{
		// Position on the right of the node
		FOverlayWidgetInfo Overlay(IndexOverlayWidget);
		Overlay.OverlayOffset = FVector2D(WidgetSize.X - IndexOverlayWidget->GetDesiredSize().X / 2.0f, OriginRightSide.Y);
		Widgets.Add(Overlay);
		OriginRightSide.Y += IndexOverlayWidget->GetDesiredSize().Y + DistanceBetweenWidgetsY;
	}

	// Add Voice overlay
	if (Settings->bShowHasVoiceIcon && DialogueGraphNode->HasVoicePropertiesSet())
	{
		// Position on the right of the node
		FOverlayWidgetInfo Overlay(VoiceOverlayWidget);
		Overlay.OverlayOffset = FVector2D(WidgetSize.X - VoiceOverlayWidget->GetDesiredSize().X / 3.0f, OriginRightSide.Y);
		Widgets.Add(Overlay);
		OriginRightSide.Y += VoiceOverlayWidget->GetDesiredSize().Y + DistanceBetweenWidgetsY;
	}

	// Add Generic overlay
	if (Settings->bShowHasGenericDataIcon && DialogueGraphNode->HasGenericDataSet())
	{
		// Position on the right of the node
		FOverlayWidgetInfo Overlay(GenericOverlayWidget);
		Overlay.OverlayOffset = FVector2D(WidgetSize.X - GenericOverlayWidget->GetDesiredSize().X / 3.0f, OriginRightSide.Y);
		Widgets.Add(Overlay);
		OriginRightSide.Y += GenericOverlayWidget->GetDesiredSize().Y + DistanceBetweenWidgetsY;
	}

	// Add Condition overlay
	if (Settings->bShowHasEnterConditionsIcon && DialogueGraphNode->HasEnterConditions())
	{
		// Position on the left of the node
		FOverlayWidgetInfo Overlay(ConditionOverlayWidget);
		Overlay.OverlayOffset = FVector2D(-ConditionOverlayWidget->GetDesiredSize().X / 1.5f, OriginLeftSide.Y);
		Widgets.Add(Overlay);
		OriginLeftSide.Y += ConditionOverlayWidget->GetDesiredSize().Y + DistanceBetweenWidgetsY;
	}

	// Add Event overlay
	if (Settings->bShowHasEnterEventsIcon && DialogueGraphNode->HasEnterEvents())
	{
		// Position on the left of the node
		FOverlayWidgetInfo Overlay(EventOverlayWidget);
		Overlay.OverlayOffset = FVector2D(-EventOverlayWidget->GetDesiredSize().X / 1.5f, OriginLeftSide.Y);
		Widgets.Add(Overlay);
		OriginLeftSide.Y += EventOverlayWidget->GetDesiredSize().Y + DistanceBetweenWidgetsY;
	}

	return Widgets;
}

void SDlgGraphNode::GetOverlayBrushes(bool bSelected, const FVector2D WidgetSize, TArray<FOverlayBrushInfo>& Brushes) const
{
	Super::GetOverlayBrushes(bSelected, WidgetSize, Brushes);
}

// End SNodePanel::SNode Interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin SGraphNode interface
void SDlgGraphNode::UpdateGraphNode()
{
	Super::UpdateGraphNode();
	SetupErrorReporting();

	// Define other useful variables
	const FMargin NodePadding = 10.0f;

	// Used in GetOverlayWidgets
	IndexOverlayWidget = SNew(SDlgNodeOverlayWidget)
		.OverlayBody(
			SNew(STextBlock)
			.Text(this, &Self::GetIndexText)
			.ColorAndOpacity(FLinearColor::White)
			.Font(FNYAppStyle::GetFontStyle("BTEditor.Graph.BTNode.IndexText"))
		)
		.ToolTipText(this, &Self::GetIndexOverlayTooltipText)
		.Visibility(this, &Self::GetOverlayWidgetVisibility)
		.OnHoverStateChanged(this, &Self::OnIndexHoverStateChanged)
		.OnGetBackgroundColor(this, &Self::GetOverlayWidgetBackgroundColor);

	// Fit to content
	static constexpr int WidthOverride = 24;
	static constexpr int HeightOverride = 24;
	ConditionOverlayWidget = SNew(SDlgNodeOverlayWidget)
		.OverlayBody(
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.WidthOverride(WidthOverride)
			.HeightOverride(HeightOverride)
			[
				SNew(SImage)
				.Image(FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_ConditionIcon))
			]
		)
		.ToolTipText(this, &Self::GetConditionOverlayTooltipText)
		.Visibility(this, &Self::GetOverlayWidgetVisibility)
		.OnGetBackgroundColor(this, &Self::GetOverlayWidgetBackgroundColor);

	EventOverlayWidget = SNew(SDlgNodeOverlayWidget)
		.OverlayBody(
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.WidthOverride(WidthOverride)
			.HeightOverride(HeightOverride)
			[
				SNew(SImage)
				.Image(FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_EventIcon))
			]
		)
		.ToolTipText(this, &Self::GetEventOverlayTooltipText)
		.Visibility(this, &Self::GetOverlayWidgetVisibility)
		.OnGetBackgroundColor(this, &Self::GetOverlayWidgetBackgroundColor);

	VoiceOverlayWidget = SNew(SDlgNodeOverlayWidget)
		.OverlayBody(
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.WidthOverride(WidthOverride)
			.HeightOverride(HeightOverride)
			[
				SNew(SImage)
				.Image(FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_VoiceIcon))
			]
		)
		.ToolTipText(this, &Self::GetVoiceOverlayTooltipText)
		.Visibility(this, &Self::GetOverlayWidgetVisibility)
		.OnGetBackgroundColor(this, &Self::GetOverlayWidgetBackgroundColor);

	GenericOverlayWidget = SNew(SDlgNodeOverlayWidget)
		.OverlayBody(
			SNew(SBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.WidthOverride(WidthOverride)
			.HeightOverride(HeightOverride)
			[
				SNew(SImage)
				.Image(FDlgStyle::Get()->GetBrush(FDlgStyle::PROPERTY_GenericIcon))
			]
		)
		.ToolTipText(this, &Self::GetGenericOverlayTooltipText)
		.Visibility(this, &Self::GetOverlayWidgetVisibility)
		.OnGetBackgroundColor(this, &Self::GetOverlayWidgetBackgroundColor);

	// Set Default tooltip
	if (!SWidget::GetToolTip().IsValid())
	{
		TSharedRef<SToolTip> DefaultToolTip = IDocumentation::Get()->CreateToolTip(TAttribute<FText>(this, &Super::GetNodeTooltip), nullptr,
			GraphNode->GetDocumentationLink(), GraphNode->GetDocumentationExcerptName());
		SetToolTip(DefaultToolTip);
	}

	// Setup content
	{
		ContentScale.Bind(this, &Super::GetContentScale);
		GetOrAddSlot(ENodeZone::Center)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FNYAppStyle::GetBrush("Graph.StateNode.Body"))
				.Padding(0)
				.BorderBackgroundColor(Settings->BorderBackgroundColor)
				[
					// PIN AREA, output pin
					SNew(SOverlay)
					+SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						PinsNodeBox.ToSharedRef()
					]

					// Content/Body area
					+SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(NodePadding)
					[
						GetNodeBodyWidget()
					]
				]
			];
		CreatePinWidgets();
	}

	// Create comment bubble
	{
		TSharedPtr<SCommentBubble> CommentBubble;
		const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

		SAssignNew(CommentBubble, SCommentBubble)
			.GraphNode(GraphNode)
			.Text(this, &Super::GetNodeComment)
			.OnTextCommitted(this, &Super::OnCommentTextCommitted)
			.OnToggled(this, &Super::OnCommentBubbleToggled)
			.ColorAndOpacity(CommentColor)
			.AllowPinning(true)
			.EnableTitleBarBubble(true)
			.EnableBubbleCtrls(true)
			.GraphLOD(this, &Super::GetCurrentLOD)
			.IsGraphNodeHovered(this, &Super::IsHovered);

		// Add it at the top, right above
		GetOrAddSlot(ENodeZone::TopCenter)
			.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
			.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
			.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
			.VAlign(VAlign_Top)
			[
				CommentBubble.ToSharedRef()
			];
	}
}
// End SGraphNode interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin own functions
TSharedRef<SWidget> SDlgGraphNode::GetNodeBodyWidget()
{
	if (NodeBodyWidget.IsValid())
	{
		return NodeBodyWidget.ToSharedRef();
	}

	// Setup a meta tag for this node
//	FGraphNodeMetaData TagMeta(TEXT("UDialogueGraphNode_Base"));
//	PopulateMetaTag(&TagMeta);

	NodeBodyWidget =
		SNew(SBorder)
		.BorderImage(FNYAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
		.BorderBackgroundColor(this, &Self::GetBackgroundColor)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(1.0f)
		.Visibility(EVisibility::Visible)
		[
			// Main Content
			SNew(SVerticalBox)

			// Title
			+SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				GetTitleWidget()
			]

			// Description
			+SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoHeight()
			[
				GetDescriptionWidget()
			]
		];

	return NodeBodyWidget.ToSharedRef();
}

TSharedRef<SWidget> SDlgGraphNode::GetTitleWidget()
{
	if (TitleWidget.IsValid())
	{
		return TitleWidget.ToSharedRef();
	}

	// Title
	TSharedPtr<SNodeTitle> NodeTitleMultipleLines = SNew(SNodeTitle, GraphNode);
	TWeakPtr<SNodeTitle> WeakNodeTitle = NodeTitleMultipleLines;
	auto GetNodeTitlePlaceholderWidth = [WeakNodeTitle]() -> FOptionalSize
	{
		TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
		const float DesiredWidth = NodeTitlePin.IsValid() ? NodeTitlePin->GetTitleSize().X : 0.0f;
		return FMath::Max(75.0f, DesiredWidth);
	};
	auto GetNodeTitlePlaceholderHeight = [WeakNodeTitle]() -> FOptionalSize
	{
		TSharedPtr<SNodeTitle> NodeTitlePin = WeakNodeTitle.Pin();
		const float DesiredHeight = NodeTitlePin.IsValid() ? NodeTitlePin->GetTitleSize().Y : 0.0f;
		return FMath::Max(22.0f, DesiredHeight);
	};

	// Icon and tint is set from the node
	IconColor = FLinearColor::White; // default
	const FSlateBrush* IconBrush = nullptr;
	if (GraphNode->ShowPaletteIconOnNode())
	{
		IconBrush = GraphNode->GetIconAndTint(IconColor).GetIcon();
	}

	TitleWidget = SNew(SHorizontalBox)
		// Error message and title
		+SHorizontalBox::Slot()
		.HAlign(Settings->TitleHorizontalAlignment)
		.VAlign(VAlign_Top)
		.AutoWidth()
		[
			SNew(SHorizontalBox)

			// Error message
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				// POPUP ERROR MESSAGE
				ErrorReporting->AsWidget()
			]

			// Title
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SLevelOfDetailBranchNode)
				.UseLowDetailSlot(this, &Self::UseLowDetailNodeTitles)
				.LowDetail()
				[
					SNew(SBox)
					.WidthOverride_Lambda(GetNodeTitlePlaceholderWidth)
					.HeightOverride_Lambda(GetNodeTitlePlaceholderHeight)
				]
				.HighDetail()
				[
					SNew(SHorizontalBox)

					// Icon
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SImage)
						.Image(IconBrush)
						.ColorAndOpacity(this, &Super::GetNodeTitleIconColor)
					]

					// Title content
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SVerticalBox)

						// Display the first line
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							// See FSlateEditorStyle for styles
							.TextStyle(FNYAppStyle::Get(), "Graph.StateNode.NodeTitle")
							// gets always the first line of the title
							.Text(NodeTitleMultipleLines.Get(), &SNodeTitle::GetHeadTitle)
						]

						// Display the rest of the lines. The usage of SNodeTitle is a lie, the Style argument does not do anything.
						// This only starts dispalying from line 1 (if there is a multi line title)
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							NodeTitleMultipleLines.ToSharedRef()
						]
					]
				]
			]
		];

	return TitleWidget.ToSharedRef();
}

TSharedRef<SWidget> SDlgGraphNode::GetDescriptionWidget()
{
	if (DescriptionWidget.IsValid())
	{
		return DescriptionWidget.ToSharedRef();
	}

	if (DialogueGraphNode->IsSpeechSequenceNode())
	{
		TSharedPtr<SVerticalBox> VerticalBox = SNew(SVerticalBox);
		const int32 SpechSequnceEntriesNum = GetSpeechSequenceEntries().Num();

		for (int32 EntryIndex = 0; EntryIndex < SpechSequnceEntriesNum; EntryIndex++)
		{
			VerticalBox->AddSlot()
				.AutoHeight()
				[
					// Black border around each entry
					SNew(SBorder)
					.BorderImage(FNYAppStyle::GetBrush("PlainBorder"))
					.BorderBackgroundColor(Settings->BorderBackgroundColor)
					.Padding(FMargin(0.f, 2.f))
					[
						SNew(SVerticalBox)

						// Speaker/Owner name of entry
						+SVerticalBox::Slot()
						.HAlign(Settings->DescriptionSpeakerHorizontalAlignment)
						.VAlign(VAlign_Center)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Visibility(this, &Self::GetDescriptionVisibility)
							.Text(this, &Self::GetSpeakerForSpeechSequenceEntryAt, EntryIndex)
							.TextStyle(FNYAppStyle::Get(), "Graph.Node.NodeTitle")
							.Margin(Settings->DescriptionSpeakerMargin)
						]

						// Text for entry
						+SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Visibility(this, &Self::GetDescriptionVisibility)
							.Text(this, &Self::GetDescriptionForSpeechSequenceEntryAt, EntryIndex)
							.WrapTextAt(Settings->DescriptionWrapTextAt)
							.Margin(Settings->DescriptionTextMargin)
						]
					]
				];
		}
		DescriptionWidget = VerticalBox;
	}
	else
	{
		DescriptionWidget = SNew(STextBlock)
			.Visibility(this, &Self::GetDescriptionVisibility)
			.Text(this, &Self::GetDescription)
			.WrapTextAt(Settings->DescriptionWrapTextAt)
			.Margin(Settings->DescriptionTextMargin);
	}

	return DescriptionWidget.ToSharedRef();
}

FText SDlgGraphNode::GetDescription() const
{
	if (DialogueGraphNode && DialogueGraphNode->IsDialogueNodeSet())
	{
		const UDlgNode& DlgNode = DialogueGraphNode->GetDialogueNode();

		if (const UDlgNode_Proxy* AsProxy = Cast<UDlgNode_Proxy>(&DlgNode))
		{
			// if proxy node let's try to return with the text of the node the proxy leads to

			const int32 TargetNodeIndex = AsProxy->GetTargetNodeIndex();
			const UDlgDialogue* Dialogue = DialogueGraphNode->GetDialogue();
			const TArray<UDlgNode*>& Nodes = Dialogue->GetNodes();
			if (Nodes.IsValidIndex(TargetNodeIndex))
			{
				return Nodes[TargetNodeIndex]->GetNodeUnformattedText();
			}
		}
		else
		{
			return DialogueGraphNode->GetDialogueNode().GetNodeUnformattedText();
		}
	}

	return FText::GetEmpty();
}

EVisibility SDlgGraphNode::GetOverlayWidgetVisibility() const
{
	// always hide the index on the root node
	if (DialogueGraphNode->IsRootNode())
	{
		return EVisibility::Hidden;
	}

	// LOD this out once things get too small
	TSharedPtr<SGraphPanel> MyOwnerPanel = GetOwnerPanel();
	return !MyOwnerPanel.IsValid() || MyOwnerPanel->GetCurrentLOD() > EGraphRenderingLOD::LowDetail ? EVisibility::Visible : EVisibility::Collapsed;
}


FText SDlgGraphNode::GetIndexOverlayTooltipText() const
{
	return LOCTEXT("NodeIndexTooltip", "Node index: this shows the node index in the Dialogue.Nodes Array");
}

FText SDlgGraphNode::GetConditionOverlayTooltipText() const
{
	return LOCTEXT("NodeConditionTooltip", "Node has enter conditions.\nOnly if these conditions are satisfied the node is entered.");
}

FText SDlgGraphNode::GetEventOverlayTooltipText() const
{
	return LOCTEXT("NodeEventTooltip", "Node has enter events.\nOn node enter this events are executed.");
}

FText SDlgGraphNode::GetVoiceOverlayTooltipText() const
{
	return LOCTEXT("NodeVoiceTooltip", "Node has some voice variables set. Either the SoundWave or the DialogueWave.");
}

FText SDlgGraphNode::GetGenericOverlayTooltipText() const
{
	return LOCTEXT("NodeGenericTooltip", "Node has the generic data variable set.");
}

void SDlgGraphNode::OnIndexHoverStateChanged(bool bHovered)
{

}
// End own functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
