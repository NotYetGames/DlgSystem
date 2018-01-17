// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "MultiLineEditableTextBox_CustomRowHelper.h"

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "MultiLineEditableTextBox_CustomRowHelper"

const FText MultipleValuesDisplayName = NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values");

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FMultiLineEditableTextBox_CustomRowHelper
void FMultiLineEditableTextBox_CustomRowHelper::UpdateInternal()
{
	check(MultiLineEditableTextBoxWidget.IsValid());
	MultiLineEditableTextBoxWidget->SetToolTipText(ToolTip);

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
		MultiLineEditableTextBoxWidget.ToSharedRef()
	];
}

void FMultiLineEditableTextBox_CustomRowHelper::HandleTextCommited(const FText& NewText, ETextCommit::Type CommitInfo)
{
	FText CurrentText;
	if (PropertyHandle->GetValueAsFormattedText(CurrentText) != FPropertyAccess::MultipleValues ||
		NewText.ToString() != MultipleValuesDisplayName.ToString())
	{
		PropertyHandle->SetValueFromFormattedString(NewText.ToString());
	}
}

FText FMultiLineEditableTextBox_CustomRowHelper::GetTextValue() const
{
	FText Text;

	// Sets the value
	if (PropertyHandle->GetValueAsFormattedText(Text) == FPropertyAccess::MultipleValues)
	{
		// If multiple values
		Text = MultipleValuesDisplayName;
	}

	return Text;
}

#undef LOCTEXT_NAMESPACE
