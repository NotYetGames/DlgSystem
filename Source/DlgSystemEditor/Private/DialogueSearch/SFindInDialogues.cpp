// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SFindInDialogues.h"

#include "Editor.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/GenericCommands.h"

#include "DialogueEditor/DialogueEditor.h"
#include "DialogueSearchResult.h"
#include "DialogueSearchManager.h"


#define LOCTEXT_NAMESPACE "SFindInDialogues"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SFindInDialogues
void SFindInDialogues::Construct(const FArguments& InArgs, const TSharedPtr<FDialogueEditor>& InDialogueEditor)
{
	DialogueEditorPtr = InDialogueEditor;
	HostTab = InArgs._ContainingTab;

	if (HostTab.IsValid())
	{
		HostTab.Pin()->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateSP(this, &Self::HandleHostTabClosed));
	}

	if (InArgs._bIsSearchWindow)
	{
		//RegisterCommands();
	}

	// Only search in the current Dialogue
	bIsInFindWithinDialogueMode = DialogueEditorPtr.IsValid();

	constexpr bool bHostFindInDialoguesInGlobalTab = true;
	ChildSlot
	[
		SAssignNew(MainVerticalBoxWidget, SVerticalBox)

		// Top bar, search
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			// Search field
			+SHorizontalBox::Slot()
			.FillWidth(1)
			[
				SAssignNew(SearchTextBoxWidget, SSearchBox)
				.HintText(LOCTEXT("DialogueSearchHint", "Enter function or event name to find references..."))
				.OnTextChanged(this, &Self::HandleSearchTextChanged)
				.OnTextCommitted(this, &Self::HandleSearchTextCommitted)
				.Visibility(InArgs._bHideSearchBar? EVisibility::Collapsed : EVisibility::Visible)
			]

			// Filter Options
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 2.0f)
			[
				SNew(SComboButton)
				.ComboButtonStyle(FEditorStyle::Get(), "GenericFilters.ComboButtonStyle")
				.ForegroundColor(FLinearColor::White)
				.ContentPadding(0)
				.ToolTipText(LOCTEXT("Filters_Tooltip", "Filter options for the Dialogue Search."))
				.OnGetMenuContent(this, &Self::FillFilterEntries)
				.HasDownArrow(true)
				.ContentPadding(FMargin(1, 0))
				.ButtonContent()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
						.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.9"))
						.Text(FText::FromString(FString(TEXT("\xf0b0"))) /*fa-filter*/)
					]
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0, 0, 0)
					[
						SNew(STextBlock)
						.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
						.Text(LOCTEXT("Filters", "Filters"))
					]
				]
			]

			// Change find mode to global
			+SHorizontalBox::Slot()
			.Padding(4.f, 0.f, 2.f, 0.f)
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked(this, &Self::HandleOpenGlobalFindResults)
				// Show button if we are in a DialogueEditor
				.Visibility(DialogueEditorPtr.IsValid() && bHostFindInDialoguesInGlobalTab ? EVisibility::Visible : EVisibility::Collapsed)
				.ToolTipText(LOCTEXT("OpenInGlobalFindResultsButtonTooltip", "Find in all Dialogues"))
				[
					SNew(STextBlock)
					.TextStyle(FEditorStyle::Get(), "FindResults.FindInBlueprints")
					.Text(FText::FromString(FString(TEXT("\xf1e5"))) /*fa-binoculars*/)
				]
			]

			// Change find mode to local
			+SHorizontalBox::Slot()
			.Padding(2.f, 0.f)
			.AutoWidth()
			[
				SNew(SCheckBox)
				.OnCheckStateChanged(this, &Self::HandleFindModeChanged)
				.IsChecked(this, &Self::HandleGetFindModeChecked)
				.Visibility(InArgs._bHideSearchBar || bHostFindInDialoguesInGlobalTab ? EVisibility::Collapsed : EVisibility::Visible)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BlueprintSearchModeChange", "Find In Current Blueprint Only"))
				]
			]
		]

		// Results tree
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(0.f, 4.f, 0.f, 0.f)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				SAssignNew(TreeView, STreeView<TSharedPtr<FDialogueSearchResult>>)
				.ItemHeight(24)
				.TreeItemsSource(&ItemsFound)
				.OnGenerateRow(this, &Self::HandleGenerateRow)
				.OnGetChildren(this, &Self::HandleGetChildren)
				.OnMouseButtonDoubleClick(this, &Self::HandleTreeSelectionDoubleClicked)
				.SelectionMode(ESelectionMode::Multi)
				.OnContextMenuOpening(this, &Self::HandleContextMenuOpening)
			]
		]
	];
}

