// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "Framework/Commands/UICommandList.h"

#include "DlgSearchResult.h"

class FDlgEditor;
class SSearchBox;
class SDockTab;

/**  Widget for searching across all dialogues or just a single dialogue */
class DLGSYSTEMEDITOR_API SDlgFindInDialogues : public SCompoundWidget
{
private:
	typedef SDlgFindInDialogues Self;

public:
	SLATE_BEGIN_ARGS(Self)
		: _bIsSearchWindow(true)
		, _bHideSearchBar(false)
		, _ContainingTab()
	{}
		SLATE_ARGUMENT(bool, bIsSearchWindow)
		SLATE_ARGUMENT(bool, bHideSearchBar)
		SLATE_ARGUMENT(TSharedPtr<SDockTab>, ContainingTab)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FDlgEditor>& InDialogueEditor = nullptr);
	~SDlgFindInDialogues();

	/** Focuses this widget's search box, and changes the mode as well, and optionally the search terms */
	void FocusForUse(bool bSetFindWithinDialogue, const FDlgSearchFilter& SearchFilter = FDlgSearchFilter(), bool bSelectFirstResult = false);

	/**
	 * Submits a search query
	 *
	 * @param SearchFilter						Filter for search
	 * @param bInIsFindWithinDialogue			TRUE if searching within the current Dialogue only
	 */
	void MakeSearchQuery(const FDlgSearchFilter& SearchFilter, bool bInIsFindWithinDialogue);

	/** If this is a global find results widget, returns the host tab's unique ID. Otherwise, returns NAME_None. */
	FName GetHostTabId() const;

	/** If this is a global find results widget, ask the host tab to close */
	void CloseHostTab();

private:
	/** Called when the host tab is closed (if valid) */
	void HandleHostTabClosed(TSharedRef<SDockTab> DockTab);

	/** Called when user changes the text they are searching for */
	void HandleSearchTextChanged(const FText& Text);

	/** Called when user changes commits text to the search box */
	void HandleSearchTextCommitted(const FText& Text, ETextCommit::Type CommitType);

	/** Called when the user clicks the global find results button */
	FReply HandleOpenGlobalFindResults();

	/** Called when the find mode checkbox is hit */
	void HandleFindModeChanged(ECheckBoxState CheckState)
	{
		bIsInFindWithinDialogueMode = CheckState == ECheckBoxState::Checked;
	}

	/** Called to check what the find mode is for the checkbox */
	ECheckBoxState HandleGetFindModeChecked() const
	{
		return bIsInFindWithinDialogueMode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	/* Get the children of a row */
	void HandleGetChildren(TSharedPtr<FDlgSearchResult> InItem, TArray<TSharedPtr<FDlgSearchResult>>& OutChildren);

	/* Called when user double clicks on a new result */
	void HandleTreeSelectionDoubleClicked(TSharedPtr<FDlgSearchResult> Item);

	/* Called when a new row is being generated */
	TSharedRef<ITableRow> HandleGenerateRow(TSharedPtr<FDlgSearchResult> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/** Callback to build the context menu when right clicking in the tree */
	TSharedPtr<SWidget> HandleContextMenuOpening();

	/** Fills in the filter menu. */
	TSharedRef<SWidget> FillFilterEntries();

private:
	/** Pointer back to the Dialogue editor that owns us */
	TWeakPtr<FDlgEditor> DialogueEditorPtr;

	/* The tree view displays the results */
	TSharedPtr<STreeView<TSharedPtr<FDlgSearchResult>>> TreeView;

	/** The search text box */
	TSharedPtr<SSearchBox> SearchTextBoxWidget;

	/** Vertical box, used to add and remove widgets dynamically */
	TWeakPtr<SVerticalBox> MainVerticalBoxWidget;

	/** In Find Within Dialogue mode, we need to keep a handle on the root result, because it won't show up in the tree. */
	TSharedPtr<FDlgSearchResult> RootSearchResult;

	/* This buffer stores the currently displayed results */
	TArray<TSharedPtr<FDlgSearchResult>> ItemsFound;

	/* The string to highlight in the results */
	FText HighlightText;

	/** The current searach filter */
	FDlgSearchFilter CurrentFilter;

	/** Should we search within the current Dialogue only (rather than all Dialogues) */
	bool bIsInFindWithinDialogueMode;

	/** Tab hosting this widget. May be invalid. */
	TWeakPtr<SDockTab> HostTab;

	/** Commands handled by this widget */
	TSharedPtr<FUICommandList> CommandList;
};
