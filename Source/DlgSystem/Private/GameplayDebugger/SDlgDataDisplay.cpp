// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "SDlgDataDisplay.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SMissingWidget.h"
#include "Widgets/Input/SButton.h"

#include "DlgManager.h"

#define LOCTEXT_NAMESPACE "SDlgDataDisplay"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataDisplay
void SDlgDataDisplay::Construct(const FArguments& InArgs, TWeakObjectPtr<AActor> InReferenceActor)
{
	ReferenceActor = InReferenceActor;
	RootTreeItem = MakeShareable(new FDlgDataDisplayTreeRootNode);
	ActorsTreeView = SNew(STreeView<FDlgDataDisplayTreeNodePtr>)
		.ItemHeight(32)
		.TreeItemsSource(&RootChildren)
		.OnGenerateRow(this, &Self::HandleGenerateRow)
		.OnSelectionChanged(this, &Self::HandleTreeSelectionChanged)
		.OnGetChildren(this, &Self::HandleGetChildren)
		.SelectionMode(ESelectionMode::Single)
		.OnMouseButtonDoubleClick(this, &Self::HandleDoubleClick)
		.OnSetExpansionRecursive(this, &Self::HandleSetExpansionRecursive)
		.HeaderRow(
			SNew(SHeaderRow)
			.Visibility(EVisibility::Visible)
			+SHeaderRow::Column(NAME_Name)
			.DefaultLabel(LOCTEXT("Name", "Name"))
		);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.BorderBackgroundColor(FLinearColor::Gray) // Darken the outer border
		[
			SNew(SVerticalBox)

			// Top bar
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.f, 1.f)
			[
				SNew(SHorizontalBox)

				// Search Input
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.f, 0.f, 4.f, 0.f)
				[
					GetFilterTextBoxWidget()
				]

				// Refresh Actors
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ToolTipText(LOCTEXT("RefreshToolTip", "Refreshes/Reloads the Actors Dialogue data."))
					.OnClicked(this, &Self::HandleOnRefresh)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("RefreshDialogues", "Refresh"))
					]
				]
			]

			// The Tree view
			+SVerticalBox::Slot()
			.AutoHeight()
			.FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(0.0f, 4.0f))
				[
					ActorsTreeView.ToSharedRef()
				]
			]
		]
	];

	RefreshTree(false);
}

