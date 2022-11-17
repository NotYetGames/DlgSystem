// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SGraphNode_DialogueK2Switch.h"

#include "ScopedTransaction.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "GraphEditorSettings.h"
#include "NodeFactory.h"
#include "KismetPins/SGraphPinExec.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_Switch.h"

#include "DialogueK2Node_SwitchDialogueCallback.h"
#include "DlgSystem/NYEngineVersionHelpers.h"


//////////////////////////////////////////////////////////////////////////
// SGraphPinSwitchNodeDefaultCaseExec
class SGraphPinSwitchNodeDefaultCaseExec : public SGraphPinExec
{
public:
	SLATE_BEGIN_ARGS(SGraphPinSwitchNodeDefaultCaseExec) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin)
	{
		SGraphPin::Construct(SGraphPin::FArguments().PinLabelStyle(FName("Graph.Node.DefaultPinName")), InPin);
		CachePinIcons();
	}
};

//////////////////////////////////////////////////////////////////////////
// SGraphNode_DialogueK2Switch
void SGraphNode_DialogueK2Switch::Construct(const FArguments& InArgs, UDialogueK2Node_SwitchDialogueCallback* InNode)
{
	GraphNode = CastChecked<UEdGraphNode>(InNode);
	DialogueK2Node_Switch = InNode;

	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

void SGraphNode_DialogueK2Switch::CreatePinWidgets()
{
	UK2Node_Switch* SwitchNode = CastChecked<UK2Node_Switch>(GraphNode);
	UEdGraphPin* DefaultPin = SwitchNode->GetDefaultPin();

	// Create Pin widgets for each of the pins, except for the default pin
	for (UEdGraphPin* CurrentPin : GraphNode->Pins)
	{
		if (!CurrentPin->bHidden && CurrentPin != DefaultPin)
		{
			TSharedPtr<SGraphPin> NewPin = FNodeFactory::CreatePinWidget(CurrentPin);
			check(NewPin.IsValid());
			AddPin(NewPin.ToSharedRef());
		}
	}

	// Handle the default pin
	if (DefaultPin != nullptr)
	{
		// Create some padding
		RightNodeBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(1.0f)
			[
				SNew(SImage)
				.Image(FNYAppStyle::GetBrush("Graph.Pin.DefaultPinSeparator"))
			];

		// Create the pin itself
		TSharedPtr<SGraphPin> NewPin = SNew(SGraphPinSwitchNodeDefaultCaseExec, DefaultPin);
		AddPin(NewPin.ToSharedRef());
	}
}

void SGraphNode_DialogueK2Switch::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
		NSLOCTEXT("SwitchStatementNode", "RefreshNodeButton", "Refresh Node"),
		NSLOCTEXT("SwitchStatementNode", "RefreshNodeButton_Tooltip", "Refreshes the node with the dialogue values"));

	FMargin AddPinPadding = Settings->GetOutputPinPadding();
	AddPinPadding.Top += 6.0f;

	OutputBox->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(AddPinPadding)
		[
			AddPinButton
		];
}

EVisibility SGraphNode_DialogueK2Switch::IsAddPinButtonVisible() const
{
	return EVisibility::Visible;
}

FReply SGraphNode_DialogueK2Switch::OnAddPin()
{
	const FScopedTransaction Transaction(NSLOCTEXT("Kismet", "RefreshNode", "Refresh Node"));
	DialogueK2Node_Switch->Modify();
	FBlueprintEditorUtils::MarkBlueprintAsModified(DialogueK2Node_Switch->GetBlueprint());

	// Reconstruct and notify
	DialogueK2Node_Switch->ReconstructNode();
	UpdateGraphNode();
	GraphNode->GetGraph()->NotifyGraphChanged();

	return FReply::Handled();
}
