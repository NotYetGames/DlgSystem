// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgDataDisplay.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SMissingWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Engine/World.h"

// #if WITH_EDITOR
// #include "Editor.h"
// #endif

#include "DlgSystem/DlgManager.h"
#include "DlgSystem/DlgContext.h"
#include "SDlgDataPropertyValues.h"
#include "DlgSystem/Logging/DlgLogger.h"

//////////////////////////////////////////////////////////////////////////
DEFINE_LOG_CATEGORY(LogDlgSystemDataDisplay)
//////////////////////////////////////////////////////////////////////////

#define LOCTEXT_NAMESPACE "SDlgDataDisplay"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDlgDataDisplay
void SDlgDataDisplay::Construct(const FArguments& InArgs, const TWeakObjectPtr<const UObject>& InWorldContextObjectPtr)
{
	WorldContextObjectPtr = InWorldContextObjectPtr;
	RootTreeItem = MakeShared<FDlgDataDisplayTreeRootNode>();
	ActorsTreeView = SNew(STreeView<TSharedPtr<FDlgDataDisplayTreeNode>>)
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
	// First, save off current expansion state
	TSet<TSharedPtr<FDlgDataDisplayTreeNode>> OldExpansionState;
	if (bPreserveExpansion)
	{
		ActorsTreeView->GetExpandedItems(OldExpansionState);
	}

	RootTreeItem->ClearChildren();
	RootChildren.Empty();
	ActorsProperties.Empty();

	// Try the actor World
	UWorld* World = WorldContextObjectPtr.IsValid() ? WorldContextObjectPtr->GetWorld() : nullptr;

// 	// Try The Editor World
// #if WITH_EDITOR
// 	if (World == nullptr && GEditor)
// 	{
// 		World = GEditor->GetEditorWorldContext().World();
// 	}
// #endif

	// Can't do anything without the world
	if (!IsValid(World))
	{
		FDlgLogger::Get().Error(
			TEXT("Failed to refresh SDlgDataDisplay tree. World is a null pointer. "
				"Is the game running? "
				"Did you setup the DlgSystem Console commands in your GameMode BeginPlay/StartPlay?")
		);
		return;
	}

	const TArray<UDlgDialogue*> Dialogues = UDlgManager::GetAllDialoguesFromMemory();
	const TArray<TWeakObjectPtr<AActor>> Actors = UDlgManager::GetAllWeakActorsWithDialogueParticipantInterface(World);

	// Build fast lookup for ParticipantNames
	// Maps from ParticipantName => Array of Dialogues that have this Participant.
	TMap<FName, TSet<TWeakObjectPtr<const UDlgDialogue>>> ParticipantNamesDialoguesMap;
	for (const UDlgDialogue* Dialogue : Dialogues)
	{
		TSet<FName> ParticipantsNames = Dialogue->GetParticipantNames();

		for (const FName& ParticipantName : ParticipantsNames)
		{
			TSet<TWeakObjectPtr<const UDlgDialogue>>* ValuePtr = ParticipantNamesDialoguesMap.Find(ParticipantName);
			if (ValuePtr == nullptr)
			{
				// does not exist
				TSet<TWeakObjectPtr<const UDlgDialogue>> ValueArray{Dialogue};
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
		TSet<TWeakObjectPtr<const UDlgDialogue>> ActorDialogues;
		TSet<TWeakObjectPtr<const UDlgDialogue>>* ActorDialoguesPtr = ParticipantNamesDialoguesMap.Find(ParticipantName);
		if (ActorDialoguesPtr != nullptr)
		{
			// Found some dialogue
			ActorDialogues = *ActorDialoguesPtr;
		}

		// Create Key in the ActorsProperties for this Actor.
		TSharedPtr<FDlgDataDisplayActorProperties> ActorsPropertiesValue =
			MakeShared<FDlgDataDisplayActorProperties>(ActorDialogues);
		ActorsProperties.Add(Actor, ActorsPropertiesValue);

		// Gather Data from the Dialogues
		for (TWeakObjectPtr<const UDlgDialogue> Dialogue : ActorDialogues)
		{
			if (!Dialogue.IsValid())
			{
				return;
			}

			// Populate Event Names
			const TSet<FName> EventsNames = Dialogue->GetParticipantEventNames(ParticipantName);
			for (const FName& EventName : EventsNames)
			{
				ActorsPropertiesValue->AddDialogueToEvent(EventName, Dialogue);
			}

			// Populate conditions
			const TSet<FName> ConditionNames = Dialogue->GetParticipantConditionNames(ParticipantName);
			for (const FName& ConditionName : ConditionNames)
			{
				ActorsPropertiesValue->AddDialogueToCondition(ConditionName, Dialogue);
			}

			// Populate int variable names
			const TSet<FName> IntVariableNames = Dialogue->GetParticipantIntNames(ParticipantName);
			for (const FName& IntVariableName : IntVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToIntVariable(IntVariableName, Dialogue);
			}

			// Populate float variable names
			const TSet<FName> FloatVariableNames = Dialogue->GetParticipantFloatNames(ParticipantName);
			for (const FName& FloatVariableName : FloatVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToFloatVariable(FloatVariableName, Dialogue);
			}

			// Populate bool variable names
			const TSet<FName> BoolVariableNames = Dialogue->GetParticipantBoolNames(ParticipantName);
			for (const FName& BoolVariableName : BoolVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToBoolVariable(BoolVariableName, Dialogue);
			}

			// Populate FName variable names
			const TSet<FName> FNameVariableNames = Dialogue->GetParticipantFNameNames(ParticipantName);
			for (const FName& NameVariableName : FNameVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToFNameVariable(NameVariableName, Dialogue);
			}

			// Populate UClass int variable names
			const TSet<FName> ClassIntVariableNames = Dialogue->GetParticipantClassIntNames(ParticipantName);
			for (const FName& IntVariableName : ClassIntVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToClassIntVariable(IntVariableName, Dialogue);
			}

			// Populate UClass float variable names
			const TSet<FName> ClassFloatVariableNames = Dialogue->GetParticipantClassFloatNames(ParticipantName);
			for (const FName& FloatVariableName : ClassFloatVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToClassFloatVariable(FloatVariableName, Dialogue);
			}

			// Populate UClass bool variable names
			const TSet<FName> ClassBoolVariableNames = Dialogue->GetParticipantClassBoolNames(ParticipantName);
			for (const FName& BoolVariableName : ClassBoolVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToClassBoolVariable(BoolVariableName, Dialogue);
			}

			// Populate UClass FName variable names
			const TSet<FName> ClassFNameVariableNames = Dialogue->GetParticipantClassFNameNames(ParticipantName);
			for (const FName& NameVariableName : ClassFNameVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToClassFNameVariable(NameVariableName, Dialogue);
			}

			// Populate UClass FText variable names
			const TSet<FName> ClassFTextVariableNames = Dialogue->GetParticipantClassFTextNames(ParticipantName);
			for (const FName& NameVariableName : ClassFTextVariableNames)
			{
				ActorsPropertiesValue->AddDialogueToClassFTextVariable(NameVariableName, Dialogue);
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

		AActor* Actor = Elem.Key.Get();
		TSharedPtr<FDlgDataDisplayTreeNode> ActorItem =
			MakeShared<FDlgDataDisplayTreeActorNode>(FText::FromString(Actor->GetName()), RootTreeItem, Actor);
		BuildTreeViewItem(ActorItem);
		RootTreeItem->AddChild(ActorItem);
	}
	RootChildren = RootTreeItem->GetChildren();

	// Clear Previous states
	ActorsTreeView->ClearSelection();
	// Triggers RequestTreeRefresh
	ActorsTreeView->ClearExpandedItems();

	// Restore old Expansion
	if (bPreserveExpansion && OldExpansionState.Num() > 0)
	{
		// Flattened tree
		TArray<TSharedPtr<FDlgDataDisplayTreeNode>> AllNodes;
		RootTreeItem->GetAllNodes(AllNodes);

		// Expand to match the old state
		FDlgTreeViewHelper::RestoreTreeExpansionState<TSharedPtr<FDlgDataDisplayTreeNode>>(ActorsTreeView,
			AllNodes, OldExpansionState, Self::PredicateCompareDlgDataDisplayTreeNode);
	}
}

void SDlgDataDisplay::GenerateFilteredItems()
{
	if (FilterString.IsEmpty())
	{
		// No filtering, empty filter, restore original
		RefreshTree(false);
		return;
	}

	// Get all valid paths
	TArray<TArray<TSharedPtr<FDlgDataDisplayTreeNode>>> OutPaths;
	RootTreeItem->FilterPathsToNodesThatContainText(FilterString, OutPaths);
	RootChildren.Empty();
	RootTreeItem->GetVisibleChildren(RootChildren);

	// Refresh, clear expansion
	ActorsTreeView->ClearExpandedItems(); // Triggers RequestTreeRefresh

	// Mark paths as expanded
	for (const TArray<TSharedPtr<FDlgDataDisplayTreeNode>>& Path : OutPaths)
	{
		const int32 PathNum = Path.Num();
		for (int32 PathIndex = 0; PathIndex < PathNum; PathIndex++)
		{
			Path[PathIndex]->SetIsVisible(true);
			ActorsTreeView->SetItemExpansion(Path[PathIndex], true);
		}
	}
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
		.HintText(LOCTEXT("SearchBoxHintText", "Search by Name"))
		.OnTextChanged(this, &Self::HandleSearchTextCommited, ETextCommit::Default)
		.OnTextCommitted(this, &Self::HandleSearchTextCommited)
		.SelectAllTextWhenFocused(false)
		.DelayChangeNotificationsWhileTyping(false);

	// Should return a valid widget
	return GetFilterTextBoxWidget();
}

void SDlgDataDisplay::AddVariableChildrenToItem(
	const TSharedPtr<FDlgDataDisplayTreeNode>& Item,
	const TMap<FName, TSharedPtr<FDlgDataDisplayVariableProperties>>& Variables,
	const FText& DisplayTextFormat,
	EDlgDataDisplayVariableTreeNodeType VariableType
)
{
	if (!Item.IsValid())
	{
		return;
	}

	for (const auto& Pair : Variables)
	{
		const FName VariableName = Pair.Key;
		FFormatOrderedArguments Args;
		Args.Add(FText::FromName(VariableName));
		const FText DisplayText = FText::Format(DisplayTextFormat, Args);

		// Create Node
		const TSharedPtr<FDlgDataDisplayTreeVariableNode> ChildItem =
			MakeShared<FDlgDataDisplayTreeVariableNode>(DisplayText, Item, VariableName, VariableType);
		Item->AddChild(ChildItem);
	}
}

void SDlgDataDisplay::BuildTreeViewItem(const TSharedPtr<FDlgDataDisplayTreeNode>& Item)
{
	TWeakObjectPtr<AActor> Actor = Item->GetParentActor();
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
				Item->AddChild(MakeShared<FDlgDataDisplayTreeCategoryNode>(
					LOCTEXT("EventKey", "Events"), Item, EDlgDataDisplayCategoryTreeNodeType::Event));
				Item->AddChild(MakeShared<FDlgDataDisplayTreeCategoryNode>(
					LOCTEXT("ConditionKey", "Conditions"), Item, EDlgDataDisplayCategoryTreeNodeType::Condition));
				Item->AddChild(MakeShared<FDlgDataDisplayTreeCategoryNode>(
					LOCTEXT("VariablesKey", "Variables"), Item, EDlgDataDisplayCategoryTreeNodeType::Variables));
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
					const TSharedPtr<FDlgDataDisplayTreeNode> EventItem = MakeShared<FDlgDataDisplayTreeVariableNode>(
						FText::FromName(Pair.Key), Item, Pair.Key, EDlgDataDisplayVariableTreeNodeType::Event
					);
					Item->AddChild(EventItem);
				}
				break;

			case EDlgDataDisplayCategoryTreeNodeType::Condition:
				for (const auto& Pair: ActorPropertiesValue->GetConditions())
				{
					const TSharedPtr<FDlgDataDisplayTreeNode> ConditionItem = MakeShared<FDlgDataDisplayTreeVariableNode>(
						FText::FromName(Pair.Key), Item, Pair.Key, EDlgDataDisplayVariableTreeNodeType::Condition
					);
					Item->AddChild(ConditionItem);
				}
				break;

			case EDlgDataDisplayCategoryTreeNodeType::Variables:
			{
				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetIntegers(),
					LOCTEXT("VariableIntKey", "int {0} = "), EDlgDataDisplayVariableTreeNodeType::Integer);
				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetFloats(),
					LOCTEXT("VariableFloatKey", "float {0} = "), EDlgDataDisplayVariableTreeNodeType::Float);
				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetBools(),
					LOCTEXT("VariableBoolKey", "bool {0} = "), EDlgDataDisplayVariableTreeNodeType::Bool);
				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetFNames(),
					LOCTEXT("VariableFNameKey", "FName {0} = "), EDlgDataDisplayVariableTreeNodeType::FName);

				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetClassIntegers(),
					LOCTEXT("VariableIntKey", "int {0} = "), EDlgDataDisplayVariableTreeNodeType::ClassInteger);
				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetClassFloats(),
					LOCTEXT("VariableFloatKey", "float {0} = "), EDlgDataDisplayVariableTreeNodeType::ClassFloat);
				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetClassBools(),
					LOCTEXT("VariableBoolKey", "bool {0} = "), EDlgDataDisplayVariableTreeNodeType::ClassBool);
				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetClassFNames(),
					LOCTEXT("VariableFNameKey", "FName {0} = "), EDlgDataDisplayVariableTreeNodeType::ClassFName);
				AddVariableChildrenToItem(Item, ActorPropertiesValue->GetClassFTexts(),
					LOCTEXT("VariableFTextKey", "FText {0} = "), EDlgDataDisplayVariableTreeNodeType::ClassFText);
				break;
			}
			default:
				unimplemented();
		}
	}

	// Recursively call on children
	for (const TSharedPtr<FDlgDataDisplayTreeNode>& ChildItem : Item->GetChildren())
	{
		BuildTreeViewItem(ChildItem);
	}
}

