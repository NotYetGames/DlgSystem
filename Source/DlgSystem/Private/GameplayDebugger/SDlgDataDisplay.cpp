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
		FDlgDataDisplayActorPropertiesPtr ActorsPropertiesValue =
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
				// PopulateVariablePropertiesFromSearchResult(
				// 	ParticipantProps->AddDialogueToEvent(EventName, Dialogue),
				// 	FDialogueSearchUtilities::GetGraphNodesForEventEventName(EventName, Dialogue),
				// 	DialogueGuid);
			}
		}
	}

	// Build the Actors Tree View (aka the actual tree)
	for (const auto Elem : ActorsProperties)
	{
		if (!Elem.Key.IsValid())
		{
			continue;
		}

		const AActor* Actor = Elem.Key.Get();
		RootTreeItem->AddChild(
			MakeShareable(new FDlgDataDisplayTreeActorNode(FText::FromString(Actor->GetName()), RootTreeItem, Actor)));
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
	TWeakObjectPtr<const AActor> Actor = Item->GetParentActor();
	if (!Actor.IsValid())
	{
		return;
	}

	// Do we have the actor cached?
	FDlgDataDisplayActorPropertiesPtr* ValuePtr = ActorsProperties.Find(Actor);
	if (ValuePtr == nullptr)
	{
		return;
	}

	FDlgDataDisplayActorPropertiesPtr ActorProperties = *ValuePtr;
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
