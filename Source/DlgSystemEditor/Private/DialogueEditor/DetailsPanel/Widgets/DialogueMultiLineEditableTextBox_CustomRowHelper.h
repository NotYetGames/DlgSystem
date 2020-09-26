// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DetailWidgetRow.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "DialogueBase_CustomRowHelper.h"
#include "SDialogueTextPropertyEditableTextBox.h"

class FDetailWidgetRow;

struct FDialogueMultiLineEditableTextBoxOptions
{
	typedef FDialogueMultiLineEditableTextBoxOptions Self;
public:
	FDialogueMultiLineEditableTextBoxOptions() {}

	void SetDefaults();

public:
	bool bSelectAllTextWhenFocused = false;
	bool bClearKeyboardFocusOnCommit = false;
	bool bSelectAllTextOnCommit = false;
	bool bAutoWrapText = true;

	float WrapTextAt = 0.f;
	EModifierKey::Type ModiferKeyForNewLine = EModifierKey::None;

	// Can not be set here
	TAttribute<FSlateFontInfo> Font;
	TAttribute<FSlateColor> ForegroundColor;
	TAttribute<FSlateColor> ReadOnlyForegroundColor;
	FEditableTextBoxStyle Style;
};

/**
 * Helper for a custom row when using SMultiLineEditableTextBox.
 */
class FDialogueMultiLineEditableTextBox_CustomRowHelper :
	public FDialogueBase_CustomRowHelper,
	public TSharedFromThis<FDialogueMultiLineEditableTextBox_CustomRowHelper>
{
	typedef FDialogueMultiLineEditableTextBox_CustomRowHelper Self;
	typedef FDialogueBase_CustomRowHelper Super;

public:
	FDialogueMultiLineEditableTextBox_CustomRowHelper(FDetailWidgetRow* InDetailWidgetRow, const TSharedPtr<IPropertyHandle>& InPropertyHandle)
		: FDialogueBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle)
	{
		Options.SetDefaults();
	}

#define CREATE_OPTIONS_SETTER(_NameMethod, _VariableType, _OptionVariableName)  \
	Self& _NameMethod(_VariableType InVariableValue)              \
	{                                                             \
		Options._OptionVariableName = InVariableValue;            \
		return *this;                                             \
	}

	CREATE_OPTIONS_SETTER(Style, const FEditableTextBoxStyle&, Style)
	CREATE_OPTIONS_SETTER(Font, const TAttribute<FSlateFontInfo>&, Font)
	CREATE_OPTIONS_SETTER(ForegroundColor, const TAttribute<FSlateColor>&, ForegroundColor)
	CREATE_OPTIONS_SETTER(ReadOnlyForegroundColor, const TAttribute<FSlateColor>&, ReadOnlyForegroundColor)
	CREATE_OPTIONS_SETTER(SelectAllTextWhenFocused, bool, bSelectAllTextWhenFocused)
	CREATE_OPTIONS_SETTER(ClearKeyboardFocusOnCommit, bool, bClearKeyboardFocusOnCommit)
	CREATE_OPTIONS_SETTER(SelectAllTextOnCommit, bool, bSelectAllTextOnCommit)
	CREATE_OPTIONS_SETTER(AutoWrapText, bool, bAutoWrapText)
	CREATE_OPTIONS_SETTER(WrapTextAt, float, WrapTextAt)
	CREATE_OPTIONS_SETTER(ModiferKeyForNewLine, EModifierKey::Type, ModiferKeyForNewLine)

#undef CREATE_OPTIONS_SETTER

	/** Gets the value of the text property. */
	FText GetTextValue() const;

	// NOTE: only call these after Update()
	FDialogueTextCommitedDelegate& OnTextCommittedEvent();
	FDialogueTextChangedDelegate& OnTextChangedEvent();


protected:
	void UpdateInternal() override;

private:
	// Events
	FDialogueTextCommitedDelegate TextCommittedEvent;

	// The text box Widget.
	TSharedPtr<SDialogueTextPropertyEditableTextBox> TextBoxWidget;

	// Options for the widget
	FDialogueMultiLineEditableTextBoxOptions Options;

	// Editable text property
	TSharedPtr<IEditableTextProperty> EditableTextProperty;
};