void SDlgDataDisplay::RefreshTree(bool bPreserveExpansion)
{
	RootTreeItem->ClearChildren();
	RootChildren.Empty();
	ActorsProperties.Empty();

	// No Actor :( can't get the World
	if (!ReferenceActor.IsValid() || ReferenceActor->GetWorld() == nullptr)
	{
		return;
	}

	TArray<UDlgDialogue*> Dialogues = UDlgManager::GetAllDialoguesFromMemory();
	TArray<TWeakObjectPtr<AActor>> Actors = UDlgManager::GetAllActorsImplementingDialogueParticipantInterface(ReferenceActor->GetWorld());

	// Build fast lookup for ParticipantNames
	// Maps from ParticipantName => Array of Dialogues that have this Participant.
	TMap<FName, TSet<TWeakObjectPtr<UDlgDialogue>>> ParticipantNamesDialoguesMap;
	for (UDlgDialogue* Dialogue : Dialogues)
	{
		TSet<FName> ParticipantsNames;
		Dialogue->GetAllParticipantNames(ParticipantsNames);

		for (const FName& ParticipantName : ParticipantsNames)
		{
			TSet<TWeakObjectPtr<UDlgDialogue>>* ValuePtr = ParticipantNamesDialoguesMap.Find(ParticipantName);
			if (ValuePtr == nullptr)
			{
				// does not exist
				TSet<TWeakObjectPtr<UDlgDialogue>> ValueArray{Dialogue};
				ParticipantNamesDialoguesMap.Add(ParticipantName, ValueArray);
			}
			else
			{
				// exists, add the Dialogue
				ValuePtr->Add(Dialogue);
			}
		}
	}

	// Build the fast lookup structure for Actors (the ActorsProperties)
	for (TWeakObjectPtr<AActor> Actor : Actors)
	{
		if (!Actor.IsValid())
		{
			return;
		}

		// Should never happen, the actor should always be unique in the Actors array.
		ensure(ActorsProperties.Find(Actor) == nullptr);

		// Find out the Dialogues that have the ParticipantName of this Actor.
		const FName ParticipantName = IDlgDialogueParticipant::Execute_GetParticipantName(Actor.Get());
		TSet<TWeakObjectPtr<UDlgDialogue>> ActorDialogues;
	    TSet<TWeakObjectPtr<UDlgDialogue>>* ActorDialoguesPtr = ParticipantNamesDialoguesMap.Find(ParticipantName);
		if (ActorDialoguesPtr != nullptr)
		{
			// Found some dialogues
			ActorDialogues = *ActorDialoguesPtr;
		}

		// Create Key in the ActorsProperties for this Actor.
		TSharedPtr<FDlgDataDisplayActorProperties> ActorsPropertiesValue =
			MakeShareable(new FDlgDataDisplayActorProperties(ActorDialogues));
		ActorsProperties.Add(Actor, ActorsPropertiesValue);

		// Gather Data from the Dialogues
		for (TWeakObjectPtr<UDlgDialogue> Dialogue : ActorDialogues)
		{
			if (!Dialogue.IsValid())
			{
				return;
			}

			// Populate Event Names
			TSet<FName> EventsNames;
			Dialogue->GetEvents(ParticipantName, EventsNames);
			for (const FName& EventName : EventsNames)
			{
				ActorsPropertiesValue->AddDialogueToEvent(EventName, Dialogue);
			}

			// Populate conditions
			TSet<FName> ConditionNames;
			Dialogue->GetConditions(ParticipantName, ConditionNames);
			for (const FName& ConditionName : ConditionNames)
			{
				ActorsPropertiesValue->AddDialogueToCondition(ConditionName, Dialogue);
			}

			// Populate int variable names
			TSet<FName> IntVariableNames;
			Dialogue->GetIntNames(ParticipantName, IntVariableNames);
			for (const FName& IntVariableName : IntVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToIntVariable(IntVariableName, Dialogue);
			}

			// Populate float variable names
			TSet<FName> FloatVariableNames;
			Dialogue->GetFloatNames(ParticipantName, FloatVariableNames);
			for (const FName& FloatVariableName : FloatVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToFloatVariable(FloatVariableName, Dialogue);
			}

			// Populate bool variable names
			TSet<FName> BoolVariableNames;
			Dialogue->GetBoolNames(ParticipantName, BoolVariableNames);
			for (const FName& BoolVariableName : BoolVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToBoolVariable(BoolVariableName, Dialogue);
			}

			// Populate FName variable names
			TSet<FName> FNameVariableNames;
			Dialogue->GetNameNames(ParticipantName, FNameVariableNames);
			for (const FName& NameVariableName : FNameVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToFNameVariable(NameVariableName, Dialogue);
			}
		}
	}

	// Build the Actors Tree View (aka the actual tree)
	for (const auto& Elem : ActorsProperties)
	{
		// Key: AActor
		if (!Elem.Key.IsValid())
		{
			continue;
		}

		const AActor* Actor = Elem.Key.Get();
		FDlgDataDisplayTreeNodePtr ActorItem =
			MakeShareable(new FDlgDataDisplayTreeActorNode(FText::FromString(Actor->GetName()), RootTreeItem, Actor));
		BuildTreeViewItem(ActorItem);
		RootTreeItem->AddChild(ActorItem);
	}
	RootChildren = RootTreeItem->GetChildren();

	// Clear Previous states
	ActorsTreeView->ClearSelection();
	// Triggers RequestTreeRefresh
	ActorsTreeView->ClearExpandedItems();
}

TSharedRef<SWidget> SDlgDataDisplay::GetFilterTextBoxWidget()
{
	// Is it cached?
	if (FilterTextBoxWidget.IsValid())
	{
		return FilterTextBoxWidget.ToSharedRef();
	}

	// Cache it
	FilterTextBoxWidget = SNew(SSearchBox)
		.HintText(LOCTEXT("SearchBoxHintText", "TODO Search by Name TODO"))
		.OnTextChanged(this, &Self::HandleSearchTextCommited, ETextCommit::Default)
		.OnTextCommitted(this, &Self::HandleSearchTextCommited)
		.SelectAllTextWhenFocused(false)
		.DelayChangeNotificationsWhileTyping(false);

	// Should return a valid widget
	return GetFilterTextBoxWidget();
}

