// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SGraphNode_DialogueK2Select.h"

#include "ScopedTransaction.h"
#include "Widgets/SBoxPanel.h"
#include "GraphEditorSettings.h"
#include "K2Node.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Images/SImage.h"

#include "DialogueK2Node_Select.h"

void SGraphNode_DialogueK2Select::Construct(const FArguments& InArgs, UDialogueK2Node_Select* InNode)
{
	GraphNode = CastChecked<UK2Node>(InNode);
	DialogueK2Node_Select = InNode;

	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SGraphNode_DialogueK2Select::CreatePinWidgets()
{
	// does not matter order for output pin
	CreateStandardPinWidget(DialogueK2Node_Select->GetReturnValuePin());

	// Add all option pins first
	const int32 PinsNum = DialogueK2Node_Select->Pins.Num();
	for (int32 PinIndex = UDialogueK2Node_Select::INDEX_PIN_OPTIONS_START; PinIndex < PinsNum; PinIndex++)
	{
		CreateStandardPinWidget(DialogueK2Node_Select->Pins[PinIndex]);
	}

	// Create the Default and VariableName input pin
	// Create some padding
	LeftNodeBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(1.0f)
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("Graph.Pin.DefaultPinSeparator"))
		];

	CreateStandardPinWidget(DialogueK2Node_Select->GetDefaultValuePin());

	// Create some padding
	LeftNodeBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(1.0f)
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("Graph.Pin.DefaultPinSeparator"))
		];

	CreateStandardPinWidget(DialogueK2Node_Select->GetVariableNamePin());
}

void SGraphNode_DialogueK2Select::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	TSharedRef<SWidget> RefreshButton = AddPinButtonContent(
		NSLOCTEXT("SGraphNode_DialogueK2Select", "RefreshNodeButton", "Refresh Node"),
		NSLOCTEXT("SGraphNode_DialogueK2Select", "RefreshNodeButton_Tooltip", "Refreshes the node with the dialogue values"));

	FMargin AddPinPadding = Settings->GetOutputPinPadding();
	AddPinPadding.Top += 6.0f;

	OutputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			RefreshButton
		];
}

EVisibility SGraphNode_DialogueK2Select::IsAddPinButtonVisible() const
{
	return EVisibility::Visible;
}

FReply SGraphNode_DialogueK2Select::OnAddPin()
{
	const FScopedTransaction Transaction(NSLOCTEXT("Kismet", "RefreshNode", "Refresh Node"));
	DialogueK2Node_Select->Modify();
	FBlueprintEditorUtils::MarkBlueprintAsModified(DialogueK2Node_Select->GetBlueprint());

	// Reconstruct and notify
	DialogueK2Node_Select->ReconstructNode();
	UpdateGraphNode();
	DialogueK2Node_Select->GetGraph()->NotifyGraphChanged();

	return FReply::Handled();
}

