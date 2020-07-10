// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBorder.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SCheckBox.h"

class IPropertyHandle;

/**
 * A widget to edit a Text property but it allows you to pick from a list of possible values and search in it.
 * Inspired from SPropertyMenuAssetPicker and SAssetPicker and SAssetView
 */
class SDialogueTextPropertyPickList : public SCompoundWidget
{
	typedef TSharedPtr<FName> TextListItem;
	typedef SDialogueTextPropertyPickList Self;
	typedef SCompoundWidget Super;

public:
	SLATE_BEGIN_ARGS(Self)
		: _PropertyHandle()
		, _HasContextCheckbox(false)
		, _IsContextCheckBoxChecked(false)
		, _ContextCheckBoxText(NSLOCTEXT("SDialogueTextPropertyPickList", "ContextCheckBoxText", "Local Search"))
		, _ContextCheckBoxToolTipText(NSLOCTEXT("SDialogueTextPropertyPickList", "ContextCheckBoxToolTipText",
									 "Should the search be local? If not checked the search is global."))
		, _ToolTipText(NSLOCTEXT("SDialogueTextPropertyPickList", "DefaultToolTip", "REPLACE ME"))
		, _HintText(NSLOCTEXT("SDialogueTextPropertyPickList", "SearchBoxHint", "Search Names"))
		, _CurrentContextAvailableSuggestions(TArray<FName>())
		, _AvailableSuggestions(TArray<FName>())
		, _OnTextChanged()
		, _OnTextCommitted()
		, _OnKeyDownHandler()
		, _DelayChangeNotificationsWhileTyping(true)
	{}
		/** The Property handle of the property this widget represents. */
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle)

		/** Does this pick list has a checkbox for ContextSensitive suggestions? */
		SLATE_ARGUMENT(bool, HasContextCheckbox)

		/** The check status of the context sensitive checkbox. */
		SLATE_ARGUMENT(bool, IsContextCheckBoxChecked)

		/** The Text that is displayed besides the display checkbox */
		SLATE_ATTRIBUTE(FText, ContextCheckBoxText)

		/** The tooltip text for the context checkbox. */
		SLATE_ATTRIBUTE(FText, ContextCheckBoxToolTipText)

		/** Tooltip text to displayed all over  the place. */
		SLATE_ATTRIBUTE(FText, ToolTipText)

		/** Hint text to display for the search text when there is no value */
		SLATE_ATTRIBUTE(FText, HintText)

		/**
		 *  All possible suggestions for the search text If the context sensitive checkbox is true.
		 *  Aka the current context
		 *  Only used if HasContextCheckbox is true.
		 */
		SLATE_ATTRIBUTE(TArray<FName>, CurrentContextAvailableSuggestions)

		/** All possible suggestions for the search text. If the context sensitive checkbox is false. */
		SLATE_ATTRIBUTE(TArray<FName>, AvailableSuggestions)

		/** Invoked whenever the text changes */
		SLATE_EVENT(FOnTextChanged, OnTextChanged)

		/** Invoked whenever the text is committed (e.g. user presses enter) */
		SLATE_EVENT(FOnTextCommitted, OnTextCommitted)

		/** Callback delegate to have first chance handling of the OnKeyDown event */
		SLATE_EVENT(FOnKeyDown, OnKeyDownHandler)

		/** Whether the SearchBox should delay notifying listeners of text changed events until the user is done typing */
		SLATE_ARGUMENT(bool, DelayChangeNotificationsWhileTyping)
	SLATE_END_ARGS()

	/**
	 * Construct the widget. It must have the PropertyHandle set or else it will fail
	 * @param	InArgs				Arguments for widget construction
	 */
	void Construct(const FArguments& InArgs);

	// SWidget implementation
	/**
	 * Called after a key is pressed when this widget or a child of this widget has focus
	 * If a widget handles this event, OnKeyDown will *not* be passed to the focused widget.
	 *
	 * This event is primarily to allow parent widgets to consume an event before a child widget processes
	 * it and it should be used only when there is no better design alternative.
	 *
	 * @param MyGeometry The Geometry of the widget receiving the event
	 * @param InKeyEvent  Key event
	 * @return Returns whether the event was handled, along with other possible actions
	 */
	FReply OnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/**
	 * Checks to see if this widget supports keyboard focus.  Override this in derived classes.
	 * @return  True if this widget can take keyboard focus
	 */
	bool SupportsKeyboardFocus() const override { return true; }

	/**
	 * Checks to see if this widget currently has the keyboard focus
	 * @return  True if this widget has keyboard focus
	 */
	bool HasKeyboardFocus() const override { return InputTextWidget->HasKeyboardFocus(); }

	/**
	 * Called when focus is given to this widget.  This event does not bubble.
	 *
	 * @param MyGeometry The Geometry of the widget receiving the event
	 * @param  InFocusEvent  The FocusEvent
	 * @return  Returns whether the event was handled, along with other possible actions
	 */
	FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override
	{
		// Forward keyboard focus to our editable text widget
		if (IsEnabled() && ComboButtonWidget.IsValid() && ComboButtonWidget->IsEnabled())
		{
			return FReply::Handled().SetUserFocus(InputTextWidget.ToSharedRef(), InFocusEvent.GetCause());
		}

		return Super::OnFocusReceived(MyGeometry, InFocusEvent);
	}

	// Own functions
	/** Sets the text string currently being edited */
	void SetText(const TAttribute<FText>& InNewText);

	/** Sets the current tooltip being displayed */
	void SetToolTipAttribute(const TAttribute<FText>& InNewText);

	/** Sets the property handle this widget represents. */
	void SetPropertyHandle(const TSharedPtr<IPropertyHandle>& InPropertyHandle);

