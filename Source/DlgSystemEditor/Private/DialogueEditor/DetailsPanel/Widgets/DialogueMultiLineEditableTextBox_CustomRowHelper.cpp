// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DialogueMultiLineEditableTextBox_CustomRowHelper.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "IPropertyUtilities.h"
#include "DialogueEditableTextPropertyHandle.h"
#include "DialogueEditor/DetailsPanel/DialogueDetailsPanelUtils.h"

#define LOCTEXT_NAMESPACE "MultiLineEditableTextBox_CustomRowHelper"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueMultiLineEditableTextBoxOptions
void FDialogueMultiLineEditableTextBoxOptions::SetDefaults()
{
	const auto DefaultValues = Self{};

	// Set default values
	bSelectAllTextWhenFocused = DefaultValues.bSelectAllTextWhenFocused;
	bClearKeyboardFocusOnCommit = DefaultValues.bClearKeyboardFocusOnCommit;
	bSelectAllTextOnCommit = DefaultValues.bSelectAllTextOnCommit;
	bAutoWrapText = DefaultValues.bAutoWrapText;
	WrapTextAt = DefaultValues.WrapTextAt;
	ModiferKeyForNewLine = FDialogueDetailsPanelUtils::GetModifierKeyFromDialogueSettings();

	// Set values that can't be set in the class definition
	Style = FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");
	Font = FEditorStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont"));
	// ForegroundColor = 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDialogueMultiLineEditableTextBox_CustomRowHelper
FText FDialogueMultiLineEditableTextBox_CustomRowHelper::GetResetToolTip() const
{
	FString Tooltip = NSLOCTEXT("PropertyEditor", "ResetToDefaultToolTip", "Reset to Default").ToString();

	if (PropertyHandle.IsValid() && !PropertyHandle->IsEditConst() && PropertyHandle->DiffersFromDefault())
	{
		const FString DefaultLabel = PropertyHandle->GetResetToDefaultLabel().ToString();
		if (DefaultLabel.Len() > 0)
		{
			Tooltip += "\n";
			Tooltip += DefaultLabel;
		}
	}

	return FText::FromString(Tooltip);
}

EVisibility FDialogueMultiLineEditableTextBox_CustomRowHelper::GetDiffersFromDefaultAsVisibility() const
{
	if (PropertyHandle.IsValid())
	{
		return PropertyHandle->DiffersFromDefault() ? EVisibility::Visible : EVisibility::Hidden;
	}
	
	return EVisibility::Visible;
}

FReply FDialogueMultiLineEditableTextBox_CustomRowHelper::OnResetClicked()
{
	if (EditableTextProperty.IsValid() && PropertyHandle.IsValid())
	{
		PropertyHandle->ResetToDefault();

		// Broadcast to children
		for (int32 Index = 0; Index < EditableTextProperty->GetNumTexts(); Index++)
		{
			OnTextCommittedEvent().Broadcast(EditableTextProperty->GetText(Index), ETextCommit::OnCleared);
		}
	}
	return FReply::Handled();
}

void FDialogueMultiLineEditableTextBox_CustomRowHelper::UpdateInternal()
{
	check(PropertyHandle.IsValid());
	check(PropertyUtils.IsValid());

	PropertyHandle->MarkResetToDefaultCustomized(true);
	EditableTextProperty = MakeShared<FDialogueEditableTextPropertyHandle>(PropertyHandle.ToSharedRef(), PropertyUtils);

	TSharedPtr<SHorizontalBox> HorizontalBox;
	DetailWidgetRow
	->NameContent()
	[
		NameContentWidget.ToSharedRef()
	]
	.ValueContent()
	// Similar to TextProperty, see FTextCustomization
	.MinDesiredWidth(209.f)
	.MaxDesiredWidth(600.f)
	[
		SAssignNew(HorizontalBox, SHorizontalBox)
		+SHorizontalBox::Slot()
		.Padding(0.f, 0.f, 4.f, 0.f)
		.FillWidth(1.f)
		[
			// Set custom size of text box
			// SNew(SBox)
			// .HeightOverride(100)
			// [
				//MultiLineEditableTextBoxWidget.ToSharedRef()
			// ]
			//
			SAssignNew(TextBoxWidget, SDialogueTextPropertyEditableTextBox, EditableTextProperty.ToSharedRef())
			.Style(&Options.Style)
			.Font(Options.Font)
			.ForegroundColor(Options.ForegroundColor)
			.ReadOnlyForegroundColor(Options.ReadOnlyForegroundColor)
			.SelectAllTextWhenFocused(Options.bSelectAllTextWhenFocused)
			.ClearKeyboardFocusOnCommit(Options.bClearKeyboardFocusOnCommit)
			.SelectAllTextOnCommit(Options.bSelectAllTextOnCommit)
			.AutoWrapText(Options.bAutoWrapText)
			.WrapTextAt(Options.WrapTextAt)
			.ModiferKeyForNewLine(Options.ModiferKeyForNewLine)
		]
	];

	// Add Reset to default
	HorizontalBox->AddSlot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(4.f, 2.f)
	[
		SNew(SButton)
		.IsFocusable(false)
		.ToolTipText(this, &Self::GetResetToolTip)
		.ButtonStyle(FEditorStyle::Get(), "NoBorder")
		.ContentPadding(0)
		.Visibility(this, &Self::GetDiffersFromDefaultAsVisibility)
		.OnClicked(this, &Self::OnResetClicked)
		.Content()
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("PropertyWindow.DiffersFromDefault"))
		]
	];
}

FDialogueTextCommitedDelegate& FDialogueMultiLineEditableTextBox_CustomRowHelper::OnTextCommittedEvent()
{
	check(TextBoxWidget.IsValid());
	return TextBoxWidget->OnTextCommittedEvent();
}

FDialogueTextChangedDelegate& FDialogueMultiLineEditableTextBox_CustomRowHelper::OnTextChangedEvent()
{
	check(TextBoxWidget.IsValid());
	return TextBoxWidget->OnTextChangedEvent();
}

FText FDialogueMultiLineEditableTextBox_CustomRowHelper::GetTextValue() const
{
	if (TextBoxWidget.IsValid())
	{
		return TextBoxWidget->GetTextValue();
	}
	
	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