void SDlgDataDisplay::HandleSearchTextCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	// Trim and sanitized the filter text (so that it more likely matches)
	FilterString = FText::TrimPrecedingAndTrailing(InText).ToString();
	GenerateFilteredItems();
}

TSharedRef<ITableRow> SDlgDataDisplay::HandleGenerateRow(TSharedPtr<FDlgDataDisplayTreeNode> InItem,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	// Build row
	TSharedPtr<STableRow<TSharedPtr<FDlgDataDisplayTreeNode>>> TableRow;
	const FMargin RowPadding = FMargin(2.f, 2.f);
	TableRow = SNew(STableRow<TSharedPtr<FDlgDataDisplayTreeNode>>, OwnerTable)
		.Padding(1.0f);

	// Default row content
	TSharedPtr<STextBlock> DefaultTextBlock = SNew(STextBlock)
		.Text(InItem->GetDisplayText())
		.HighlightText(this, &Self::GetFilterText);

	TSharedPtr<SWidget> RowContent = DefaultTextBlock;
	TSharedPtr<SHorizontalBox> RowContainer;
	TableRow->SetRowContent(SAssignNew(RowContainer, SHorizontalBox));

	if (InItem->IsText())
	{
		// Add custom widget for variables/events/conditions
		if (InItem->GetTextType() == EDlgDataDisplayTextTreeNodeType::Variable)
		{
			TSharedPtr<SDlgDataPropertyValue> RightWidget;
			TSharedPtr<FDlgDataDisplayTreeVariableNode> VariableNode =
				StaticCastSharedPtr<FDlgDataDisplayTreeVariableNode>(InItem);

			// The widget on the right depends on the variable type.
			switch (VariableNode->GetVariableType())
			{
				case EDlgDataDisplayVariableTreeNodeType::Integer:
				case EDlgDataDisplayVariableTreeNodeType::Float:
				case EDlgDataDisplayVariableTreeNodeType::FName:
				case EDlgDataDisplayVariableTreeNodeType::ClassInteger:
				case EDlgDataDisplayVariableTreeNodeType::ClassFloat:
				case EDlgDataDisplayVariableTreeNodeType::ClassFName:
				case EDlgDataDisplayVariableTreeNodeType::ClassFText:
					// Editable text box
					SAssignNew(RightWidget, SDlgDataTextPropertyValue, VariableNode);
					break;

				case EDlgDataDisplayVariableTreeNodeType::Event:
					// Trigger Event Button
					SAssignNew(RightWidget, SDlgDataEventPropertyValue, VariableNode);
					break;

				case EDlgDataDisplayVariableTreeNodeType::Bool:
				case EDlgDataDisplayVariableTreeNodeType::ClassBool:
				case EDlgDataDisplayVariableTreeNodeType::Condition:
					// Checkbox
					SAssignNew(RightWidget, SDlgDataBoolPropertyValue, VariableNode);
					break;

				case EDlgDataDisplayVariableTreeNodeType::Default:
				default:
					// Static text
					SAssignNew(RightWidget, SDlgDataPropertyValue, VariableNode);
					break;
			}

			RowContent = SNew(SHorizontalBox)
				// <variable type> <variable name> =
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					DefaultTextBlock.ToSharedRef()
				]

				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				[
					RightWidget.ToSharedRef()
				];
		}
	}

	// Add expand arrow
	RowContainer->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Right)
		[
			SNew(SExpanderArrow, TableRow)
		];

	// Add the row content
	RowContainer->AddSlot()
		.FillWidth(1.0)
		.Padding(RowPadding)
		[
			RowContent.ToSharedRef()
		];

	return TableRow.ToSharedRef();
}