private:
	/** Gets the menu content widget that is shown once the text is clicked on */
	TSharedRef<SWidget> GetMenuWidget();

	/** Gets the context sensitive checkbox widget */
	TSharedRef<SWidget> GetContextCheckBoxWidget();

	/** Gets the search box widget/input text. */
	TSharedRef<SWidget> GetSearchBoxWidget();

	/** Gets the list view that is displaying the text strings  */
	TSharedRef<SWidget> GetListViewWidget();

	/** Makes the widget row for the list view */
	TSharedRef<ITableRow> HandleListGenerateRow(TextListItem Text, const TSharedRef<STableViewBase>& OwnerTable);

	/** Gets the text to highlight in the suggestion list */
	FText GetHighlightText() const { return InputTextWidget->GetText(); }

	/** Adds A SScrollBorder over a Table */
	TSharedRef<SWidget> CreateShadowOverlay(TSharedRef<STableViewBase> Table) const
	{
		return SNew(SScrollBorder, Table)
			[
				Table
			];
	}

	/** Handles when the menu of the combo box open status changes */
	void HandleMenuOpenChanged(bool bOpen);

	/** Handles when the combo box opens */
	void HandleComboBoxOpened();

	/** Handles when text in the editable text box changed */
	void HandleTextChanged(const FText& InSearchText);

	/** Handles for when text in the editable text box is commited (pressed enter/click). */
	void HandleTextCommitted(const FText& InSearchText, ETextCommit::Type CommitInfo);

	/** Handles key down events to the editable text widget */
	FReply HandleKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	/** Handles when the selection changes in the suggestion list (list view widget) */
	void HandleListSelectionChanged(TextListItem NewValue, ESelectInfo::Type SelectInfo);

	/** Toggle changed. */
	void HandleContextCheckboxChanged(ECheckBoxState CheckState);

	/** Is the context sensitive toggle checked? */
	ECheckBoxState IsContextCheckBoxChecked() const
	{
		return bIsContextCheckBoxChecked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	/** Updates and shows or hides the suggestion list */
	void UpdateSuggestionList();

	/** Returns the currently selected suggestion */
	TextListItem GetSelectedSuggestion() const;

	/** Sets the focus to the InputText box */
	void FocusSearchBox() const;

private:
	/** The Property handle of the property this widget represents. */
	TSharedPtr<IPropertyHandle> PropertyHandle;

	/** All possible suggestions for the search text. When there is no context sensitive checkbox or the context sensitive checkbox is unchecked. */
	TAttribute<TArray<FName>> SuggestionAttributes;

	/** Used when the context sensitive checkbox is cehcked */
	TAttribute<TArray<FName>> CurrentContextSuggestionAttributes;;

	/** Text Value to display for the search text/Combo Button */
	TAttribute<FText> TextAttribute;

	/** Tooltip Value to display.  */
	TAttribute<FText> ToolTipAttribute;

	/** Hint text to display for the search text when there is no value */
	TAttribute<FText> HintTextAttribute;

	/** The Text that is displayed besides the display checkbox */
	TAttribute<FText> ContextCheckBoxTextAttribute;

	/** The tooltip text for the context checkbox. */
	TAttribute<FText> ContextCheckBoxToolTipTextAttribute;

	/** Delegate for when text is changed in the edit box */
	FOnTextChanged OnTextChanged;

	/** Delegate for when text is changed in the edit box */
	FOnTextCommitted OnTextCommitted;

	/** Delegate for first chance handling for key down events */
	FOnKeyDown OnKeyDownHandler;

	/** The list view that is displaying the text strings */
	TSharedPtr<SBorder> ListViewContainerWidget;

	/** The list view for showing all suggestions */
	TSharedPtr<SListView<TextListItem>> ListViewWidget;

	/** Main combo button */
	TSharedPtr<SComboButton> ComboButtonWidget;

	/** Text of the combo button */
	TSharedPtr<STextBlock> ComboButtonTextWidget;

	/** The context sensitive checkbox widget. */
	TSharedPtr<SCheckBox> ContextCheckBoxWidget;

	/** The menu content widget displyed on click. */
	TSharedPtr<SVerticalBox> MenuWidget;

	/** The editable text field aka the search box */
	TSharedPtr<SSearchBox> InputTextWidget;

	/**
	 * All suggestions stored in this widget for the list view.
	 * It represents a snapshot of the SuggestionsAttribute or ContextSensitiveSuggestionsAttribute depending on situation.
	 * Call UpdateSuggestionList to update.
	 */
	TArray<TextListItem> Suggestions;

	/** Whether the SearchBox should delay notifying listeners of text changed events until the user is done typing */
	bool bDelayChangeNotificationsWhileTyping = true;

	/** Do we display the context sensitive checkbox? */
	bool bHasContextCheckBox = false;

	/** Is the box checked? */
	bool bIsContextCheckBoxChecked = false;
};
