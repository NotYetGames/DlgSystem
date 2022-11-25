// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SSearchBox.h"

#include "DlgDataDisplayTreeNode.h"
#include "DlgDataDisplayActorProperties.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgSystemDataDisplay, Verbose, All);

// Implements the Runtime Dialogue Data Display
class DLGSYSTEM_API SDlgDataDisplay : public SCompoundWidget
{
	typedef SDlgDataDisplay Self;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TWeakObjectPtr<const UObject>& InWorldContextObjectPtr);

	void SetWorldContextObject(const TWeakObjectPtr<const UObject>& InWorldContextObjectPtr)
	{
		WorldContextObjectPtr = InWorldContextObjectPtr;
	}

	// Updates the actors tree.
	void RefreshTree(bool bPreserveExpansion);

	// Get current filter text
	FText GetFilterText() const { return FilterTextBoxWidget->GetText(); }

private:
	// Handle filtering.
	void GenerateFilteredItems();

	// Getters for widgets.
	TSharedRef<SWidget> GetFilterTextBoxWidget();

	// Add the Variables as children to the Item
	void AddVariableChildrenToItem(
		const TSharedPtr<FDlgDataDisplayTreeNode>& Item,
		const TMap<FName, TSharedPtr<FDlgDataDisplayVariableProperties>>& Variables,
		const FText& DisplayTextFormat,
		EDlgDataDisplayVariableTreeNodeType VariableType
	);

	// Recursively build the view item.
	void BuildTreeViewItem(const TSharedPtr<FDlgDataDisplayTreeNode>& Item);

	// Text search changed
	void HandleSearchTextCommited(const FText& InText, ETextCommit::Type InCommitType);

	// Refresh button clicked.
	FReply HandleOnRefresh()
	{
		RefreshTree(true);
		return FReply::Handled();
	}

	// Make the row
	TSharedRef<ITableRow> HandleGenerateRow(TSharedPtr<FDlgDataDisplayTreeNode> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	// General Get children
	void HandleGetChildren(TSharedPtr<FDlgDataDisplayTreeNode> InItem, TArray<TSharedPtr<FDlgDataDisplayTreeNode>>& OutChildren);

	// Handles changes in the Tree View.
	void HandleTreeSelectionChanged(TSharedPtr<FDlgDataDisplayTreeNode> InItem, ESelectInfo::Type SelectInfo);

	// User clicked on item.
	void HandleDoubleClick(TSharedPtr<FDlgDataDisplayTreeNode> InItem);

	// Callback for expanding tree items recursively
	void HandleSetExpansionRecursive(TSharedPtr<FDlgDataDisplayTreeNode> InItem, bool bInIsItemExpanded);

	// Compare two FDlgDataDisplayTreeNode
	static bool PredicateCompareDlgDataDisplayTreeNode(const TSharedPtr<FDlgDataDisplayTreeNode>& FirstNode, const TSharedPtr<FDlgDataDisplayTreeNode>& SecondNode)
	{
		if (!FirstNode.IsValid() || !SecondNode.IsValid())
		{
			return false;
		}
		return *FirstNode == *SecondNode;
	}

private:
	// The search box
	TSharedPtr<SSearchBox> FilterTextBoxWidget;

	// The filter text from the search box.
	FString FilterString;

	// The root data source
	TSharedPtr<FDlgDataDisplayTreeNode> RootTreeItem;

	// The root children. Kept separate so that we do not corrupt the data.
	TArray<TSharedPtr<FDlgDataDisplayTreeNode>> RootChildren;

	// Tree view for showing all Actors that implement the , etc.
	TSharedPtr<STreeView<TSharedPtr<FDlgDataDisplayTreeNode>>> ActorsTreeView;

	// Used for fast lookup of each actor
	// Key: Weak Pointer to the Actor
	// Value: Actor properties
	TMap<TWeakObjectPtr<AActor>, TSharedPtr<FDlgDataDisplayActorProperties>> ActorsProperties;

	// Reference Object used to get the World
	TWeakObjectPtr<const UObject> WorldContextObjectPtr = nullptr;
};
