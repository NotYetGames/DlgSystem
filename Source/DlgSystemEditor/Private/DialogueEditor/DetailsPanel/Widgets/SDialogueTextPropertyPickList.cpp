// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDialogueTextPropertyPickList.h"

#include "PropertyHandle.h"
#include "Widgets/Input/SSearchBox.h"
#include "Framework/Application/SlateApplication.h"
#include "DetailWidgetRow.h"
#include "IDocumentation.h"
#include "Layout/WidgetPath.h"

#define LOCTEXT_NAMESPACE "SDialogueTextPropertyPickList"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SDialogueTextPropertyPickList
void SDialogueTextPropertyPickList::Construct(const FArguments& InArgs)
{
	// MUST call SetPropertyHandle later or a check will fail.
	if (InArgs._PropertyHandle.IsValid())
	{
		SetPropertyHandle(InArgs._PropertyHandle);
	}
	SetToolTipAttribute(InArgs._ToolTipText);

	// Context checkbox arguments
	bHasContextCheckBox = InArgs._HasContextCheckbox;
	bIsContextCheckBoxChecked = bHasContextCheckBox ? InArgs._IsContextCheckBoxChecked : false;
	CurrentContextSuggestionAttributes = InArgs._CurrentContextAvailableSuggestions;
	ContextCheckBoxTextAttribute = InArgs._ContextCheckBoxText;
	ContextCheckBoxToolTipTextAttribute = InArgs._ContextCheckBoxToolTipText;

	HintTextAttribute = InArgs._HintText;
	SuggestionAttributes = InArgs._AvailableSuggestions;
	OnTextChanged = InArgs._OnTextChanged;
	OnTextCommitted = InArgs._OnTextCommitted;
	OnKeyDownHandler = InArgs._OnKeyDownHandler;
	bDelayChangeNotificationsWhileTyping = InArgs._DelayChangeNotificationsWhileTyping;

	// Assign the main horizontal box of this widget
	TSharedPtr<SHorizontalBox> ContentBox = SNew(SHorizontalBox);
	ChildSlot
		[
			ContentBox.ToSharedRef()
		];

	// right of the combo box
//	const TSharedRef<SHorizontalBox> ButtonBox = SNew(SHorizontalBox);
//	TSharedRef<SWidget> ButtonBoxWrapper =
//		SNew(SBox)
//		.Padding(FMargin(1.f, 0.f))
//		[
//			ButtonBox
//		];

	// TODO maybe have a look at SNameComboBox and SComboBox
	// Build the button and text view
	ComboButtonWidget = SNew(SComboButton)
		.ButtonStyle(FEditorStyle::Get(), "PropertyEditor.AssetComboStyle")
		.ForegroundColor(FEditorStyle::GetColor("PropertyEditor.AssetName.ColorAndOpacity"))
		.OnGetMenuContent(this, &Self::GetMenuWidget)
		.OnMenuOpenChanged(this, &Self::HandleMenuOpenChanged)
		.OnComboBoxOpened(this, &Self::HandleComboBoxOpened)
		.IsEnabled(true)
		.IsFocusable(false) // if set to true, it won't select the search input box
		.ContentPadding(2.0f)
		.ButtonContent()
		[
			// Show the name of the asset or actor
			SAssignNew(ComboButtonTextWidget, STextBlock)
			.TextStyle(FEditorStyle::Get(), "PropertyEditor.AssetClass")
			.Text(TextAttribute)
		];

	ContentBox->AddSlot()
		[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				[
					ComboButtonWidget.ToSharedRef()
				]

	//				+SHorizontalBox::Slot()
	//				.AutoWidth()
	//				[
	//					ButtonBoxWrapper
	//				]
		];

//	ButtonBoxWrapper->SetVisibility(ButtonBox->NumSlots() > 0 ? EVisibility::Visible : EVisibility::Collapsed);
}

