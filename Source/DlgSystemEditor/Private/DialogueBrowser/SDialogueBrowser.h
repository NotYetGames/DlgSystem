// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SSearchBox.h"

#include "DialogueBrowserTreeNode.h"
#include "DialogueTreeProperties/DialogueBrowserTreeParticipantProperties.h"
#include "DialogueBrowserUtilities.h"

enum class EDialogueBlueprintOpenType : unsigned char;
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

	// Updates the participants tree.
	void RefreshTree(bool bPreserveExpansion);

	// Get current filter text
	FText GetFilterText() const { return FilterTextBoxWidget->GetText(); }

protected:
	// Handle filtering.
	void GenerateFilteredItems();

	// Getters for widgets.
	TSharedRef<SWidget> GetFilterTextBoxWidget();

	// Add Dialogue Children of type TextType to the InItem.
	void AddDialogueChildrenToItemFromProperty(
		const TSharedPtr<FDialogueBrowserTreeNode>& InItem,
		const TSharedPtr<FDialogueBrowserTreeVariableProperties>* PropertyPtr,
		EDialogueTreeNodeTextType TextType
	);

	// Add GraphNode Children of type TextType to the InItem.
	void AddGraphNodeChildrenToItem(
		const TSharedPtr<FDialogueBrowserTreeNode>& InItem,
		const TSet<TWeakObjectPtr<const UDialogueGraphNode>>& GraphNodes,
		EDialogueTreeNodeTextType TextType
	);

	// Add EdgeNode Children of type TextType to the InItem.
	void AddEdgeNodeChildrenToItem(
		const TSharedPtr<FDialogueBrowserTreeNode>& InItem,
		const TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>& EdgeNodes,
		EDialogueTreeNodeTextType TextType
	);

	// Add both GraphNode Children and EdgeNode Children to the InItem from The Property.
	void AddGraphNodeBaseChildrenToItemFromProperty(
		const TSharedPtr<FDialogueBrowserTreeNode>& InItem,
		const TSharedPtr<FDialogueBrowserTreeVariableProperties>* PropertyPtr,
		EDialogueTreeNodeTextType GraphNodeTextType,
		EDialogueTreeNodeTextType EdgeNodeTextType);

	// Adds the Variables to the Item
	void AddVariableChildrenToItem(
		const TSharedPtr<FDialogueBrowserTreeNode>& Item,
		const TMap<FName, TSharedPtr<FDialogueBrowserTreeVariableProperties>>& Variables,
		EDialogueTreeNodeTextType VariableType
	);

	// Recursively build the view item.
	void BuildTreeViewItem(const TSharedPtr<FDialogueBrowserTreeNode>& Item);

	// helper function to generate inline widgets for item.
	TSharedRef<SWidget> MakeInlineWidget(const TSharedPtr<FDialogueBrowserTreeNode>& InItem);

	// Make the row of buttons for the graph nodes.
	TSharedRef<SWidget> MakeButtonWidgetForGraphNodes(const TArray<TSharedPtr<FDialogueBrowserTreeNode>>& InChildren);

	// Make the buttons to open the dialogue.
	TSharedRef<SWidget> MakeButtonsWidgetForDialogue(const TSharedPtr<FDialogueBrowserTreeNode>& InItem);

	// Text search changed
	void HandleSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType);

	// Make the row
	TSharedRef<ITableRow> HandleGenerateRow(TSharedPtr<FDialogueBrowserTreeNode> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	// General Get children
	void HandleGetChildren(TSharedPtr<FDialogueBrowserTreeNode> InItem, TArray<TSharedPtr<FDialogueBrowserTreeNode>>& OutChildren);

	// Handles changes in the Tree View.
	void HandleTreeSelectionChanged(TSharedPtr<FDialogueBrowserTreeNode> NewValue, ESelectInfo::Type SelectInfo);

	// User clicked on item.
	void HandleDoubleClick(TSharedPtr<FDialogueBrowserTreeNode> InItem);

	// Refresh button clicked.
	FReply HandleOnRefresh()
	{
		RefreshTree(true);
		return FReply::Handled();
	}

	// Callback for expanding tree items recursively
	void HandleSetExpansionRecursive(TSharedPtr<FDialogueBrowserTreeNode> InItem, bool bInIsItemExpanded);

	// When the sort option selection changes.
	void HandleSortSelectionChanged(SortOptionType Selection, ESelectInfo::Type SelectInfo);

	// Finds the object of the item inside the Content browser.
	FReply FindInContentBrowserForItem(TSharedPtr<FDialogueBrowserTreeNode> InItem);

	// Makes Participant categories.
	TArray<TSharedPtr<FDialogueBrowserTreeNode>> MakeParticipantCategoriesChildren(
		const TSharedPtr<FDialogueBrowserTreeNode>& Parent,
		const TSharedPtr<FDialogueBrowserTreeParticipantProperties>& ParticipantProperties,
		bool bHideEmptyCategories
	) const;

	// Makes Variable categories.
	TArray<TSharedPtr<FDialogueBrowserTreeNode>> MakeVariableCategoriesChildren(
		const TSharedPtr<FDialogueBrowserTreeNode>& Parent,
		const TSharedPtr<FDialogueBrowserTreeParticipantProperties>& ParticipantProperties,
		bool bHideEmptyCategories
	) const;

	// Makes Class Variable categories.
	TArray<TSharedPtr<FDialogueBrowserTreeNode>> MakeClassVariableCategoriesChildren(
		const TSharedPtr<FDialogueBrowserTreeNode>& Parent,
		const TSharedPtr<FDialogueBrowserTreeParticipantProperties>& ParticipantProperties,
		bool bHideEmptyCategories
	) const;

	// Makes a widget that has IconName. Text of item.
	TSharedRef<SHorizontalBox> MakeIconAndTextWidget(
		const FText& InText,
		const FSlateBrush* IconBrush,
		int32 IconSize = 24
	);

	// Makes a widget for a Class that has IconName. Text of item.
	TSharedRef<SHorizontalBox> MakeCustomObjectIconAndTextWidget(
        const FText& InText,
        const FSlateBrush* IconBrush,
        UClass* Class,
        EDialogueBlueprintOpenType OpenType,
		FName FunctionNameToOpen,
        int32 IconSize = 24
    );

	// Fills the menu of the View Options
	TSharedRef<SWidget>	FillViewOptionsEntries();

	static EVisibility GetOpenAssetButtonVisibility(UClass* Class);
	static EVisibility GetBrowseAssetButtonVisibility(UClass* Class);
	static FReply OnBrowseAssetClicked(UClass* Class);
	static FReply OnOpenAssetClicked(
		UClass* Class,
		EDialogueBlueprintOpenType OpenType,
		FName FunctionNameToOpen
	);
	static FText GetJumpToAssetText(UClass* Class);
	static FText GetBrowseAssetText(UClass* Class);

protected:
	// The search box
	TSharedPtr<SSearchBox> FilterTextBoxWidget;

	// The filter text from the search box.
	FString FilterString;

	// The root data source
	TSharedPtr<FDialogueBrowserTreeNode> RootTreeItem;

	// The root children. Kept separate so that we do not corrupt the data.
	TArray<TSharedPtr<FDialogueBrowserTreeNode>> RootChildren;

	// Tree view for showing all participants, etc.
	TSharedPtr<STreeView<TSharedPtr<FDialogueBrowserTreeNode>>> ParticipantsTreeView;

	/**
	 * Used for fast lookup of each participants
	 * Key: Participant Name
	 * Value: participant properties
	 */
	TMap<FName, TSharedPtr<FDialogueBrowserTreeParticipantProperties>> ParticipantsProperties;

	//
	// Sort variables
	//

	// The data sources
	TArray<SortOptionType> SortOptions;

	// Default option.
	SortOptionType DefaultSortOption;

	// Selected option.
	SortOptionType SelectedSortOption;
};
