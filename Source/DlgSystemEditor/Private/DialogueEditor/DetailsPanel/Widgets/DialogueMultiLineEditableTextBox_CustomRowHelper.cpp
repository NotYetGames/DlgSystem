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

#define LOCTEXT_NAMESPACE "MultiLineEditableTextBox_CustomRowHelper"

const FText FDialogueMultiLineEditableTextBox_CustomRowHelper::MultipleValuesText = NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values");

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
	return EVisibility::Visible;
}

FReply FDialogueMultiLineEditableTextBox_CustomRowHelper::OnResetClicked()
{
	if (PropertyHandle.IsValid())
	{
		PropertyHandle->ResetToDefault();
	}
	return FReply::Handled();
}

void FDialogueMultiLineEditableTextBox_CustomRowHelper::UpdateInternal()
{
	check(MultiLineEditableTextBoxWidget.IsValid());
	check(PropertyHandle.IsValid());
	check(PropertyUtils.IsValid());
	MultiLineEditableTextBoxWidget->SetToolTipText(ToolTip);

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
				MultiLineEditableTextBoxWidget.ToSharedRef()
			// ]
		]

		// Localization widget
		// TODO reeneable
		// +SHorizontalBox::Slot()
		// .AutoWidth()
		// [
		// 	SNew(STextPropertyEditableTextBox, EditableTextProperty.ToSharedRef())
		// 	.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
		// 	.AutoWrapText(true)
		// ]
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

void FDialogueMultiLineEditableTextBox_CustomRowHelper::HandleTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	FText CurrentText;
	if (PropertyHandle->GetValueAsFormattedText(CurrentText) != FPropertyAccess::MultipleValues ||
		NewText.ToString() != MultipleValuesText.ToString())
	{
		PropertyHandle->SetValueFromFormattedString(NewText.ToString());
	}
}

FText FDialogueMultiLineEditableTextBox_CustomRowHelper::GetTextValue() const
{
	FText Text;

	// Sets the value, only happens in rare cases
	if (PropertyHandle->GetValueAsFormattedText(Text) == FPropertyAccess::MultipleValues)
	{
		// If multiple values
		Text = MultipleValuesText;
	}

	return Text;
}

#undef LOCTEXT_NAMESPACE