FReply SDialogueTextPropertyPickList::OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		// Clear any selection first to prevent the currently selection being set in the text box
		ListViewWidget->ClearSelection();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SDialogueTextPropertyPickList::SetText(const TAttribute<FText>& InNewText)
{
	const FText NewText = InNewText.Get();
	TextAttribute.Set(NewText);

	PropertyHandle->SetValueFromFormattedString(*NewText.ToString());
	if (ComboButtonTextWidget.IsValid())
	{
		ComboButtonTextWidget->SetText(TextAttribute);
	}
	if (InputTextWidget.IsValid())
	{
		InputTextWidget->SetText(TextAttribute);
	}
}

void SDialogueTextPropertyPickList::SetToolTipAttribute(const TAttribute<FText>& InNewText)
{
	const FText NewText = InNewText.Get();
	ToolTipAttribute.Set(NewText);
	SetToolTipText(ToolTipAttribute);
}

void SDialogueTextPropertyPickList::SetPropertyHandle(const TSharedPtr<IPropertyHandle>& InPropertyHandle)
{
	PropertyHandle = InPropertyHandle;
	check(PropertyHandle.IsValid());

	// Read the initial value of the text this widget belongs to
	FText ReadData;
	if (PropertyHandle->GetValueAsFormattedText(ReadData) == FPropertyAccess::Success)
	{
		SetText(ReadData);
	}
}

TSharedRef<SWidget> SDialogueTextPropertyPickList::GetMenuWidget()
{
	// Is it cached?
	if (MenuWidget.IsValid())
	{
		return SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.f, 2.f, 2.f, 5.f)
			[
				MenuWidget.ToSharedRef()
			];
	}

	// Cache it
	MenuWidget = SNew(SVerticalBox);

	// Context Sensitive widget
	if (bHasContextCheckBox)
	{
		MenuWidget->AddSlot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 1.f)
			[
				GetContextCheckBoxWidget()
			];
	}

	// Search Box
	MenuWidget->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 1.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SearchHeader", "Search or Set new text (press enter)"))
			.TextStyle(FCoreStyle::Get(), TEXT("Menu.Heading"))
		];

	MenuWidget->AddSlot()
		.AutoHeight()
		.Padding(0, 0, 0, 1)
		[
			GetSearchBoxWidget()
		];

	// List view widget
	MenuWidget->AddSlot()
		.FillHeight(1.f)
		.Padding(1.f, 2.f)
		[
			GetListViewWidget()
		];

	// Should return a valid widget
	return GetMenuWidget();
}

TSharedRef<SWidget> SDialogueTextPropertyPickList::GetContextCheckBoxWidget()
{
	// It is cached?
	if (ContextCheckBoxWidget.IsValid())
	{
		return SNew(SHorizontalBox)

			// Context Toggle
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			[
				ContextCheckBoxWidget.ToSharedRef()
			];
	}

	// Cache it
	ContextCheckBoxWidget = SNew(SCheckBox)
		.OnCheckStateChanged(this, &Self::HandleContextCheckboxChanged)
		.IsFocusable(false)
		.IsChecked(this, &Self::IsContextCheckBoxChecked)
		.ToolTipText(ContextCheckBoxToolTipTextAttribute)
		[
			SNew(STextBlock)
			.Text(ContextCheckBoxTextAttribute)
		];

	// Should return a valid widget
	return GetContextCheckBoxWidget();
}

TSharedRef<SWidget> SDialogueTextPropertyPickList::GetSearchBoxWidget()
{
	// Is it cached?
	if (InputTextWidget.IsValid())
	{
		return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				InputTextWidget.ToSharedRef()
			];
	}

	// Cache it
	InputTextWidget = SNew(SSearchBox)
		.InitialText(TextAttribute)
		.HintText(HintTextAttribute)
		.OnTextChanged(this, &Self::HandleTextChanged)
		.OnTextCommitted(this, &Self::HandleTextCommitted)
		.SelectAllTextWhenFocused(true)
		.DelayChangeNotificationsWhileTyping(bDelayChangeNotificationsWhileTyping)
		.OnKeyDownHandler(this, &Self::HandleKeyDown);

	// Should return a valid widget
	return GetSearchBoxWidget();
}

