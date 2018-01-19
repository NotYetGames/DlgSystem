// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "EditorStyle.h"
#include "DetailWidgetRow.h"

#include "Base_CustomRowHelper.h"

class FDetailWidgetRow;

/**
 * Helper for a custom row when using SMultiLineEditableTextBox.
 */
class FMultiLineEditableTextBox_CustomRowHelper : public FBase_CustomRowHelper,
		public TSharedFromThis<FMultiLineEditableTextBox_CustomRowHelper>
{
	typedef FMultiLineEditableTextBox_CustomRowHelper Self;
	typedef FBase_CustomRowHelper Super;
public:
	FMultiLineEditableTextBox_CustomRowHelper(FDetailWidgetRow* InDetailWidgetRow, TSharedPtr<IPropertyHandle> InPropertyHandle)
		: FBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle) {}

	/** Sets the text box widget */
	Self* SetMultiLineEditableTextBoxWidget(const TSharedRef<SMultiLineEditableTextBox> InWidget)
	{
		MultiLineEditableTextBoxWidget = InWidget;
		return this;
	}

	/** When the Text Property Changes. Sets it new value. */
	void HandleTextCommited(const FText& NewText, ETextCommit::Type CommitInfo);

	/** Gets the value of the text property. */
	FText GetTextValue() const;

private:
	void UpdateInternal() override;

private:
	/** The text box Widget. */
	TSharedPtr<SMultiLineEditableTextBox> MultiLineEditableTextBoxWidget;
};
