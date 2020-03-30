// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "SGraphPin.h"

#include "DlgSystemSettings.h"
#include "DialogueGraphNode_Edge.h"

/** Own SGraphPin  custom class, allows us to customize the pins as we like. */
class SDialogueGraphPin : public SGraphPin
{
	typedef SGraphPin Super;
	typedef SDialogueGraphPin Self;

public:
	SLATE_BEGIN_ARGS(Self) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

	// Begin SGraphPin Interface
	/**
	 * Handle clicking on the pin.
	 * Note: Used instead of OnMouseButtonDown because this is called by the SGraphPanel, NOT OnMouseButtonDown
	 */
	FReply OnPinMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent) override;

	// SWidget interface
	/**
	 * The system calls this method to notify the widget that a mouse button was release within it. This event is bubbled.
	 *
	 * @param MyGeometry The Geometry of the widget receiving the event
	 * @param MouseEvent Information about the input event
	 * @return Whether the event was handled along with possible requests for the system to take action.
	*/
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/**
	 * The system calls this method to notify the widget that a mouse moved within it. This event is bubbled.
	 *
	 * @param MyGeometry The Geometry of the widget receiving the event
	 * @param MouseEvent Information about the input event
	 * @return Whether the event was handled along with possible requests for the system to take action.
	*/
	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/**
	 * The system will use this event to notify a widget that the cursor has entered it. This event is uses a custom bubble strategy.
	 *
	 * @param MyGeometry The Geometry of the widget receiving the event
	 * @param MouseEvent Information about the input event
	*/
	void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/**
	 * The system will use this event to notify a widget that the cursor has left it. This event is uses a custom bubble strategy.
	 *
	 * @param MouseEvent Information about the input event
	 */
	void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	//
	// DRAG AND DROP (DragDrop)
	//
	/**
	 * Called during drag and drop when the drag enters a widget.
	 *
	 * Enter/Leave events in slate are meant as lightweight notifications.
	 * So we do not want to capture mouse or set focus in response to these.
	 * However, OnDragEnter must also support external APIs (e.g. OLE Drag/Drop)
	 * Those require that we let them know whether we can handle the content
	 * being dragged OnDragEnter.
	 *
	 * The concession is to return a can_handled/cannot_handle
	 * boolean rather than a full FReply.
	 *
	 * @param MyGeometry      The geometry of the widget receiving the event.
	 * @param DragDropEvent   The drag and drop event.
	 *
	 * @return A reply that indicated whether the contents of the DragDropEvent can potentially be processed by this widget.
	 */
	void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	/**
	 * Called during drag and drop when the drag leaves a widget.
	 *
	 * @param DragDropEvent   The drag and drop event.
	 */
	void OnDragLeave(const FDragDropEvent& DragDropEvent) override;

	/**
	 * Called during drag and drop when the the mouse is being dragged over a widget.
	 *
	 * @param MyGeometry      The geometry of the widget receiving the event.
	 * @param DragDropEvent   The drag and drop event.
	 * @return A reply that indicated whether this event was handled.
	 */
	FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

	/**
	 * Called when the user is dropping something onto a widget; terminates drag and drop.
	 *
	 * @param MyGeometry      The geometry of the widget receiving the event.
	 * @param DragDropEvent   The drag and drop event.
	 * @return A reply that indicated whether this event was handled.
	 */
	FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// End of SWidget interface

protected:
	// Begin SGraphPin Interface
	/** Get the widget we should put into the 'default value' space, shown when nothing connected */
	TSharedRef<SWidget>	GetDefaultValueWidget() override
	{
		return SNew(STextBlock);
	}

	/** @return The color that we should use to draw this pin */
	FSlateColor GetPinColor() const override
	{
		const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
		return IsHovered() ? Settings->BorderHoveredBackgroundColor : Settings->BorderBackgroundColor;
	}

	// Begin own functions
	/** Handles when the alt button and left mouse is on the pin.  */
	FReply OnAltAndLeftMouseButtonDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent);

	/** Handles when the ctrl button and left mouse is on the pin.  */
	FReply OnCtrlAndLeftMouseButtonDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent);

	/** @return The tooltip to display for this pin */
	FText GetTooltipText() const;

	/** Gets the Index in the current pin LinkeTo array that corresponds to the MousePosition on the wire/spline. */
	UEdGraphPin* GetBestLinkedToPinFromSplineMousePosition(const FVector2D& MousePosition) const;

	/** Gets the pin border */
	const FSlateBrush* GetPinBorder() const
	{
		return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Body"));
	}
};
