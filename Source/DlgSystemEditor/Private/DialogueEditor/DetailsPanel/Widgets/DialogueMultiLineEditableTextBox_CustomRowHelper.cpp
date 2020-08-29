// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
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
void FDialogueMultiLineEditableTextBox_CustomRowHelper::UpdateInternal()
{
	check(PropertyHandle.IsValid());
	check(PropertyUtils.IsValid());

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
			SAssignNew(TextBoxWidget, SDialogueTextPropertyEditableTextBox, EditableTextProperty.ToSharedRef(), PropertyHandle.ToSharedRef())
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
			.AddResetToDefaultWidget(true)
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
