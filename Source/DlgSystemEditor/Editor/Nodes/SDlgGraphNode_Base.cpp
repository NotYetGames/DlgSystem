// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgGraphNode_Base.h"

#include "Widgets/Layout/SBox.h"
#include "DialogueGraphNode_Root.h"
#include "IDocumentation.h"
#include "GraphEditorDragDropAction.h"

#include "SDlgGraphPin.h"

#define LOCTEXT_NAMESPACE "DialogueEditor"


/////////////////////////////////////////////////////
// SDlgGraphNode_Base
void SDlgGraphNode_Base::Construct(const FArguments& InArgs, UDialogueGraphNode_Base* InNode)
{
	GraphNode = Cast<UEdGraphNode>(InNode);
	DialogueGraphNode_Base = InNode;
	Settings = GetDefault<UDlgSystemSettings>();
}

void SDlgGraphNode_Base::CreatePinWidgets()
{
	// Create only one pin, the output one, the rest is handled by FDlgGraphConnectionDrawingPolicy
	UEdGraphPin* OutputPin = DialogueGraphNode_Base->GetOutputPin();
	UEdGraphPin* InputPin = DialogueGraphNode_Base->GetInputPin();

	// Order is important, or something like that.
	if (!IsValidPin(InputPin))
	{
		return;
	}

	//CreateStandardPinWidget(InputPin);

	if (!IsValidPin(OutputPin))
	{
		return;
	}

	CreateStandardPinWidget(OutputPin);
}

void SDlgGraphNode_Base::CreateStandardPinWidget(UEdGraphPin* Pin)
{
	Super::CreateStandardPinWidget(Pin);
}

void SDlgGraphNode_Base::UpdateGraphNode()
{
	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	InputPins.Empty();
	OutputPins.Empty();
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	PinsNodeBox.Reset();
	SAssignNew(PinsNodeBox, SVerticalBox);

	// This Node visibility
	SetVisibility(TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateSP(this, &Self::GetNodeVisibility)));
}

void SDlgGraphNode_Base::SetOwner(const TSharedRef<SGraphPanel>& OwnerPanel)
{
	check(!OwnerGraphPanelPtr.IsValid());
	SetParentPanel(OwnerPanel);
	OwnerGraphPanelPtr = OwnerPanel;
	GraphNode->DEPRECATED_NodeWidget = SharedThis(this);

	// Once we have an owner, and if hide Unused pins is enabled, we need to remake our pins to drop the hidden ones
	if (OwnerGraphPanelPtr.Pin()->GetPinVisibility() != SGraphEditor::Pin_Show && PinsNodeBox.IsValid())
	{
		PinsNodeBox->ClearChildren();
		CreatePinWidgets();
	}
}

EVisibility SDlgGraphNode_Base::GetEventAndConditionVisibility() const
{
	return Settings->bShowEventsAndConditions ? EVisibility::Visible : EVisibility::Collapsed;
}

TSharedPtr<SGraphPin> SDlgGraphNode_Base::CreatePinWidget(UEdGraphPin* Pin) const
{
	// Called by CreateStandardPinWidget
	return SNew(SDlgGraphPin, Pin);
}

void SDlgGraphNode_Base::CreateEventAndConditionWidgets(TSharedPtr<SVerticalBox> TargetWidget)
{
	for (int32 i = 0; i < GetEnterConditions()->Num(); ++i)
	{
		TargetWidget->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(2.0f)
		[
			SNew(SBorder)
			.BorderImage(FNYAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
			.BorderBackgroundColor(Settings->GraphConditionBorderColor)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Visibility(this, &Self::GetEventAndConditionVisibility)
			.Padding(Settings->GraphConditionBorderSize)
			[
				SNew(SBorder)
				.BorderImage(FNYAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
				.BorderBackgroundColor(Settings->GraphConditionBackgroundColor)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(Settings->GraphConditionTextMargin)
				[
					SNew(STextBlock)
					.ColorAndOpacity(Settings->GraphConditionTextColor)
					.Text_Lambda([this, i]()
					{
						return FText::FromString((*GetEnterConditions())[i].GetEditorDisplayString(DialogueGraphNode_Base->GetDialogue()));
					})
				]
			]
		];
	}

	const TArray<FDlgEvent>* EnterEventsPtr = GetEnterEvents();
	if (EnterEventsPtr == nullptr)
	{
		return;
	}

	for (int32 i = 0; i < EnterEventsPtr->Num(); ++i)
	{
		TargetWidget->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(2.0f)
		[
			SNew(SBorder)
			.Visibility(this, &Self::GetEventAndConditionVisibility)
			.BorderImage(FNYAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
			.BorderBackgroundColor(Settings->GraphEventBorderColor)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(Settings->GraphEventBorderSize)
			[
				SNew(SBorder)
				.BorderImage(FNYAppStyle::GetBrush("BTEditor.Graph.BTNode.Body"))
				.BorderBackgroundColor(Settings->GraphEventBackgroundColor)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(Settings->GraphEventTextMargin)
				[
					SNew(STextBlock)
					.ColorAndOpacity(Settings->GraphEventTextColor)
					.Text_Lambda([this, i]()
					{
						return FText::FromString((*GetEnterEvents())[i].GetEditorDisplayString(DialogueGraphNode_Base->GetDialogue()));
					})
				]
			]
		];
	}
}

void SDlgGraphNode_Base::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	PinsNodeBox->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.FillHeight(1.0f)
		[
			PinToAdd
		];

	if (PinToAdd->GetDirection() == EGPD_Output)
	{
		OutputPins.Add(PinToAdd);
	}
	else
	{
		InputPins.Add(PinToAdd);
	}
}
// End SGraphNode interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
