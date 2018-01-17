// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SCompoundWidget.h"
#include "SListView.h"
#include "STreeView.h"
#include "SImage.h"
#include "SSearchBox.h"

#include "DialogueTreeNode.h"
#include "DialogueTreeProperties/ParticipantProperties_DialogueTree.h"
#include "DialogueBrowserUtilities.h"

class UDlgDialogue;

/**
 * Implements the Dialogue Browser
 */
class SDialogueBrowser : public SCompoundWidget
{
	typedef SDialogueBrowser Self;
	typedef TSharedPtr<FDialogueBrowserSortOption> SortOptionType;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Updates the participants tree. */
	void RefreshTree(bool bPreserveExpansion);

	/** Get current filter text */
	FText GetFilterText() const { return FilterTextBoxWidget->GetText(); }

private:
	/** Handle filtering. */
	void GenerateFilteredItems();

	/** Getters for widgets.  */
	TSharedRef<SWidget> GetFilterTextBoxWidget();

	/** Add Dialogue Children of type TextType to the InItem. */
	void AddDialogueChildrenToItemFromProperty(FDialogueTreeNodePtr InItem,
								   const FDlgTreeVariablePropertiesPtr* PropertyPtr,
								   const EDialogueTreeNodeTextType TextType);

	/** Add GraphNode Children of type TextType to the InItem. */
	void AddGraphNodeChildrenToItem(FDialogueTreeNodePtr InItem,
									const TSet<TWeakObjectPtr<UDialogueGraphNode>>& GraphNodes,
									const EDialogueTreeNodeTextType TextType);

	/** Add EdgeNode Children of type TextType to the InItem. */
	void AddEdgeNodeChildrenToItem(FDialogueTreeNodePtr InItem,
									const TSet<TWeakObjectPtr<UDialogueGraphNode_Edge>>& EdgeNodes,
									const EDialogueTreeNodeTextType TextType);

	/** Add both GraphNode Children and EdgeNode Children to the InItem from The Property. */
	void AddGraphNodeBaseChildrenToItemFromProperty(FDialogueTreeNodePtr InItem,
													 const FDlgTreeVariablePropertiesPtr* PropertyPtr,
													 const EDialogueTreeNodeTextType GraphNodeTextType,
													 const EDialogueTreeNodeTextType EdgeNodeTextType);

	/** Recursively build the view item. */
	void BuildTreeViewItem(FDialogueTreeNodePtr Item);

	// helper function to generate inline widgets for item.
	TSharedRef<SWidget> MakeInlineWidget(const FDialogueTreeNodePtr InItem);

	/** Make the row of buttons for the graph nodes. */
	TSharedRef<SWidget> MakeButtonWidgetForGraphNodes(const TArray<FDialogueTreeNodePtr>& InChildren);

	/** Make the buttons to open the dialogue. */
	TSharedRef<SWidget> MakeButtonsWidgetForDialogue(const FDialogueTreeNodePtr InItem);

	/** Text search changed */
	void HandleSearchTextCommited(const FText& InText, ETextCommit::Type InCommitType);