TSharedRef<SWidget> SDialogueTextPropertyPickList::GetListViewWidget()
{
	// Is it cached?
	if (ListViewContainerWidget.IsValid())
	{
		return SNew(SOverlay)
			+SOverlay::Slot()
			[
				ListViewContainerWidget.ToSharedRef()
			];
	}

	// Cache it
	ListViewContainerWidget = SNew(SBorder)
		.Padding(0)
		.BorderImage(FEditorStyle::GetBrush("NoBorder"));

	ListViewWidget = SNew(SListView<TextListItem>)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&Suggestions)
		.OnGenerateRow(this, &Self::HandleListGenerateRow)
		.OnSelectionChanged(this, &Self::HandleListSelectionChanged)
		.ItemHeight(20);

	ListViewContainerWidget->SetContent(CreateShadowOverlay(ListViewWidget.ToSharedRef()));

	// Should return a valid widget
	return GetListViewWidget();
}

TSharedRef<ITableRow> SDialogueTextPropertyPickList::HandleListGenerateRow(TextListItem Text, const TSharedRef<STableViewBase>& OwnerTable)
{
	check(Text.IsValid());
	return SNew(STableRow<TextListItem>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromName(*Text.Get()))
			.HighlightText(this, &Self::GetHighlightText)
		];
}

void SDialogueTextPropertyPickList::HandleMenuOpenChanged(bool bOpen)
{
	if (!bOpen)
	{
		FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::SetDirectly);
		ComboButtonWidget->SetMenuContent(SNullWidget::NullWidget);
	}
}

void SDialogueTextPropertyPickList::HandleComboBoxOpened()
{
	GetSearchBoxWidget();
	FocusSearchBox();
	UpdateSuggestionList();
}

/** Handler for when text in the editable text box changed */
void SDialogueTextPropertyPickList::HandleTextChanged(const FText& InSearchText)
{
	UpdateSuggestionList();

	// Check if in suggestion list
	OnTextChanged.ExecuteIfBound(InSearchText);
}

/** Handler for when text in the editable text box changed */
void SDialogueTextPropertyPickList::HandleTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
{
	// Ignore default
	if (CommitType == ETextCommit::Default)
	{
		return;
	}

	TSharedPtr<FName> SelectedSuggestion = GetSelectedSuggestion();
	FText CommittedText;
	if (SelectedSuggestion.IsValid() && CommitType != ETextCommit::OnCleared)
	{
		// Pressed selected a suggestion, set the text
		CommittedText = FText::FromName(*SelectedSuggestion.Get());
	}
	else
	{
		if (CommitType == ETextCommit::OnCleared)
		{
			// Clear text when escape is pressed then commit an empty string
			CommittedText = FText::GetEmpty();
		}
		else
		{
			// otherwise, set the typed text
			CommittedText = NewText;
		}
	}

	// Use "None" as empty string
	if (CommittedText.IdenticalTo(FText::GetEmpty()))
	{
		CommittedText = FText::FromName(NAME_None);
	}

	// Update the displayed text
	SetText(CommittedText);
	OnTextCommitted.ExecuteIfBound(CommittedText, CommitType);

	// Only close the menu when the user did not loose focus
	if (CommitType != ETextCommit::OnUserMovedFocus)
	{
		ComboButtonWidget->SetIsOpen(false);
	}
}

FReply SDialogueTextPropertyPickList::HandleKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Up || InKeyEvent.GetKey() == EKeys::Down)
	{
		const bool bSelectingUp = InKeyEvent.GetKey() == EKeys::Up;
		TSharedPtr<FName> SelectedSuggestion = GetSelectedSuggestion();

		if (SelectedSuggestion.IsValid())
		{
			// Find the selection index and select the previous or next one
			int32 TargetIdx = INDEX_NONE;
			for (int32 SuggestionIdx = 0; SuggestionIdx < Suggestions.Num(); ++SuggestionIdx)
			{
				if (Suggestions[SuggestionIdx] == SelectedSuggestion)
				{
					if (bSelectingUp)
					{
						TargetIdx = SuggestionIdx - 1;
					}
					else
					{
						TargetIdx = SuggestionIdx + 1;
					}

					break;
				}
			}

			if (Suggestions.IsValidIndex(TargetIdx))
			{
				ListViewWidget->SetSelection(Suggestions[TargetIdx]);
				ListViewWidget->RequestScrollIntoView(Suggestions[TargetIdx]);
			}
		}
		else if (!bSelectingUp && Suggestions.Num() > 0)
		{
			// Nothing selected and pressed down, select the first item
			ListViewWidget->SetSelection(Suggestions[0]);
		}

		return FReply::Handled();
	}

	if (OnKeyDownHandler.IsBound())
	{
		return OnKeyDownHandler.Execute(MyGeometry, InKeyEvent);
	}

	return FReply::Unhandled();
}

