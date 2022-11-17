// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

/** Widget for overlaying another widget besides the node */
class SDialogueNodeOverlayWidget : public SCompoundWidget
{
	typedef SDialogueNodeOverlayWidget Self;

public:
	/** Delegate event fired when the hover state of this widget changes */
	DECLARE_DELEGATE_OneParam(FOnHoverStateChanged, bool /* bHovered */);

	/** Delegate used to receive the background color of the node, depending on hover state and state of other siblings */
	DECLARE_DELEGATE_RetVal_OneParam(FSlateColor, FOnGetBackgroundColor, bool /* bHovered */);

	SLATE_BEGIN_ARGS(SDialogueNodeOverlayWidget) {}
		SLATE_ATTRIBUTE(TSharedPtr<SWidget>, OverlayBody)

		// Events
		SLATE_EVENT(FOnHoverStateChanged, OnHoverStateChanged)
		SLATE_EVENT(FOnGetBackgroundColor, OnGetBackgroundColor)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(true);
		SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
	}

	void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		OnHoverStateChangedEvent.ExecuteIfBound(false);
		SCompoundWidget::OnMouseLeave(MouseEvent);
	}

	/** Get the color we use to display the rounded border */
	FSlateColor GetBackgroundColor() const
	{
		if (OnGetBackgroundColorEvent.IsBound())
			return OnGetBackgroundColorEvent.Execute(IsHovered());

		return FSlateColor::UseForeground();
	}

private:
	/** Delegate event fired when the hover state of this widget changes */
	FOnHoverStateChanged OnHoverStateChangedEvent;

	/** Delegate used to receive the color of the node, depending on hover state and state of other siblings */
	FOnGetBackgroundColor OnGetBackgroundColorEvent;

	/** The OverlayBody used for this widget*/
	TSharedPtr<SWidget> OverlayBody;
};