	/** Make the row */
	TSharedRef<ITableRow> HandleGenerateRow(FDialogueTreeNodePtr InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/** General Get children  */
	void HandleGetChildren(FDialogueTreeNodePtr InItem, TArray<FDialogueTreeNodePtr>& OutChildren);

	/** Handles changes in the ParticipantsListViewWidget */
	void HandleParticipantsTreeSelectionChanged(FDialogueTreeNodePtr NewValue, ESelectInfo::Type SelectInfo);

	/** User clicked on item. */
	void HandleDoubleClick(FDialogueTreeNodePtr InItem);

	/** Refresh button clicked. */
	FReply HandleOnRefresh()
	{
		RefreshTree(true);

		return FReply::Handled();
	}

	/** Callback for expanding tree items recursively */
	void HandleSetExpansionRecursive(FDialogueTreeNodePtr InItem, bool bInIsItemExpanded);

	/** When the sort option selection changes. */
	void HandleSortSelectionChanged(SortOptionType Selection, ESelectInfo::Type SelectInfo);

	/** Opens the specified editor for an item. */
	FReply OpenEditorForItem(const FDialogueTreeNodePtr InItem);

	/** Jumps to the specified node represent by the item. */
	FReply JumpToNodeForItem(const FDialogueTreeNodePtr InItem);

	/** Finds the object of the item inside the Content browser. */
	FReply FindInContentBrowserForItem(const FDialogueTreeNodePtr InItem);

	/** Makes Participant categories. */
	TArray<FDialogueTreeNodePtr> MakeParticipantCategoriesChildren(const FName& ParticipantName) const
	{
		TArray<FDialogueTreeNodePtr> Categories;
		{
			FDialogueTreeNodePtr Category = FDialogueTreeNode::MakeCategory(TEXT("Dialogues"), ParticipantName);
			Category->SetCategoryType(EDialogueTreeNodeCategoryType::Dialogue);
			Categories.Add(Category);
		}
		{
			FDialogueTreeNodePtr Category = FDialogueTreeNode::MakeCategory(TEXT("Events"), ParticipantName);
			Category->SetCategoryType(EDialogueTreeNodeCategoryType::Event);
			Categories.Add(Category);
		}
		{
			FDialogueTreeNodePtr Category = FDialogueTreeNode::MakeCategory(TEXT("Conditions"), ParticipantName);
			Category->SetCategoryType(EDialogueTreeNodeCategoryType::Condition);
			Categories.Add(Category);
		}
		{
			FDialogueTreeNodePtr Category = FDialogueTreeNode::MakeCategory(TEXT("Variables"), ParticipantName);
			Category->SetCategoryType(EDialogueTreeNodeCategoryType::Variable);
			Categories.Add(Category);
		}
		return Categories;
	}

	/** Makes Variable categories. */
	TArray<FDialogueTreeNodePtr> MakeVariableCategoriesChildren(const FName& ParticipantName) const
	{
		TArray<FDialogueTreeNodePtr> Categories;
		{
			FDialogueTreeNodePtr Category = FDialogueTreeNode::MakeCategory(TEXT("Integers"), ParticipantName);
			Category->SetCategoryType(EDialogueTreeNodeCategoryType::VariableInt);
			Categories.Add(Category);
		}
		{
			FDialogueTreeNodePtr Category = FDialogueTreeNode::MakeCategory(TEXT("Floats"), ParticipantName);
			Category->SetCategoryType(EDialogueTreeNodeCategoryType::VariableFloat);
			Categories.Add(Category);
		}
		{
			FDialogueTreeNodePtr Category = FDialogueTreeNode::MakeCategory(TEXT("Bools"), ParticipantName);
			Category->SetCategoryType(EDialogueTreeNodeCategoryType::VariableBool);
			Categories.Add(Category);
		}
		return Categories;
	}

	template<typename ItemType, typename ComparisonType>
	void RestoreExpansionState(TSharedPtr<STreeView<ItemType>> InTree, const TArray<ItemType>& ItemSource,
							   const TSet<ItemType>& OldExpansionState, ComparisonType ComparisonFunction)
	{
		check(InTree.IsValid());

		// Iterate over new tree items
		for(int32 ItemIdx = 0; ItemIdx < ItemSource.Num(); ItemIdx++)
		{
			ItemType NewItem = ItemSource[ItemIdx];

			// Look through old expansion state
			for (const ItemType OldItem : OldExpansionState)
			{
				// See if this matches this new item
				if (ComparisonFunction(OldItem, NewItem))
				{
					// It does, so expand it
					InTree->SetItemExpansion(NewItem, true);
				}
			}
		}
	}

	/** Makes a widget that has IconName. Text of item. */
	TSharedRef<SHorizontalBox> MakeIconAndTextWidget(const FText& InText,
		const FSlateBrush* IconBrush, const int32 IconSize = 24)
	{
		return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.WidthOverride(IconSize)
				.HeightOverride(IconSize)
				[
					SNew(SImage)
					.Image(IconBrush)
				]
			]

			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(2.f, 0.f)
			[
				SNew(STextBlock)
				.Text(InText)
				.HighlightText(this, &Self::GetFilterText)
			];
	}

	static bool PredicateCompareDialogueTreeNode(FDialogueTreeNodePtr FirstNode, FDialogueTreeNodePtr SecondNode)
	{
		check(FirstNode.IsValid());
		check(SecondNode.IsValid());
		return *FirstNode == *SecondNode;
	}

	/** Predicate that sorts participants by dialogue number references, in descending order. */
	static bool PredicateSortByDialoguesNumDescending(const FName& FirstParticipant, const FName& SecondParticipant,
		const TMap<FName, FDlgTreeParticipantPropertiesPtr>& ParticipantsProperties)
	{
		int32 FirstNum = 0;
		int32 SecondNum = 0;

		const FDlgTreeParticipantPropertiesPtr* FirstPtr = ParticipantsProperties.Find(FirstParticipant);
		if (FirstPtr)
		{
			FirstNum = (*FirstPtr)->GetDialogues().Num();
		}
		const FDlgTreeParticipantPropertiesPtr* SecondPtr = ParticipantsProperties.Find(SecondParticipant);
		if (SecondPtr)
		{
			SecondNum = (*SecondPtr)->GetDialogues().Num();
		}

		return FirstNum > SecondNum;
	}

private:
	/** The search box */
	TSharedPtr<SSearchBox> FilterTextBoxWidget;

	/** The filter text from the search box. */
	FString FilterString;

	/** The root data source */
	FDialogueTreeNodePtr RootTreeItem;

	/** The root children. */
	TArray<FDialogueTreeNodePtr> RootChildren;

	/** The original root tree item, before filter was applied. */
	FDialogueTreeNodePtr OriginalRootTreeItem;

	/** Tree view for showing all participants, etc. */
	TSharedPtr<STreeView<FDialogueTreeNodePtr>> ParticipantsTreeView;

	/**
	 * Used for fast lookup of each participants
	 * Key: Participant Name
	 * Value: participant properties
	 */
	TMap<FName, FDlgTreeParticipantPropertiesPtr> ParticipantsProperties;

	// Sort variables
	/** The data sources */
	TArray<SortOptionType> SortOptions;

	/** Default option. */
	SortOptionType DefaultSortOption;

	/** selected option. */
	SortOptionType SelectedSortOption;
};