void SDialogueTextPropertyPickList::HandleListSelectionChanged(TextListItem NewValue, ESelectInfo::Type SelectInfo)
{
	// If the user selected it via click or keyboard then select it, then accept the choice and close the window
	if (SelectInfo == ESelectInfo::OnMouseClick || SelectInfo == ESelectInfo::OnKeyPress || SelectInfo == ESelectInfo::Direct )
	{
		if (NewValue.IsValid())
		{
			SetText(FText::FromName(*NewValue.Get()));
			FocusSearchBox();
		}
		else
		{
			// Can happen in the case selecting the option directly (SelectInfo == ESelectInfo::Direct)
			// HandleTextCommitted will be called automatically because it looses focus, but we want
			// to close the menu explicitly
			ComboButtonWidget->SetIsOpen(false);
		}
	}
}

void SDialogueTextPropertyPickList::HandleContextCheckboxChanged(ECheckBoxState CheckState)
{
	bIsContextCheckBoxChecked = CheckState == ECheckBoxState::Checked;
	UpdateSuggestionList();
	// Return focus to search input so it is easier to navigate down.
	FocusSearchBox();
}

void SDialogueTextPropertyPickList::UpdateSuggestionList()
{
	const FString TypedText = InputTextWidget.IsValid() ? InputTextWidget->GetText().ToString() : TEXT("");
	Suggestions.Empty();

	// Find out what pool of suggestions ot use
	TArray<FName> AllSuggestions;
	if (bHasContextCheckBox && bIsContextCheckBoxChecked)
	{
		// has checkbox and it is true
		AllSuggestions = CurrentContextSuggestionAttributes.Get();
	}
	else
	{
		// default
		AllSuggestions = SuggestionAttributes.Get();
	}

	// Must have typed something, but that something must be different than the set value
	if (TypedText.Len() > 0 && TypedText != TextAttribute.Get().ToString())
	{
		// Match typed text
		for (const FName& Suggestion : AllSuggestions)
		{
			if (Suggestion.ToString().Contains(TypedText))
			{
				Suggestions.Add(MakeShared<FName>(Suggestion));
			}
		}
	}
	else
	{
		// Copy all
		for (const FName& Suggestion : AllSuggestions)
		{
			Suggestions.Add(MakeShared<FName>(Suggestion));
		}
	}

	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
}

void SDialogueTextPropertyPickList::FocusSearchBox() const
{
	FWidgetPath WidgetToFocusPath;
	FSlateApplication::Get().GeneratePathToWidgetChecked(InputTextWidget.ToSharedRef(), WidgetToFocusPath);
	FSlateApplication::Get().SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);
}

TSharedPtr<FName> SDialogueTextPropertyPickList::GetSelectedSuggestion() const
{
	TSharedPtr<FName> SelectedSuggestion;
	const TArray<TSharedPtr<FName>>& SelectedSuggestionList = ListViewWidget->GetSelectedItems();
	if (SelectedSuggestionList.Num() > 0)
	{
		// Selection mode is Single, so there should only be one suggestion at the most
		check(SelectedSuggestionList.Num() == 1)
		SelectedSuggestion = SelectedSuggestionList[0];
	}

	return SelectedSuggestion;
}


//////////////////////////////////////////////////////////////////////////
#undef LOCTEXT_NAMESPACE