void SDlgDataDisplay::HandleGetChildren(TSharedPtr<FDlgDataDisplayTreeNode> InItem,
	TArray<TSharedPtr<FDlgDataDisplayTreeNode>>& OutChildren)
{
	if (!InItem.IsValid())
	{
		return;
	}

	OutChildren = InItem->GetChildren();
}

void SDlgDataDisplay::HandleTreeSelectionChanged(TSharedPtr<FDlgDataDisplayTreeNode> InItem, ESelectInfo::Type SelectInfo)
{
	// Ignored
}

void SDlgDataDisplay::HandleDoubleClick(TSharedPtr<FDlgDataDisplayTreeNode> InItem)
{
	if (!InItem.IsValid())
	{
		return;
	}

	// Expand on double click
	if (InItem->HasChildren())
	{
		ActorsTreeView->SetItemExpansion(InItem, !ActorsTreeView->IsItemExpanded(InItem));
	}
}

void SDlgDataDisplay::HandleSetExpansionRecursive(TSharedPtr<FDlgDataDisplayTreeNode> InItem, bool bInIsItemExpanded)
{
	if (InItem.IsValid() && InItem->HasChildren())
	{
		ActorsTreeView->SetItemExpansion(InItem, bInIsItemExpanded);
		for (const TSharedPtr<FDlgDataDisplayTreeNode>& Child : InItem->GetChildren())
		{
			HandleSetExpansionRecursive(Child, bInIsItemExpanded);
		}
	}
}

#undef LOCTEXT_NAMESPACE
