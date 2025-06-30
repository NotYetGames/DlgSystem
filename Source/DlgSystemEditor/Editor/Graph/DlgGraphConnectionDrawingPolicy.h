// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Layout/ArrangedWidget.h"
#include "Widgets/SWidget.h"
#include "ConnectionDrawingPolicy.h"

#include "DlgSystem/DlgSystemSettings.h"

class FSlateWindowElementList;
class UEdGraph;

// This class draws the connections for an UEdGraph using a Dialogue schema
// Aka how the wires look and and the flow look.
class FDlgGraphConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
	typedef FConnectionDrawingPolicy Super;

	/**
	 * Call order in SGraphPanel:
	 * - DrawPreviewConnector
	 * - SetIncompatiblePinDrawState
	 *   OR
	 * - ResetIncompatiblePinDrawState
	 *
	 * - Draw
	 *		- DetermineLinkGeometry
	 *		- DetermineWiringStyle
	 *		- DrawSplineWithArrow
	 *			- DrawConnection
	 */

public:
	FDlgGraphConnectionDrawingPolicy(
		int32 InBackLayerID,
		int32 InFrontLayerID,
		float ZoomFactor,
		const FSlateRect& InClippingRect,
		FSlateWindowElementList& InDrawElements,
		UEdGraph* InGraphObj
	);

	//
	// FConnectionDrawingPolicy interface
	//

	// Determine how the wires looks
	virtual void DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params) override;

	// Determinte to what geometries the wires are connected.
	virtual void DetermineLinkGeometry(
		FArrangedChildren& ArrangedNodes,
		TSharedRef<SWidget>& OutputPinWidget,
		UEdGraphPin* OutputPin,
		UEdGraphPin* InputPin,
		/*out*/ FArrangedWidget*& StartWidgetGeometry,
		/*out*/ FArrangedWidget*& EndWidgetGeometry
	) override;

	// How should we draw the spline and arrow arrow?
	virtual void DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params) override;
	virtual void DrawSplineWithArrow(const FNYVector2f& StartPoint, const FNYVector2f& EndPoint, const FConnectionParams& Params) override;

	// The method that actually draws the spline
	virtual void DrawConnection(int32 LayerId, const FNYVector2f& Start, const FNYVector2f& End, const FConnectionParams& Params) override;

	// Sometimes the panel draws a preview connector; e.g. when the user is connecting pins
	virtual void DrawPreviewConnector(const FGeometry& PinGeometry, const FNYVector2f& StartPoint, const FNYVector2f& EndPoint, UEdGraphPin* Pin) override;

	// Compute the tangent of the spline.
	virtual FNYVector2f ComputeSplineTangent(const FNYVector2f& Start, const FNYVector2f& End) const override;

	// Draws the wire
	virtual void Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& PinGeometries, FArrangedChildren& ArrangedNodes) override;

	// End of FConnectionDrawingPolicy interface

protected:
	void Internal_DrawLineWithArrow(const FNYVector2f& StartAnchorPoint, const FNYVector2f& EndAnchorPoint, const FConnectionParams& Params);

	// Map for widgets
	UEdGraph* Graph = nullptr;

	// Map for widgets
	TMap<UEdGraphNode*, int32> NodeWidgetMap;

	// Cache the settings
	const UDlgSystemSettings* DialogueSettings = nullptr;
};