SFindInDialogues::~SFindInDialogues()
{
	// TODO clear all cache for this
}

void SFindInDialogues::FocusForUse(bool bSetFindWithinDialogue, const FDialogueSearchFilter& SearchFilter, bool bSelectFirstResult)
{
	// NOTE: Careful, GeneratePathToWidget can be reentrant in that it can call visibility delegates and such
	FWidgetPath FilterTextBoxWidgetPath;
	FSlateApplication::Get().GeneratePathToWidgetUnchecked(SearchTextBoxWidget.ToSharedRef(), FilterTextBoxWidgetPath);

	// Set keyboard focus directly
	FSlateApplication::Get().SetKeyboardFocus(FilterTextBoxWidgetPath, EFocusCause::SetDirectly);

	// Set the filter mode
	bIsInFindWithinDialogueMode = bSetFindWithinDialogue;

	// Set the new search terms
	if (!SearchFilter.SearchString.IsEmpty())
	{
		SearchTextBoxWidget->SetText(FText::FromString(SearchFilter.SearchString));
		MakeSearchQuery(SearchFilter, bIsInFindWithinDialogueMode);

		// Select the first result
		if (bSelectFirstResult && ItemsFound.Num())
		{
			auto ItemToFocusOn = ItemsFound[0];

			// Focus the deepest child
			while (ItemToFocusOn->HasChildren())
			{
				ItemToFocusOn = ItemToFocusOn->GetChildren()[0];
			}
			TreeView->SetSelection(ItemToFocusOn);
			ItemToFocusOn->OnClick();
		}
	}
}

void SFindInDialogues::MakeSearchQuery(const FDialogueSearchFilter& SearchFilter, bool bInIsFindWithinDialogue)
{
	SearchTextBoxWidget->SetText(FText::FromString(SearchFilter.SearchString));

	// Reset the scroll to the top
	if (ItemsFound.Num())
	{
		TreeView->RequestScrollIntoView(ItemsFound[0]);
	}
	ItemsFound.Empty();

	// Nothing to search for :(
	if (SearchFilter.SearchString.IsEmpty())
	{
		return;
	}

	HighlightText = FText::FromString(SearchFilter.SearchString);
	RootSearchResult = MakeShared<FDialogueSearchResult_RootNode>();

	// TODO use threads extend FRunnable
	if (bInIsFindWithinDialogue)
	{
		// Local
		if (DialogueEditorPtr.IsValid())
		{
			FDialogueSearchManager::Get()
					->QuerySingleDialogue(SearchFilter, DialogueEditorPtr.Pin()->GetDialogueBeingEdited(), RootSearchResult);

			// Do now show the Dialogue in the search results.
			const TArray<TSharedPtr<FDialogueSearchResult>>& Children = RootSearchResult->GetChildren();
			if (Children.Num() == 1 && Children[0].IsValid())
			{
				// Make the root be the first result (aka de dialogue).
				// NOTE: we must keep a reference here otherwise it crashes inside the parent reset
				TSharedPtr<FDialogueSearchResult> TempChild = Children[0];
				RootSearchResult = TempChild;
				RootSearchResult->ClearParent();
			}
		}
	}
	else
	{
		// Global
		FDialogueSearchManager::Get()->QueryAllDialogues(SearchFilter, RootSearchResult);
	}

	ItemsFound = RootSearchResult->GetChildren();
	if (ItemsFound.Num() == 0)
	{
		// Some Items found
		ItemsFound.Add(MakeShared<FDialogueSearchResult>(LOCTEXT("DialogueSearchNoResults", "No Results found"), RootSearchResult));
		HighlightText = FText::GetEmpty();

	}
	else
	{
		// No Items found
		RootSearchResult->ExpandAllChildren(TreeView);
	}

	TreeView->RequestTreeRefresh();
}

