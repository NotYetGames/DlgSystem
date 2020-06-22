// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DialogueGraphConnectionDrawingPolicy.h"

#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"

#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueEditor/Nodes/DialogueGraphNode_Edge.h"
#include "DialogueEditor/Nodes/SDialogueGraphNode_Edge.h"

/////////////////////////////////////////////////////
// FDialogueGraphConnectionDrawingPolicy
FDialogueGraphConnectionDrawingPolicy::FDialogueGraphConnectionDrawingPolicy(
	int32 InBackLayerID,
	int32 InFrontLayerID,
	float ZoomFactor,
	const FSlateRect& InClippingRect,
	FSlateWindowElementList& InDrawElements,
	UEdGraph* InGraphObj
) : Super(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements),
	Graph(InGraphObj), DialogueSettings(GetDefault<UDlgSystemSettings>())
{
}

void FDialogueGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
	/*inout*/ FConnectionParams& Params)
{
	// Is the connection bidirectional?
	Params.bUserFlag1 = false;
	Params.AssociatedPin1 = OutputPin;
	Params.AssociatedPin2 = InputPin;
	Params.WireThickness = DialogueSettings->WireThickness;
	Params.bDrawBubbles = DialogueSettings->bWireDrawBubbles;

	if (InputPin)
	{
		if (const UDialogueGraphNode_Edge* GraphNode_Edge = Cast<UDialogueGraphNode_Edge>(InputPin->GetOwningNode()))
		{
			Params.WireColor = GraphNode_Edge->GetEdgeColor(HoveredPins.Contains(InputPin));
		}
	}

	const bool bDeemphasizeUnhoveredPins = HoveredPins.Num() > 0;
	if (bDeemphasizeUnhoveredPins)
	{
		ApplyHoverDeemphasis(OutputPin, InputPin, /*inout*/ Params.WireThickness, /*inout*/ Params.WireColor);
	}
}

void FDialogueGraphConnectionDrawingPolicy::DetermineLinkGeometry(
	FArrangedChildren& ArrangedNodes,
	TSharedRef<SWidget>& OutputPinWidget,
	UEdGraphPin* OutputPin,
	UEdGraphPin* InputPin,
	/*out*/ FArrangedWidget*& StartWidgetGeometry,
	/*out*/ FArrangedWidget*& EndWidgetGeometry
)
{
	if (UDialogueGraphNode_Edge* GraphNode_Edge = Cast<UDialogueGraphNode_Edge>(InputPin->GetOwningNode()))
	{
		// Do not determine anything, this way the links won't be drawn
		if (!GraphNode_Edge->ShouldDrawEdge())
			return;

		// Find the actual nodes each edge is connected to, and link to those
		// From Parent to Child, the Edge is just a proxy
		UDialogueGraphNode* ParentNode = GraphNode_Edge->GetParentNode();
		UDialogueGraphNode* ChildNode = GraphNode_Edge->GetChildNode();
		int32* PrevNodeIndex = NodeWidgetMap.Find(ParentNode);
		int32* NextNodeIndex = NodeWidgetMap.Find(ChildNode);
		if (PrevNodeIndex != nullptr && NextNodeIndex != nullptr)
		{
			StartWidgetGeometry = &(ArrangedNodes[*PrevNodeIndex]);
			EndWidgetGeometry = &(ArrangedNodes[*NextNodeIndex]);
		}
	}
	else
	{
		checkNoEntry();
		//FConnectionDrawingPolicy::DetermineLinkGeometry(ArrangedNodes, OutputPinWidget, OutputPin, InputPin, StartWidgetGeometry, EndWidgetGeometry);
	}
}

void FDialogueGraphConnectionDrawingPolicy::DrawSplineWithArrow(
	const FGeometry& StartGeom,
	const FGeometry& EndGeom,
	const FConnectionParams& Params
)
{
	// Draw the spline and arrow from/to the closest points between the two geometries (nodes)
	// Get a reasonable seed point (halfway between the boxes)
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) / 2.0f;

	// Find the (approximate) closest points between the two boxes
	const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

void FDialogueGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartPoint, const FVector2D& EndPoint,
	const FConnectionParams& Params)
{
	Internal_DrawLineWithArrow(StartPoint, EndPoint, Params);
	// Is the connection bidirectional?
	if (Params.bUserFlag1)
	{
		Internal_DrawLineWithArrow(EndPoint, StartPoint, Params);
	}
}

