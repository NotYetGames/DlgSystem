// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/CoreStyle.h"
#include "STextPropertyEditableTextBox.h"
#include "PropertyHandle.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FDialogueTextCommitedDelegate, const FText&, ETextCommit::Type);
DECLARE_MULTICAST_DELEGATE_OneParam(FDialogueTextChangedDelegate, const FText&);


// Own STextPropertyEditableTextBox version with custom modifications
// Used to edit FText multiline FText instances
class SDialogueTextPropertyEditableTextBox : public SCompoundWidget
{
	typedef SDialogueTextPropertyEditableTextBox Self;

	SLATE_BEGIN_ARGS(SDialogueTextPropertyEditableTextBox)
		: _Style(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
		, _Font()
		, _ForegroundColor()
		, _ReadOnlyForegroundColor()
		, _WrapTextAt(0.0f)
		, _AutoWrapText(true)
		, _SelectAllTextWhenFocused(false)
		, _ClearKeyboardFocusOnCommit(false)
		, _SelectAllTextOnCommit(false)
		, _AddResetToDefaultWidget(true)
		, _ModiferKeyForNewLine(EModifierKey::None)

		// Similar to TextProperty, see FTextCustomization
		, _MinDesiredWidth(209.f)
		, _MaxDesiredHeight(600.f)
		{}
		/** The styling of the textbox */
		SLATE_STYLE_ARGUMENT(FEditableTextBoxStyle, Style)

		/** Font color and opacity (overrides Style) */
		SLATE_ATTRIBUTE(FSlateFontInfo, Font)

		/** Text color and opacity (overrides Style) */
		SLATE_ATTRIBUTE(FSlateColor, ForegroundColor)

		/** Text color and opacity when read-only (overrides Style) */
		SLATE_ATTRIBUTE(FSlateColor, ReadOnlyForegroundColor)

		/** Whether text wraps onto a new line when it's length exceeds this width; if this value is zero or negative, no wrapping occurs */
		SLATE_ATTRIBUTE(float, WrapTextAt)

		/** Whether to wrap text automatically based on the widget's computed horizontal space */
		SLATE_ATTRIBUTE(bool, AutoWrapText)

		/** Whether to select all text when the user clicks to give focus on the widget */
		SLATE_ATTRIBUTE(bool, SelectAllTextWhenFocused)

		/** Whether to clear keyboard focus when pressing enter to commit changes */
		SLATE_ATTRIBUTE(bool, ClearKeyboardFocusOnCommit)

		/** Whether to select all text when pressing enter to commit changes */
		SLATE_ATTRIBUTE(bool, SelectAllTextOnCommit)

		// Add reset to default buton
		SLATE_ARGUMENT(bool, AddResetToDefaultWidget)

		/** The optional modifier key necessary to create a newline when typing into the editor. */
		SLATE_ARGUMENT(EModifierKey::Type, ModiferKeyForNewLine)

		/** When specified, will report the MinDesiredWidth if larger than the content's desired width */
		SLATE_ATTRIBUTE(FOptionalSize, MinDesiredWidth)

		/** When specified, will report the MaxDesiredHeight if smaller than the content's desired height */
		SLATE_ATTRIBUTE(FOptionalSize, MaxDesiredHeight)
	SLATE_END_ARGS()

public:
	void Construct(
		const FArguments& Arguments,
		const TSharedRef<IEditableTextProperty>& InEditableTextProperty,
		const TSharedRef<IPropertyHandle>& InPropertyHandle
	);
	bool SupportsKeyboardFocus() const override;
	FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;

	FText GetTextValue() const;
	void SetTextValue(const FText& NewValue);

	FDialogueTextCommitedDelegate& OnTextCommittedEvent() { return TextCommittedEvent; }
	FDialogueTextChangedDelegate& OnTextChangedEvent() { return TextChangedEvent; }

private:
	void GetDesiredWidth(float& OutMinDesiredWidth, float& OutMaxDesiredWidth);
	bool CanEdit() const;
	bool IsCultureInvariantFlagEnabled() const;
	bool IsSourceTextReadOnly() const;

	bool WillNamespaceBeUpdated() const;
	bool IsNamespaceReadOnly() const;
	bool IsKeyReadOnly() const { return IsIdentityReadOnly(); }
	bool IsIdentityReadOnly() const;

	FText GetToolTipText() const;
	EVisibility GetLocalizableVisibility() const;

	void OnTextChanged(const FText& NewText);
	void OnTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);
	void SetTextError(const FText& InErrorMsg);

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

	EVisibility GetNamespaceOverridenWarningImageVisibility() const;
	EVisibility GetTextWarningImageVisibility() const;

	bool IsValidIdentity(const FText& InIdentity, FText* OutReason = nullptr, const FText* InErrorCtx = nullptr) const;

	// Reset to default
	FText GetResetToolTip() const;
	EVisibility GetDiffersFromDefaultAsVisibility() const;
	FReply OnResetClicked();

protected:
	// Events
	FDialogueTextCommitedDelegate TextCommittedEvent;
	FDialogueTextChangedDelegate TextChangedEvent;

	// Property variables
	TSharedPtr<IEditableTextProperty> EditableTextProperty;
	TSharedPtr<IPropertyHandle> PropertyHandle;

	TSharedPtr<SWidget> PrimaryWidget;
	TSharedPtr<SMultiLineEditableTextBox> MultiLineWidget;

	// TSharedPtr<SEditableTextBox> SingleLineWidget;

	TSharedPtr<SEditableTextBox> NamespaceEditableTextBox;
	TSharedPtr<SEditableTextBox> KeyEditableTextBox;

	TOptional<float> PreviousHeight;

	bool bIsMultiLine = true;
	bool bAddResetToDefaultWidget = true;

	static FText MultipleValuesText;
};