void SDlgDataDisplay::BuildTreeViewItem(FDlgDataDisplayTreeNodePtr Item)
{
	static int32 NumberCalls = 0;
	NumberCalls++;
	// check(NumberCalls < 30);

	TWeakObjectPtr<const AActor> Actor = Item->GetParentActor();
	if (!Actor.IsValid())
	{
		return;
	}

	// Do we have the actor cached?
	TSharedPtr<FDlgDataDisplayActorProperties>* ValuePtr = ActorsProperties.Find(Actor);
	if (ValuePtr == nullptr)
	{
		return;
	}
	TSharedPtr<FDlgDataDisplayActorProperties> ActorPropertiesValue = *ValuePtr;


	if (Item->IsText())
	{
		switch (Item->GetTextType())
		{
			case EDlgDataDisplayTextTreeNodeType::Actor:
				Item->AddChild(MakeShareable(new FDlgDataDisplayTreeCategoryNode(
					LOCTEXT("EventKey", "Events"), Item, EDlgDataDisplayCategoryTreeNodeType::Event)));
				Item->AddChild(MakeShareable(new FDlgDataDisplayTreeCategoryNode(
					LOCTEXT("ConditionKey", "Conditions"), Item, EDlgDataDisplayCategoryTreeNodeType::Condition)));
				Item->AddChild(MakeShareable(new FDlgDataDisplayTreeCategoryNode(
					LOCTEXT("VariablesKey", "Variables"), Item, EDlgDataDisplayCategoryTreeNodeType::Variables)));
				break;

			case EDlgDataDisplayTextTreeNodeType::Variable:
				// No children for variable.
				break;

			default:
				unimplemented()
		}
	}
	else if (Item->IsCategory())
	{
		// Add variables for each appropriate category
		switch (Item->GetCategoryType())
		{
			case EDlgDataDisplayCategoryTreeNodeType::Event:
				for (const auto& Pair: ActorPropertiesValue->GetEvents())
				{
					const FDlgDataDisplayTreeNodePtr EventItem = MakeShareable(new FDlgDataDisplayTreeVariableNode(
						FText::FromName(Pair.Key), Item, Pair.Key, EDlgDataDisplayVariableTreeNodeType::Event));
					Item->AddChild(EventItem);
				}
				break;

			case EDlgDataDisplayCategoryTreeNodeType::Condition:
				for (const auto& Pair: ActorPropertiesValue->GetConditions())
				{
					const FDlgDataDisplayTreeNodePtr ConditionItem = MakeShareable(new FDlgDataDisplayTreeVariableNode(
						FText::FromName(Pair.Key), Item, Pair.Key, EDlgDataDisplayVariableTreeNodeType::Condition));
					Item->AddChild(ConditionItem);
				}
				break;

			case EDlgDataDisplayCategoryTreeNodeType::Variables:
			{
				for (const auto& Pair: ActorPropertiesValue->GetIntegers())
				{
					const FName VariableName = Pair.Key;
					const int32 Value = IDlgDialogueParticipant::Execute_GetIntValue(Actor.Get(), VariableName);

					FFormatOrderedArguments Args;
					Args.Add(FText::FromName(VariableName));
					Args.Add(Value);
					const FText DisplayText = FText::Format(LOCTEXT("VariableTextKey", "int {0} = {1}"), Args);

					// Create Node
					const TSharedPtr<FDlgDataDisplayTreeVariableNode> IntItem = MakeShareable(new FDlgDataDisplayTreeVariableNode(
						DisplayText, Item, VariableName, EDlgDataDisplayVariableTreeNodeType::Integer));
					IntItem->SetVariableValue(FString::FromInt(Value));
					Item->AddChild(IntItem);
				}
				for (const auto& Pair: ActorPropertiesValue->GetFloats())
				{
					const FName VariableName = Pair.Key;
					const float Value = IDlgDialogueParticipant::Execute_GetFloatValue(Actor.Get(), VariableName);

					FFormatOrderedArguments Args;
					Args.Add(FText::FromName(VariableName));
					Args.Add(Value);
					const FText DisplayText = FText::Format(LOCTEXT("VariableTextKey", "float {0} = {1}"), Args);

					// Create Node
					const TSharedPtr<FDlgDataDisplayTreeVariableNode> FloatItem = MakeShareable(new FDlgDataDisplayTreeVariableNode(
						DisplayText, Item, Pair.Key, EDlgDataDisplayVariableTreeNodeType::Float));
					FloatItem->SetVariableValue(FString::SanitizeFloat(Value));
					Item->AddChild(FloatItem);
				}
				for (const auto& Pair: ActorPropertiesValue->GetBools())
				{
					const FName VariableName = Pair.Key;
					const bool Value = IDlgDialogueParticipant::Execute_GetBoolValue(Actor.Get(), VariableName);
					const FString ValueString = Value ? TEXT("true") : TEXT("false");

					FFormatOrderedArguments Args;
					Args.Add(FText::FromName(VariableName));
					Args.Add(FText::FromString(ValueString));
					const FText DisplayText = FText::Format(LOCTEXT("VariableTextKey", "bool {0} = {1}"), Args);

					// Create Node
					const TSharedPtr<FDlgDataDisplayTreeVariableNode> BoolItem = MakeShareable(new FDlgDataDisplayTreeVariableNode(
						DisplayText, Item, Pair.Key, EDlgDataDisplayVariableTreeNodeType::Bool));
					BoolItem->SetVariableValue(ValueString);
					Item->AddChild(BoolItem);
				}
				for (const auto& Pair: ActorPropertiesValue->GetFNames())
				{
					const FName VariableName = Pair.Key;
					const FName Value = IDlgDialogueParticipant::Execute_GetNameValue(Actor.Get(), VariableName);

					FFormatOrderedArguments Args;
					Args.Add(FText::FromName(VariableName));
					Args.Add(FText::FromName(Value));
					const FText DisplayText = FText::Format(LOCTEXT("VariableTextKey", "FName {0} = {1}"), Args);

					// Create Node
					const TSharedPtr<FDlgDataDisplayTreeVariableNode> FNameItem = MakeShareable(new FDlgDataDisplayTreeVariableNode(
						DisplayText, Item, Pair.Key, EDlgDataDisplayVariableTreeNodeType::FName));
					FNameItem->SetVariableValue(Value.ToString());
					Item->AddChild(FNameItem);
				}
				break;
			}
			default:
				unimplemented();
		}
	}

	// Recursively call on children
	for (const FDlgDataDisplayTreeNodePtr& ChildItem : Item->GetChildren())
	{
		BuildTreeViewItem(ChildItem);
	}
}