void FDialogueGraphConnectionDrawingPolicy::DrawConnection(
	int32 LayerId,
	const FVector2D& Start,
	const FVector2D& End,
	const FConnectionParams& Params
)
{
	// Code mostly from Super::DrawConnection
	const FVector2D& P0 = Start;
	const FVector2D& P1 = End;

	const FVector2D SplineTangent = ComputeSplineTangent(P0, P1);
	const FVector2D P0Tangent = Params.StartDirection == EGPD_Output ? SplineTangent : -SplineTangent;
	const FVector2D P1Tangent = Params.EndDirection == EGPD_Input ? SplineTangent : -SplineTangent;

	if (Settings->bTreatSplinesLikePins)
	{
		// Distance to consider as an overlap
		const float QueryDistanceTriggerThresholdSquared = FMath::Square(Settings->SplineHoverTolerance + Params.WireThickness * 0.5f);

		// Distance to pass the bounding box cull test (may want to expand this later on if we want to do 'closest pin' actions that don't require an exact hit)
		const float QueryDistanceToBoundingBoxSquared = QueryDistanceTriggerThresholdSquared;

		bool bCloseToSpline = false;
		{
			// The curve will include the endpoints but can extend out of a tight bounds because of the tangents
			// P0Tangent coefficient maximizes to 4/27 at a=1/3, and P1Tangent minimizes to -4/27 at a=2/3.
			constexpr float MaximumTangentContribution = 4.0f / 27.0f;
			FBox2D Bounds(ForceInit);

			Bounds += FVector2D(P0);
			Bounds += FVector2D(P0 + MaximumTangentContribution * P0Tangent);
			Bounds += FVector2D(P1);
			Bounds += FVector2D(P1 - MaximumTangentContribution * P1Tangent);

			bCloseToSpline = Bounds.ComputeSquaredDistanceToPoint(LocalMousePosition) < QueryDistanceToBoundingBoxSquared;
		}

		if (bCloseToSpline)
		{
			// Find the closest approach to the spline
			FVector2D ClosestPoint(ForceInit);
			float ClosestDistanceSquared = FLT_MAX;

			constexpr int32 NumStepsToTest = 16;
			constexpr float StepInterval = 1.0f / static_cast<float>(NumStepsToTest);
			FVector2D Point1 = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, 0.0f);
			for (float TestAlpha = 0.0f; TestAlpha < 1.0f; TestAlpha += StepInterval)
			{
				const FVector2D Point2 = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, TestAlpha + StepInterval);

				const FVector2D ClosestPointToSegment = FMath::ClosestPointOnSegment2D(LocalMousePosition, Point1, Point2);
				const float DistanceSquared = (LocalMousePosition - ClosestPointToSegment).SizeSquared();

				if (DistanceSquared < ClosestDistanceSquared)
				{
					ClosestDistanceSquared = DistanceSquared;
					ClosestPoint = ClosestPointToSegment;
				}

				Point1 = Point2;
			}

			// Record the overlap
			if (ClosestDistanceSquared < QueryDistanceTriggerThresholdSquared)
			{
				if (ClosestDistanceSquared < SplineOverlapResult.GetDistanceSquared())
				{
					const float SquaredDistToPin1 = Params.AssociatedPin1 != nullptr ? (P0 - ClosestPoint).SizeSquared() : FLT_MAX;

					float SquaredDistToPin2 = FLT_MAX;
					UEdGraphPin* InputPin = Params.AssociatedPin2;
					// Can happen when we call DrawPreviewConnector
					if (InputPin != nullptr)
					{
						// Must be Edge input pin
						UDialogueGraphNode_Edge* GraphNode_Edge = CastChecked<UDialogueGraphNode_Edge>(Params.AssociatedPin2->GetOwningNode());
						InputPin = GraphNode_Edge->GetChildNode()->GetInputPin();
						SquaredDistToPin2 = (P1 - ClosestPoint).SizeSquared();
					}

					// The AssociatedPin2 is the Edge pin, which is not displayed, always hover over the current NodePin
					SplineOverlapResult = FGraphSplineOverlapResult(Params.AssociatedPin1, Params.AssociatedPin1, ClosestDistanceSquared,
																	SquaredDistToPin1, SquaredDistToPin1);
				}
			}
		}
	}

	// Draw the spline itself
	FSlateDrawElement::MakeDrawSpaceSpline(
		DrawElementsList,
		LayerId,
		P0, P0Tangent,
		P1, P1Tangent,
		Params.WireThickness,
		ESlateDrawEffect::None,
		Params.WireColor
	);

	if (Params.bDrawBubbles || (MidpointImage != nullptr))
	{
		// This table maps distance along curve to alpha
		FInterpCurve<float> SplineReparamTable;
		const float SplineLength = MakeSplineReparamTable(P0, P0Tangent, P1, P1Tangent, SplineReparamTable);

		// Draw bubbles on the spline
		if (Params.bDrawBubbles)
		{
			const float BubbleSpacing = 64.f * ZoomFactor;
			const float BubbleSpeed = 192.f * ZoomFactor;
			const FVector2D BubbleSize = BubbleImage->ImageSize * ZoomFactor * 0.1f * Params.WireThickness;

			const float Time = FPlatformTime::Seconds() - GStartTime;
			const float BubbleOffset = FMath::Fmod(Time * BubbleSpeed, BubbleSpacing);
			const int32 NumBubbles = FMath::CeilToInt(SplineLength / BubbleSpacing);
			for (int32 i = 0; i < NumBubbles; ++i)
			{
				const float Distance = (static_cast<float>(i) * BubbleSpacing) + BubbleOffset;
				if (Distance < SplineLength)
				{
					const float Alpha = SplineReparamTable.Eval(Distance, 0.f);
					FVector2D BubblePos = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, Alpha);
					BubblePos -= (BubbleSize * 0.5f);

					FSlateDrawElement::MakeBox(
						DrawElementsList,
						LayerId,
						FPaintGeometry(BubblePos, BubbleSize, ZoomFactor),
						BubbleImage,
						ESlateDrawEffect::None,
						Params.WireColor
					);
				}
			}
		}

		// Draw the midpoint image
		if (MidpointImage != nullptr)
		{
			// Determine the spline position for the midpoint
			const float MidpointAlpha = SplineReparamTable.Eval(SplineLength * 0.5f, 0.f);
			const FVector2D Midpoint = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, MidpointAlpha);

			// Approximate the slope at the midpoint (to orient the midpoint image to the spline)
			const FVector2D MidpointPlusE = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, MidpointAlpha + KINDA_SMALL_NUMBER);
			const FVector2D MidpointMinusE = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, MidpointAlpha - KINDA_SMALL_NUMBER);
			const FVector2D SlopeUnnormalized = MidpointPlusE - MidpointMinusE;

			// Draw the arrow
			const FVector2D MidpointDrawPos = Midpoint - MidpointRadius;
			const float AngleInRadians = SlopeUnnormalized.IsNearlyZero() ? 0.0f : FMath::Atan2(SlopeUnnormalized.Y, SlopeUnnormalized.X);

			FSlateDrawElement::MakeRotatedBox(
				DrawElementsList,
				LayerId,
				FPaintGeometry(MidpointDrawPos, MidpointImage->ImageSize * ZoomFactor, ZoomFactor),
				MidpointImage,
				ESlateDrawEffect::None,
				AngleInRadians,
				TOptional<FVector2D>(),
				FSlateDrawElement::RelativeToElement,
				Params.WireColor
			);
		}
	}
}

