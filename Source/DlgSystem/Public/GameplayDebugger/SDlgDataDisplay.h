// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SCompoundWidget.h"
#include "STreeView.h"
#include "Widgets/Input/SSearchBox.h"

#include "DlgDataDisplayTreeNode.h"
#include "DlgDataDisplayActorProperties.h"


/**
 * Implements the Runtime Dialogue Data Display
 */
class SDlgDataDisplay : public SCompoundWidget
{
	typedef SDlgDataDisplay Self;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TWeakObjectPtr<AActor> InReferenceActor = nullptr);

	/** Updates the actors tree. */
	void RefreshTree(bool bPreserveExpansion);

	/** Get current filter text */
	FText GetFilterText() const { return FilterTextBoxWidget->GetText(); }

private:
	/** Getters for widgets.  */
	TSharedRef<SWidget> GetFilterTextBoxWidget();

	/** Recursively build the view item. */
	void BuildTreeViewItem(FDlgDataDisplayTreeNodePtr Item);

	/** Text search changed */
	void HandleSearchTextCommited(const FText& InText, ETextCommit::Type InCommitType);

	/** Refresh button clicked. */
	FReply HandleOnRefresh()
	{
		RefreshTree(true);
		return FReply::Handled();
	}

	/** Make the row */
	TSharedRef<ITableRow> HandleGenerateRow(FDlgDataDisplayTreeNodePtr InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/** General Get children  */
	void HandleGetChildren(FDlgDataDisplayTreeNodePtr InItem, TArray<FDlgDataDisplayTreeNodePtr>& OutChildren);

	/** Handles changes in the Tree View. */
	void HandleTreeSelectionChanged(FDlgDataDisplayTreeNodePtr InItem, ESelectInfo::Type SelectInfo);

	/** User clicked on item. */
	void HandleDoubleClick(FDlgDataDisplayTreeNodePtr InItem);

	/** Callback for expanding tree items recursively */
	void HandleSetExpansionRecursive(FDlgDataDisplayTreeNodePtr InItem, bool bInIsItemExpanded);

private:
	/** The search box */
	TSharedPtr<SSearchBox> FilterTextBoxWidget;

	/** The filter text from the search box. */
	FString FilterString;

	/** The root data source */
	FDlgDataDisplayTreeNodePtr RootTreeItem;

	/** The root children. Kept seperate so that we do not corrupt the data. */
	TArray<FDlgDataDisplayTreeNodePtr> RootChildren;

	/** Tree view for showing all Actors that implement the , etc. */
	TSharedPtr<STreeView<FDlgDataDisplayTreeNodePtr>> ActorsTreeView;

	/**
	 * Used for fast lookup of each actor
	 * Key: Weak Pointer to the Actor
	 * Value: Actor properties
	 */
	TMap<TWeakObjectPtr<const AActor>, TSharedPtr<FDlgDataDisplayActorProperties>> ActorsProperties;

	/** Reference Actor used to get the UWorld. */
	TWeakObjectPtr<const AActor> ReferenceActor = nullptr;
};