FName SFindInDialogues::GetHostTabId() const
{
	TSharedPtr<SDockTab> HostTabPtr = HostTab.Pin();
	if (HostTabPtr.IsValid())
	{
		return HostTabPtr->GetLayoutIdentifier().TabType;
	}

	return NAME_None;
}

void SFindInDialogues::CloseHostTab()
{
	TSharedPtr<SDockTab> HostTabPtr = HostTab.Pin();
	if (HostTabPtr.IsValid())
	{
		HostTabPtr->RequestCloseTab();
	}
}

void SFindInDialogues::HandleHostTabClosed(TSharedRef<SDockTab> DockTab)
{
	FDialogueSearchManager::Get()->CloseGlobalFindResults(SharedThis(this));
}

void SFindInDialogues::HandleSearchTextChanged(const FText& Text)
{
	CurrentFilter.SearchString = Text.ToString();
}

void SFindInDialogues::HandleSearchTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		CurrentFilter.SearchString = Text.ToString();
		MakeSearchQuery(CurrentFilter, bIsInFindWithinDialogueMode);
	}
}

FReply SFindInDialogues::HandleOpenGlobalFindResults()
{
	TSharedPtr<SFindInDialogues> GlobalFindResults = FDialogueSearchManager::Get()->GetGlobalFindResults();
	if (GlobalFindResults.IsValid())
	{
		GlobalFindResults->FocusForUse(false, CurrentFilter, true);
	}

	return FReply::Handled();
}

void SFindInDialogues::HandleGetChildren(TSharedPtr<FDialogueSearchResult> InItem, TArray<TSharedPtr<FDialogueSearchResult>>& OutChildren)
{
	OutChildren += InItem->GetChildren();
}

void SFindInDialogues::HandleTreeSelectionDoubleClicked(TSharedPtr<FDialogueSearchResult> Item)
{
	if (Item.IsValid())
	{
		Item->OnClick();
	}
}

TSharedRef<ITableRow> SFindInDialogues::HandleGenerateRow(TSharedPtr<FDialogueSearchResult> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	// We have categories if we are searching in multiple Dialogues
	// OR the grandparent of this item is not valid (aka root node)
	const bool bIsCategoryWidget = !bIsInFindWithinDialogueMode &&
		(!InItem->GetParent().IsValid() || (InItem->GetParent().IsValid() && InItem->GetParent().Pin()->IsRoot()));

	// Category entry
	if (bIsCategoryWidget)
	{
		return SNew(STableRow<TSharedPtr<FDialogueSearchResult>>, OwnerTable)
			[
				SNew(SBorder)
				.VAlign(VAlign_Center)
				.BorderImage(FEditorStyle::GetBrush("PropertyWindow.CategoryBackground"))
				.Padding(FMargin(2.0f))
				.ForegroundColor(FEditorStyle::GetColor("PropertyWindow.CategoryForeground"))
				[
					SNew(SHorizontalBox)

					// Icon
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						InItem->CreateIcon()
					]

					// Display text
					+SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(2, 0)
					[
						SNew(STextBlock)
						.Text(InItem.Get(), &FDialogueSearchResult::GetDisplayText)
						.ToolTipText(LOCTEXT("DialogueCatSearchToolTip", "Dialogue"))
					]
				]
			];
	}

	// Normal entry
	FText CommentText = FText::GetEmpty();
	if (!InItem->GetCommentString().IsEmpty())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Comment"), FText::FromString(InItem->GetCommentString()));
		CommentText = FText::Format(LOCTEXT("NodeComment", "{Comment}"), Args);
	}

	FFormatNamedArguments Args;
	Args.Add(TEXT("Category"), InItem->GetCategory());
	Args.Add(TEXT("DisplayTitle"), InItem->GetDisplayText());
	FText Tooltip = FText::Format(LOCTEXT("DialogueResultSearchToolTip", "{Category} : {DisplayTitle}"), Args);

	return SNew(STableRow<TSharedPtr<FDialogueSearchResult>>, OwnerTable)
		[
			SNew(SHorizontalBox)

			// Icon
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				InItem->CreateIcon()
			]

			// Display text
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2,0)
			[
				SNew(STextBlock)
				.Text(InItem.Get(), &FDialogueSearchResult::GetDisplayText)
				.HighlightText(HighlightText)
				.ToolTipText(Tooltip)
			]

			// Comment Block
			+SHorizontalBox::Slot()
			.FillWidth(1)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Padding(2,0)
			[
				SNew(STextBlock)
				.Text(CommentText)
				.ColorAndOpacity(FLinearColor::Yellow)
			]
		];
}

