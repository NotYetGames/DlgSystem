// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DetailWidgetRow.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "DialogueBase_CustomRowHelper.h"
#include "STextPropertyEditableTextBox.h"

class FDetailWidgetRow;

/**
 * Helper for a custom row when using SMultiLineEditableTextBox.
 */
class FDialogueMultiLineEditableTextBox_CustomRowHelper : public FDialogueBase_CustomRowHelper,
		public TSharedFromThis<FDialogueMultiLineEditableTextBox_CustomRowHelper>
{
	typedef FDialogueMultiLineEditableTextBox_CustomRowHelper Self;
	typedef FDialogueBase_CustomRowHelper Super;
public:
	FDialogueMultiLineEditableTextBox_CustomRowHelper(FDetailWidgetRow* InDetailWidgetRow, const TSharedPtr<IPropertyHandle>& InPropertyHandle)
		: FDialogueBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle) {}

	/** Sets the text box widget */
	Self* SetMultiLineEditableTextBoxWidget(const TSharedRef<SMultiLineEditableTextBox>& InWidget)
	{
		MultiLineEditableTextBoxWidget = InWidget;
		return this;
	}

	/** When the Text Property Changes. Sets it new value. */
	void HandleTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

	/** Gets the value of the text property. */
	FText GetTextValue() const;

protected:
	// Reset to default
	FText GetResetToolTip() const;
	EVisibility GetDiffersFromDefaultAsVisibility() const;
	FReply OnResetClicked();

	
	void UpdateInternal() override;

private:
	static const FText MultipleValuesText;

	/** The text box Widget. */
	TSharedPtr<SMultiLineEditableTextBox> MultiLineEditableTextBoxWidget;

	// Editable text property
	TSharedPtr<IEditableTextProperty> EditableTextProperty;
};
