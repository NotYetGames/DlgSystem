// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgMultiLineEditableTextBox_CustomRowHelper.h"

#include "DetailWidgetRow.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "DlgSystemEditor/Editor/DetailsPanel/Widgets/DlgEditableTextPropertyHandle.h"
#include "DlgSystemEditor/Editor/DetailsPanel/DlgDetailsPanelUtils.h"

#define LOCTEXT_NAMESPACE "MultiLineEditableTextBox_CustomRowHelper"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgMultiLineEditableTextBoxOptions
void FDlgMultiLineEditableTextBoxOptions::SetDefaults()
{
	const auto DefaultValues = Self{};

	// Set default values
	bSelectAllTextWhenFocused = DefaultValues.bSelectAllTextWhenFocused;
	bClearKeyboardFocusOnCommit = DefaultValues.bClearKeyboardFocusOnCommit;
	bSelectAllTextOnCommit = DefaultValues.bSelectAllTextOnCommit;
	bAutoWrapText = DefaultValues.bAutoWrapText;
	WrapTextAt = DefaultValues.WrapTextAt;
	ModiferKeyForNewLine = FDlgDetailsPanelUtils::GetModifierKeyFromDialogueSettings();

	// Set values that can't be set in the class definition
	Style = FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");
	Font = FNYAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont"));
	// ForegroundColor =
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgMultiLineEditableTextBox_CustomRowHelper
void FDlgMultiLineEditableTextBox_CustomRowHelper::UpdateInternal()
{
	check(PropertyHandle.IsValid());
	check(PropertyUtils.IsValid());

	EditableTextProperty = MakeShared<FDlgEditableTextPropertyHandle>(PropertyHandle.ToSharedRef(), PropertyUtils);

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
			SAssignNew(TextBoxWidget, SDlgTextPropertyEditableTextBox, EditableTextProperty.ToSharedRef(), PropertyHandle.ToSharedRef())
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

FDialogueTextCommitedDelegate& FDlgMultiLineEditableTextBox_CustomRowHelper::OnTextCommittedEvent()
{
	check(TextBoxWidget.IsValid());
	return TextBoxWidget->OnTextCommittedEvent();
}

FDialogueTextChangedDelegate& FDlgMultiLineEditableTextBox_CustomRowHelper::OnTextChangedEvent()
{
	check(TextBoxWidget.IsValid());
	return TextBoxWidget->OnTextChangedEvent();
}

FText FDlgMultiLineEditableTextBox_CustomRowHelper::GetTextValue() const
{
	if (TextBoxWidget.IsValid())
	{
		return TextBoxWidget->GetTextValue();
	}

	return FText::GetEmpty();
}

#undef LOCTEXT_NAMESPACE
