// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SCompoundWidget.h"

#include "DlgDataDisplayTreeNode.h"
#include "DlgDataDisplayActorProperties.h"


/** The base type PropertyValue Widget. If just used by itself it displays the VariableValue as a static text. */
class SDlgDataPropertyValue : public SCompoundWidget
{
	typedef SDlgDataPropertyValue Self;
	typedef SCompoundWidget Super;
public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FDlgDataDisplayTreeVariableNode> InVariableNode);

	// SWidget Interface
	/**
	 * Ticks this widget with Geometry.  Override in derived classes, but always call the parent implementation.
	 *
	 * @param  AllottedGeometry The space allotted for this widget
	 * @param  InCurrentTime  Current absolute real time
	 * @param  InDeltaTime  Real time passed since last tick
	 */
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Gets the Value of this Property as an FText; */
	FText GetTextValue() const { return FText::FromString(VariableNode->GetVariableValue()); }

protected:
	/** Updates the VariableNode value from the Actor. */
	void UpdateVariableNodeFromActor();

protected:
	/** The Node this widget value represents */
	TSharedPtr<FDlgDataDisplayTreeVariableNode> VariableNode;

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

	void Construct(const FArguments& InArgs, TSharedPtr<FDlgDataDisplayTreeVariableNode> InVariableNode);

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

	void Construct(const FArguments& InArgs, TSharedPtr<FDlgDataDisplayTreeVariableNode> InVariableNode);

	// SWidget Interface
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	}

protected:
	FReply HandleTriggerEventClicked();
};
