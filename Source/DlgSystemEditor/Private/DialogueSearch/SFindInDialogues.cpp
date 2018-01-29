// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "SFindInDialogues.h"

#include "STableRow.h"
#include "Editor.h"
#include "EditorStyle.h"
#include "Widgets/Input/SSearchBox.h"
#include "Framework/Commands/GenericCommands.h"

#include "DialogueEditor/DialogueEditor.h"
#include "FindInDialoguesResult.h"
#include "FindInDialogueSearchManager.h"

#define LOCTEXT_NAMESPACE "SFindInDialogues"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SFindInDialogues
void SFindInDialogues::Construct(const FArguments& InArgs, TSharedPtr<FDialogueEditor> InDialogueEditor)
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

			// Change find mode to global
			+SHorizontalBox::Slot()
			.Padding(4.f, 0.f, 2.f, 0.f)
			.AutoWidth()
			[
				SNew(SButton)
				.OnClicked(this, &Self::HandleOpenGlobalFindResults)
				// Show button if we are in a DialogueEditor
				.Visibility(DialogueEditorPtr.IsValid() && bHostFindInDialoguesInGlobalTab ? EVisibility::Visible : EVisibility::Collapsed)
				.ToolTipText(LOCTEXT("OpenInGlobalFindResultsButtonTooltip", "Find in all Blueprints"))
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
				SAssignNew(TreeView, STreeView<FFindInDialoguesResultPtr>)
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

void SFindInDialogues::FocusForUse(const bool bSetFindWithinDialogue, const FString& NewSearchTerms, const bool bSelectFirstResult)
{
	// NOTE: Careful, GeneratePathToWidget can be reentrant in that it can call visibility delegates and such
	FWidgetPath FilterTextBoxWidgetPath;
	FSlateApplication::Get().GeneratePathToWidgetUnchecked(SearchTextBoxWidget.ToSharedRef(), FilterTextBoxWidgetPath);

	// Set keyboard focus directly
	FSlateApplication::Get().SetKeyboardFocus(FilterTextBoxWidgetPath, EFocusCause::SetDirectly);

	// Set the filter mode
	bIsInFindWithinDialogueMode = bSetFindWithinDialogue;

	// Set the new search terms
	if (!NewSearchTerms.IsEmpty())
	{
		SearchTextBoxWidget->SetText(FText::FromString(NewSearchTerms));
		MakeSearchQuery(SearchValue, bIsInFindWithinDialogueMode);

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


void SFindInDialogues::MakeSearchQuery(const FString& InSearchString, const bool bInIsFindWithinDialogue)
{
	SearchTextBoxWidget->SetText(FText::FromString(InSearchString));

	// Reset the scroll to the top
	if (ItemsFound.Num())
	{
		TreeView->RequestScrollIntoView(ItemsFound[0]);
	}
	ItemsFound.Empty();

	// Nothing to search for :(
	if (InSearchString.IsEmpty())
	{
		return;
	}

	HighlightText = FText::FromString(InSearchString);
	RootSearchResult = MakeShareable(new FFindInDialoguesRootNode);

	// TODO use threads extend FRunnable
	if (bInIsFindWithinDialogue)
	{
		// Local
		if (DialogueEditorPtr.IsValid())
		{
			FFindInDialogueSearchManager::Get()
					->QuerySingleDialogue(InSearchString, DialogueEditorPtr.Pin()->GetDialogueBeingEdited(), RootSearchResult);

			// Do now show the Dialogue in the search results.
			const TArray<FFindInDialoguesResultPtr>& Children = RootSearchResult->GetChildren();
			if (Children.Num() == 1 && Children[0].IsValid())
			{
				// Make the root be the first result (aka de dialogue).
				// NOTE: we must keep a reference here otherwise it crashes inside the parent reset
				FFindInDialoguesResultPtr TempChild = Children[0];
				RootSearchResult = TempChild;
				RootSearchResult->ClearParent();
			}
		}
	}
	else
	{
		// Global
		FFindInDialogueSearchManager::Get()->QueryAllDialogues(InSearchString, RootSearchResult);
	}

	ItemsFound = RootSearchResult->GetChildren();
	if (ItemsFound.Num() == 0)
	{
		// Some Items found
		ItemsFound.Add(MakeShareable(new FFindInDialoguesResult(LOCTEXT("DialogueSearchNoResults", "No Results found"), RootSearchResult)));
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
	// TODO call FFindInDialogueSearchManager
	FFindInDialogueSearchManager::Get()->CloseGlobalFindResults(SharedThis(this));
}

void SFindInDialogues::HandleSearchTextChanged(const FText& Text)
{
	SearchValue = Text.ToString();
}

void SFindInDialogues::HandleSearchTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		MakeSearchQuery(SearchValue, bIsInFindWithinDialogueMode);
	}
}

FReply SFindInDialogues::HandleOpenGlobalFindResults()
{
	TSharedPtr<SFindInDialogues> GlobalFindResults = FFindInDialogueSearchManager::Get()->GetGlobalFindResults();
	if (GlobalFindResults.IsValid())
	{
		GlobalFindResults->FocusForUse(false, SearchValue, true);
	}

	return FReply::Handled();
}

void SFindInDialogues::HandleGetChildren(FFindInDialoguesResultPtr InItem, TArray<FFindInDialoguesResultPtr>& OutChildren)
{
	OutChildren += InItem->GetChildren();
}

void SFindInDialogues::HandleTreeSelectionDoubleClicked(FFindInDialoguesResultPtr Item)
{
	if (Item.IsValid())
	{
		Item->OnClick();
	}
}

TSharedRef<ITableRow> SFindInDialogues::HandleGenerateRow(FFindInDialoguesResultPtr InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	// We have categories if we are searching in multiple Dialogues
	// OR the grandparent of this item is not valid (aka root node)
	const bool bIsCategoryWidget = !bIsInFindWithinDialogueMode &&
		(!InItem->GetParent().IsValid() || (InItem->GetParent().IsValid() && InItem->GetParent().Pin()->IsRoot()));

	// Category entry
	if (bIsCategoryWidget)
	{
		return SNew(STableRow<FFindInDialoguesResultPtr>, OwnerTable)
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
						.Text(InItem.Get(), &FFindInDialoguesResult::GetDisplayText)
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

	return SNew(STableRow<FFindInDialoguesResultPtr>, OwnerTable)
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
				.Text(InItem.Get(), &FFindInDialoguesResult::GetDisplayText)
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
				.HighlightText(HighlightText)
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

#undef LOCTEXT_NAMESPACE
