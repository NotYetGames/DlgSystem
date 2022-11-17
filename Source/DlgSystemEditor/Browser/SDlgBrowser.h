// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SSearchBox.h"

#include "DlgBrowserTreeNode.h"
#include "DialogueTreeProperties/DlgBrowserTreeParticipantProperties.h"
#include "DlgBrowserUtilities.h"

enum class EDlgBlueprintOpenType : unsigned char;
class UDlgDialogue;
class SImage;

/**
 * Implements the Dialogue Browser
 */
class SDlgBrowser : public SCompoundWidget
{
	typedef SDlgBrowser Self;
	typedef TSharedPtr<FDlgBrowserSortOption> SortOptionType;

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
		const TSharedPtr<FDlgBrowserTreeNode>& InItem,
		const TSharedPtr<FDlgBrowserTreeVariableProperties>* PropertyPtr,
		EDlgTreeNodeTextType TextType
	);

	// Add GraphNode Children of type TextType to the InItem.
	void AddGraphNodeChildrenToItem(
		const TSharedPtr<FDlgBrowserTreeNode>& InItem,
		const TSet<TWeakObjectPtr<const UDialogueGraphNode>>& GraphNodes,
		EDlgTreeNodeTextType TextType
	);

	// Add EdgeNode Children of type TextType to the InItem.
	void AddEdgeNodeChildrenToItem(
		const TSharedPtr<FDlgBrowserTreeNode>& InItem,
		const TSet<TWeakObjectPtr<const UDialogueGraphNode_Edge>>& EdgeNodes,
		EDlgTreeNodeTextType TextType
	);

	// Add both GraphNode Children and EdgeNode Children to the InItem from The Property.
	void AddGraphNodeBaseChildrenToItemFromProperty(
		const TSharedPtr<FDlgBrowserTreeNode>& InItem,
		const TSharedPtr<FDlgBrowserTreeVariableProperties>* PropertyPtr,
		EDlgTreeNodeTextType GraphNodeTextType,
		EDlgTreeNodeTextType EdgeNodeTextType);

	// Adds the Variables to the Item
	void AddVariableChildrenToItem(
		const TSharedPtr<FDlgBrowserTreeNode>& Item,
		const TMap<FName, TSharedPtr<FDlgBrowserTreeVariableProperties>>& Variables,
		EDlgTreeNodeTextType VariableType
	);

	// Recursively build the view item.
	void BuildTreeViewItem(const TSharedPtr<FDlgBrowserTreeNode>& Item);

	// helper function to generate inline widgets for item.
	TSharedRef<SWidget> MakeInlineWidget(const TSharedPtr<FDlgBrowserTreeNode>& InItem);

	// Make the row of buttons for the graph nodes.
	TSharedRef<SWidget> MakeButtonWidgetForGraphNodes(const TArray<TSharedPtr<FDlgBrowserTreeNode>>& InChildren);

	// Make the buttons to open the dialogue.
	TSharedRef<SWidget> MakeButtonsWidgetForDialogue(const TSharedPtr<FDlgBrowserTreeNode>& InItem);

	// Text search changed
	void HandleSearchTextCommitted(const FText& InText, ETextCommit::Type InCommitType);

	// Make the row
	TSharedRef<ITableRow> HandleGenerateRow(TSharedPtr<FDlgBrowserTreeNode> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	// General Get children
	void HandleGetChildren(TSharedPtr<FDlgBrowserTreeNode> InItem, TArray<TSharedPtr<FDlgBrowserTreeNode>>& OutChildren);

	// Handles changes in the Tree View.
	void HandleTreeSelectionChanged(TSharedPtr<FDlgBrowserTreeNode> NewValue, ESelectInfo::Type SelectInfo);

	// User clicked on item.
	void HandleDoubleClick(TSharedPtr<FDlgBrowserTreeNode> InItem);

	// Refresh button clicked.
	FReply HandleOnRefresh()
	{
		RefreshTree(true);
		return FReply::Handled();
	}

	// Callback for expanding tree items recursively
	void HandleSetExpansionRecursive(TSharedPtr<FDlgBrowserTreeNode> InItem, bool bInIsItemExpanded);

	// When the sort option selection changes.
	void HandleSortSelectionChanged(SortOptionType Selection, ESelectInfo::Type SelectInfo);

	// Finds the object of the item inside the Content browser.
	FReply FindInContentBrowserForItem(TSharedPtr<FDlgBrowserTreeNode> InItem);

	// Makes Participant categories.
	TArray<TSharedPtr<FDlgBrowserTreeNode>> MakeParticipantCategoriesChildren(
		const TSharedPtr<FDlgBrowserTreeNode>& Parent,
		const TSharedPtr<FDlgBrowserTreeParticipantProperties>& ParticipantProperties,
		bool bHideEmptyCategories
	) const;

	// Makes Variable categories.
	TArray<TSharedPtr<FDlgBrowserTreeNode>> MakeVariableCategoriesChildren(
		const TSharedPtr<FDlgBrowserTreeNode>& Parent,
		const TSharedPtr<FDlgBrowserTreeParticipantProperties>& ParticipantProperties,
		bool bHideEmptyCategories
	) const;

	// Makes Class Variable categories.
	TArray<TSharedPtr<FDlgBrowserTreeNode>> MakeClassVariableCategoriesChildren(
		const TSharedPtr<FDlgBrowserTreeNode>& Parent,
		const TSharedPtr<FDlgBrowserTreeParticipantProperties>& ParticipantProperties,
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
		EDlgBlueprintOpenType OpenType,
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
		EDlgBlueprintOpenType OpenType,
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
	TSharedPtr<FDlgBrowserTreeNode> RootTreeItem;

	// The root children. Kept separate so that we do not corrupt the data.
	TArray<TSharedPtr<FDlgBrowserTreeNode>> RootChildren;

	// Tree view for showing all participants, etc.
	TSharedPtr<STreeView<TSharedPtr<FDlgBrowserTreeNode>>> ParticipantsTreeView;

	/**
	 * Used for fast lookup of each participants
	 * Key: Participant Name
	 * Value: participant properties
	 */
	TMap<FName, TSharedPtr<FDlgBrowserTreeParticipantProperties>> ParticipantsProperties;

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