void SDlgDataDisplay::HandleSearchTextCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	// Trim and sanitized the filter text (so that it more likely matches)
	FilterString = FText::TrimPrecedingAndTrailing(InText).ToString();
}

TSharedRef<ITableRow> SDlgDataDisplay::HandleGenerateRow(FDlgDataDisplayTreeNodePtr InItem,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// Build row
	TSharedPtr<STableRow<FDlgDataDisplayTreeNodePtr>> TableRow;
	FMargin RowPadding = FMargin(2.f, 2.f);
	TableRow = SNew(STableRow<FDlgDataDisplayTreeNodePtr>, OwnerTable)
		.Padding(1.0f);

	// Default row content
	TSharedPtr<STextBlock> DefaultTextBlock = SNew(STextBlock)
			.Text(InItem->GetDisplayText())
			.HighlightText(this, &Self::GetFilterText);

	TSharedPtr<SWidget> RowContent = DefaultTextBlock;
	TSharedPtr<SHorizontalBox> RowContainer;
	TableRow->SetRowContent(SAssignNew(RowContainer, SHorizontalBox));

	RowContainer->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Right)
		[
			SNew(SExpanderArrow, TableRow)
		];

	RowContainer->AddSlot()
		.FillWidth(1.0)
		.Padding(RowPadding)
		[
			RowContent.ToSharedRef()
		];

	return TableRow.ToSharedRef();
}

void SDlgDataDisplay::HandleGetChildren(FDlgDataDisplayTreeNodePtr InItem,
	TArray<FDlgDataDisplayTreeNodePtr>& OutChildren)
{
	if (!InItem.IsValid())
	{
		return;
	}

	OutChildren = InItem->GetChildren();
}

void SDlgDataDisplay::HandleTreeSelectionChanged(FDlgDataDisplayTreeNodePtr InItem, ESelectInfo::Type SelectInfo)
{
	// TODO
}

void SDlgDataDisplay::HandleDoubleClick(FDlgDataDisplayTreeNodePtr InItem)
{
	if (!InItem.IsValid())
	{
		return;
	}
	// Ignore
}

void SDlgDataDisplay::HandleSetExpansionRecursive(FDlgDataDisplayTreeNodePtr InItem, bool bInIsItemExpanded)
{
	// TODO
}

#undef LOCTEXT_NAMESPACE
