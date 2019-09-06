// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DetailWidgetRow.h"

#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Base_CustomRowHelper.h"
#include "STextPropertyEditableTextBox.h"

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
	FMultiLineEditableTextBox_CustomRowHelper(FDetailWidgetRow* InDetailWidgetRow, const TSharedPtr<IPropertyHandle>& InPropertyHandle)
		: FBase_CustomRowHelper(InDetailWidgetRow, InPropertyHandle) {}

	/** Sets the text box widget */
	Self* SetMultiLineEditableTextBoxWidget(const TSharedRef<SMultiLineEditableTextBox>& InWidget)
	{
		MultiLineEditableTextBoxWidget = InWidget;
		return this;
	}

	/** When the Text Property Changes. Sets it new value. */
	void HandleTextCommited(const FText& NewText, ETextCommit::Type CommitInfo);

	/** Gets the value of the text property. */
	FText GetTextValue() const;

protected:
	// Reset to default
	FText GetResetToolTip() const;
	EVisibility GetDiffersFromDefaultAsVisibility() const;
	FReply OnResetClicked();

	// Edit advanced text settings
	bool CanEdit() const;
	bool IsCultureInvariantFlagEnabled() const;
	bool IsSourceTextReadOnly() const;
	bool IsIdentityReadOnly() const;

	FText GetNamespaceValue() const;
	void OnNamespaceChanged(const FText& NewText);
	void OnNamespaceCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

	FText GetKeyValue() const;
#if USE_STABLE_LOCALIZATION_KEYS
	void OnKeyChanged(const FText& NewText);
	void OnKeyCommitted(const FText& NewText, ETextCommit::Type CommitInfo);
	FText GetPackageValue() const;
#endif // USE_STABLE_LOCALIZATION_KEYS

	ECheckBoxState GetLocalizableCheckState(bool bActiveState) const;
	void HandleLocalizableCheckStateChanged(ECheckBoxState InCheckboxState, bool bActiveState);

	bool IsValidIdentity(const FText& InIdentity, FText* OutReason = nullptr, const FText* InErrorCtx = nullptr) const;
	EVisibility GetTextWarningImageVisibility() const;

	void UpdateInternal() override;

private:
	static const FText MultipleValuesText;

	/** The text box Widget. */
	TSharedPtr<SMultiLineEditableTextBox> MultiLineEditableTextBoxWidget;

	// Editable text property
	TSharedPtr<IEditableTextProperty> EditableTextProperty;

	// For localization
	TSharedPtr<SEditableTextBox> KeyEditableTextBox;
	TSharedPtr<SEditableTextBox> NamespaceEditableTextBox;
};
