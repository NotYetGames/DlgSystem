// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

#include "DlgDataDisplayTreeNode.h"
#include "DlgDataDisplayActorProperties.h"

class SEditableTextBox;
class SCheckBox;


/** The base type PropertyValue Widget. If just used by itself it displays the VariableValue as a static text. */
class DLGSYSTEM_API SDlgDataPropertyValue : public SCompoundWidget
{
	typedef SDlgDataPropertyValue Self;
	typedef SCompoundWidget Super;
public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FDlgDataDisplayTreeVariableNode>& InVariableNode);

	// SWidget Interface

	/**
	 * Ticks this widget with Geometry.  Override in derived classes, but always call the parent implementation.
	 *
	 * @param  AllottedGeometry The space allotted for this widget
	 * @param  InCurrentTime  Current absolute real time
	 * @param  InDeltaTime  Real time passed since last tick
	 */
	void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;

	/**
	 * Checks to see if this widget supports keyboard focus.  Override this in derived classes.
	 *
	 * @return  True if this widget can take keyboard focus
	 */
	bool SupportsKeyboardFocus() const override
	{
		return VariableNode.IsValid() && PrimaryWidget.IsValid() && PrimaryWidget->SupportsKeyboardFocus();
	}

	/**
	 * Called when focus is given to this widget.  This event does not bubble.
	 *
	 * @param MyGeometry The Geometry of the widget receiving the event
	 * @param  InFocusEvent  The FocusEvent
	 * @return  Returns whether the event was handled, along with other possible actions
	 */
	FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override
	{
		if (PrimaryWidget.IsValid())
		{
			return FReply::Handled().SetUserFocus(PrimaryWidget.ToSharedRef(), InFocusEvent.GetCause());
		}

		return Super::OnFocusReceived(MyGeometry, InFocusEvent);
	}

	// Own functions

	/** Gets the Value of this Property as an FText; */
	FText GetTextValue() const { return FText::FromString(VariableNode->GetVariableValue()); }

protected:
	/** Updates the VariableNode value from the Actor. */
	void UpdateVariableNodeFromActor();

protected:
	/** The Node this widget value represents */
	TSharedPtr<FDlgDataDisplayTreeVariableNode> VariableNode;

	/** Primary Widget of this PropertyValue */
	TSharedPtr<SWidget> PrimaryWidget;

	/** Number of seconds passed in the Tick */
	float TickPassedDeltaTimeSeconds = 0.f;

	static constexpr float TickUpdateTimeSeconds = 1.0f;
};


/** The editable text property value of the variable. */
class SDlgDataTextPropertyValue : public SDlgDataPropertyValue
{
	typedef SDlgDataTextPropertyValue Self;
public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FDlgDataDisplayTreeVariableNode>& InVariableNode);

protected:
	void HandleTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);
	void HandleTextChanged(const FText& NewText);

	bool IsReadOnly() const { return !VariableNode.IsValid(); }

protected:
	/** Widget used for the single line version of the text property */
	TSharedPtr<SEditableTextBox> TextBoxWidget;

	/** True if property is an FName property which causes us to run extra size validation checks */
	bool bIsFNameProperty = false;
};


/** Even property value is just a button that triggers the event. */
class SDlgDataEventPropertyValue : public SDlgDataPropertyValue
{
	typedef SDlgDataEventPropertyValue Self;
public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FDlgDataDisplayTreeVariableNode>& InVariableNode);

	// SWidget Interface
	void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override
	{
		SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	}

protected:
	FReply HandleTriggerEventClicked();
};


/** The editable bool property value of the variable. Is represented by a checkbox */
class SDlgDataBoolPropertyValue : public SDlgDataPropertyValue
{
	typedef SDlgDataBoolPropertyValue Self;
public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedPtr<FDlgDataDisplayTreeVariableNode>& InVariableNode);

	// SWidget interface
	bool HasKeyboardFocus() const override;

	/**
	 * Called when a mouse button is double clicked.  Override this in derived classes.
	 *
	 * @param  InMyGeometry  Widget geometry
	 * @param  InMouseEvent  Mouse button event
	 * @return  Returns whether the event was handled, along with other possible actions
	 */
	FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	ECheckBoxState IsChecked() const;
	void HandleCheckStateChanged(ECheckBoxState InNewState);
	bool IsBoolProperty() const { return VariableNode->GetVariableType() == EDlgDataDisplayVariableTreeNodeType::Bool ||
										 VariableNode->GetVariableType() == EDlgDataDisplayVariableTreeNodeType::ClassBool; }

protected:
	TSharedPtr<SCheckBox> CheckBoxWidget;
};
