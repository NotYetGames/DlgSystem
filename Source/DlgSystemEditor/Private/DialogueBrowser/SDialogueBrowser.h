// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SCompoundWidget.h"
#include "SListView.h"
#include "STreeView.h"
#include "SSearchBox.h"

#include "DialogueBrowserTreeNode.h"
#include "DialogueTreeProperties/DialogueBrowserTreeParticipantProperties.h"
#include "DialogueBrowserUtilities.h"

class UDlgDialogue;
class SImage;

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
	void AddDialogueChildrenToItemFromProperty(FDialogueBrowserTreeNodePtr InItem,
								   const TSharedPtr<FDialogueBrowserTreeVariableProperties>* PropertyPtr,
								   const EDialogueTreeNodeTextType TextType);

	/** Add GraphNode Children of type TextType to the InItem. */
	void AddGraphNodeChildrenToItem(FDialogueBrowserTreeNodePtr InItem,
									const TSet<TWeakObjectPtr<UDialogueGraphNode>>& GraphNodes,
									const EDialogueTreeNodeTextType TextType);

	/** Add EdgeNode Children of type TextType to the InItem. */
	void AddEdgeNodeChildrenToItem(FDialogueBrowserTreeNodePtr InItem,
									const TSet<TWeakObjectPtr<UDialogueGraphNode_Edge>>& EdgeNodes,
									const EDialogueTreeNodeTextType TextType);

	/** Add both GraphNode Children and EdgeNode Children to the InItem from The Property. */
	void AddGraphNodeBaseChildrenToItemFromProperty(FDialogueBrowserTreeNodePtr InItem,
													 const TSharedPtr<FDialogueBrowserTreeVariableProperties>* PropertyPtr,
													 const EDialogueTreeNodeTextType GraphNodeTextType,
													 const EDialogueTreeNodeTextType EdgeNodeTextType);

	/** Recursively build the view item. */
	void BuildTreeViewItem(FDialogueBrowserTreeNodePtr Item);

	// helper function to generate inline widgets for item.
	TSharedRef<SWidget> MakeInlineWidget(const FDialogueBrowserTreeNodePtr InItem);

	/** Make the row of buttons for the graph nodes. */
	TSharedRef<SWidget> MakeButtonWidgetForGraphNodes(const TArray<FDialogueBrowserTreeNodePtr>& InChildren);

	/** Make the buttons to open the dialogue. */
	TSharedRef<SWidget> MakeButtonsWidgetForDialogue(const FDialogueBrowserTreeNodePtr InItem);

	/** Text search changed */
	void HandleSearchTextCommited(const FText& InText, ETextCommit::Type InCommitType);

	/** Make the row */
	TSharedRef<ITableRow> HandleGenerateRow(FDialogueBrowserTreeNodePtr InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/** General Get children  */
	void HandleGetChildren(FDialogueBrowserTreeNodePtr InItem, TArray<FDialogueBrowserTreeNodePtr>& OutChildren);

	/** Handles changes in the Tree View. */
	void HandleTreeSelectionChanged(FDialogueBrowserTreeNodePtr NewValue, ESelectInfo::Type SelectInfo);

	/** User clicked on item. */
	void HandleDoubleClick(FDialogueBrowserTreeNodePtr InItem);

	/** Refresh button clicked. */
	FReply HandleOnRefresh()
	{
		RefreshTree(true);
		return FReply::Handled();
	}

	/** Callback for expanding tree items recursively */
	void HandleSetExpansionRecursive(FDialogueBrowserTreeNodePtr InItem, bool bInIsItemExpanded);

	/** When the sort option selection changes. */
	void HandleSortSelectionChanged(SortOptionType Selection, ESelectInfo::Type SelectInfo);

	/** Finds the object of the item inside the Content browser. */
	FReply FindInContentBrowserForItem(const FDialogueBrowserTreeNodePtr InItem);

	/** Makes Participant categories. */
	TArray<FDialogueBrowserTreeNodePtr> MakeParticipantCategoriesChildren(FDialogueBrowserTreeNodePtr Parent) const;

	/** Makes Variable categories. */
	TArray<FDialogueBrowserTreeNodePtr> MakeVariableCategoriesChildren(FDialogueBrowserTreeNodePtr Parent) const;

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
		const FSlateBrush* IconBrush, const int32 IconSize = 24);

private:
	/** The search box */
	TSharedPtr<SSearchBox> FilterTextBoxWidget;

	/** The filter text from the search box. */
	FString FilterString;

	/** The root data source */
	FDialogueBrowserTreeNodePtr RootTreeItem;

	/** The root children. Kept seperate so that we do not corrupt the data. */
	TArray<FDialogueBrowserTreeNodePtr> RootChildren;

	/** Tree view for showing all participants, etc. */
	TSharedPtr<STreeView<FDialogueBrowserTreeNodePtr>> ParticipantsTreeView;

	/**
	 * Used for fast lookup of each participants
	 * Key: Participant Name
	 * Value: participant properties
	 */
	TMap<FName, TSharedPtr<FDialogueBrowserTreeParticipantProperties>> ParticipantsProperties;

	// Sort variables
	/** The data sources */
	TArray<SortOptionType> SortOptions;

	/** Default option. */
	SortOptionType DefaultSortOption;

	/** selected option. */
	SortOptionType SelectedSortOption;
};