TSharedPtr<SWidget> SFindInDialogues::HandleContextMenuOpening()
{
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, CommandList);

	MenuBuilder.BeginSection("BasicOperations");
	{
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().SelectAll);
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
	}

	return MenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SFindInDialogues::FillFilterEntries()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("IncludeIndices", "Include Indices"),
		LOCTEXT("IncludeIndices_ToolTip", "Include indices of nodes/edges in the search result"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
			{
				CurrentFilter.bIncludeIndices = !CurrentFilter.bIncludeIndices;
				MakeSearchQuery(CurrentFilter, bIsInFindWithinDialogueMode);
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this]() -> bool
			{
				return CurrentFilter.bIncludeIndices;
			})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("IncludeDialogueGUID", "Include Dialogue GUIDs"),
		LOCTEXT("IncludeDialogueGUID_ToolTip", "Include Dialogue GUIDs in search in the search result"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
			{
				CurrentFilter.bIncludeDialogueGUID = !CurrentFilter.bIncludeDialogueGUID;
				MakeSearchQuery(CurrentFilter, bIsInFindWithinDialogueMode);
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this]() -> bool
			{
				return CurrentFilter.bIncludeDialogueGUID;
			})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("IncludeNodeGUID", "Include Node GUIDs"),
		LOCTEXT("IncludeNodeGUID_ToolTip", "Include Node GUIDs in search in the search result"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
			{
				CurrentFilter.bIncludeNodeGUID = !CurrentFilter.bIncludeNodeGUID;
				MakeSearchQuery(CurrentFilter, bIsInFindWithinDialogueMode);
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this]() -> bool
			{
				return CurrentFilter.bIncludeNodeGUID;
			})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("IncludeComments", "Include Comments"),
		LOCTEXT("IncludeComments_ToolTip", "Include Comments from nodes and comments on nodes in the search result"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
			{
				CurrentFilter.bIncludeComments = !CurrentFilter.bIncludeComments;
				MakeSearchQuery(CurrentFilter, bIsInFindWithinDialogueMode);
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this]() -> bool
			{
				return CurrentFilter.bIncludeComments;
			})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("IncludeNumericalTypes", "Include Integers & Floats"),
		LOCTEXT("IncludeNumericalTypes_ToolTip", "Include int32/floats in search result"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
			{
				CurrentFilter.bIncludeNumericalTypes = !CurrentFilter.bIncludeNumericalTypes;
				MakeSearchQuery(CurrentFilter, bIsInFindWithinDialogueMode);
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this]() -> bool
			{
				return CurrentFilter.bIncludeNumericalTypes;
			})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("IncludeTextLocalizationData", "Include Text Localization Data"),
		LOCTEXT("IncludeTextLocalizationData_ToolTip", "Include text localization data in search (key/namespace)"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
			{
				CurrentFilter.bIncludeTextLocalizationData = !CurrentFilter.bIncludeTextLocalizationData;
				MakeSearchQuery(CurrentFilter, bIsInFindWithinDialogueMode);
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this]() -> bool
			{
				return CurrentFilter.bIncludeTextLocalizationData;
			})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
	MenuBuilder.AddMenuEntry(
		LOCTEXT("IncludeCustomObjectNames", "Include Custom Object Names"),
		LOCTEXT("IncludeCustomObjectNames_ToolTip", "Include the Custom Text Argument/Condition/Event/Node Data object names"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([this]()
			{
				CurrentFilter.bIncludeCustomObjectNames = !CurrentFilter.bIncludeCustomObjectNames;
				MakeSearchQuery(CurrentFilter, bIsInFindWithinDialogueMode);
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([this]() -> bool
			{
				return CurrentFilter.bIncludeCustomObjectNames;
			})
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);

	return MenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