void FDialogueGraphConnectionDrawingPolicy::DrawPreviewConnector(
	const FGeometry& PinGeometry,
	const FVector2D& StartPoint,
	const FVector2D& EndPoint,
	UEdGraphPin* Pin
)
{
	FConnectionParams Params;
	DetermineWiringStyle(Pin, nullptr, /*inout*/ Params);

	// Find closesest point to each geometry, so that we draw from that source, simulates DrawSplineWithArrow
	if (Pin->Direction == EGPD_Output)
	{
		// From output pin, closest point on the SourceGeometry (source node) that goes to the EndPoint (destination node)
		DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, EndPoint), EndPoint, Params);
	}
	else
	{
		// From input pin, should never happen
		DrawSplineWithArrow(FGeometryHelper::FindClosestPointOnGeom(PinGeometry, StartPoint), StartPoint, Params);
	}
}

FVector2D FDialogueGraphConnectionDrawingPolicy::ComputeSplineTangent(const FVector2D& Start, const FVector2D& End) const
{
	// Draw a straight line
	const FVector2D Delta = End - Start;
	const FVector2D NormDelta = Delta.GetSafeNormal();
	return NormDelta;
}

void FDialogueGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
	FArrangedChildren& ArrangedNodes)
{
	// Build an acceleration structure to quickly find geometry for the nodes
	NodeWidgetMap.Empty();
	for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex)
	{
		FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
		TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
		NodeWidgetMap.Add(ChildNode->GetNodeObj(), NodeIndex);
	}

	// Now draw
	Super::Draw(InPinGeometries, ArrangedNodes);
}

void FDialogueGraphConnectionDrawingPolicy::Internal_DrawLineWithArrow(
	const FVector2D& StartAnchorPoint,
	const FVector2D& EndAnchorPoint,
	const FConnectionParams& Params
)
{
	constexpr float LineSeparationAmount = 4.5f;

	const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
	const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	// Come up with the final start/end points
	const FVector2D DirectionBias = Normal * LineSeparationAmount;
	const FVector2D LengthBias = ArrowRadius.X * UnitDelta;
	const FVector2D StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
	const FVector2D EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

	// Draw a line/spline
	DrawConnection(WireLayerID, StartPoint, EndPoint, Params);

	// Draw the arrow
	if (ArrowImage)
	{
		const FVector2D ArrowDrawPos = EndPoint - ArrowRadius;
		const float AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

		FSlateDrawElement::MakeRotatedBox(
			DrawElementsList,
			ArrowLayerID,
			FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
			ArrowImage,
			ESlateDrawEffect::None,
			AngleInRadians,
			TOptional<FVector2D>(),
			FSlateDrawElement::RelativeToElement,
			Params.WireColor
		);
	}
}
