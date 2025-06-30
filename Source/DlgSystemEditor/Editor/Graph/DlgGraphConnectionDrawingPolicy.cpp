// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgGraphConnectionDrawingPolicy.h"

#include "Rendering/DrawElements.h"

#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode_Edge.h"
#include "DlgSystemEditor/Editor/Nodes/SDlgGraphNode_Edge.h"

/////////////////////////////////////////////////////
// FDlgGraphConnectionDrawingPolicy
FDlgGraphConnectionDrawingPolicy::FDlgGraphConnectionDrawingPolicy(
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

void FDlgGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
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

void FDlgGraphConnectionDrawingPolicy::DetermineLinkGeometry(
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

void FDlgGraphConnectionDrawingPolicy::DrawSplineWithArrow(
	const FGeometry& StartGeom,
	const FGeometry& EndGeom,
	const FConnectionParams& Params
)
{
	// Draw the spline and arrow from/to the closest points between the two geometries (nodes)
	// Get a reasonable seed point (halfway between the boxes)
	const FNYVector2f StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FNYVector2f EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FNYVector2f SeedPoint = (StartCenter + EndCenter) / 2.0f;

	// Find the (approximate) closest points between the two boxes
	const FNYVector2f StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FNYVector2f EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

void FDlgGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FNYVector2f& StartPoint, const FNYVector2f& EndPoint,
	const FConnectionParams& Params)
{
	Internal_DrawLineWithArrow(StartPoint, EndPoint, Params);
	// Is the connection bidirectional?
	if (Params.bUserFlag1)
	{
		Internal_DrawLineWithArrow(EndPoint, StartPoint, Params);
	}
}

void FDlgGraphConnectionDrawingPolicy::DrawConnection(
	int32 LayerId,
	const FNYVector2f& Start,
	const FNYVector2f& End,
	const FConnectionParams& Params
)
{
	// Code mostly from Super::DrawConnection
	const FNYVector2f& P0 = Start;
	const FNYVector2f& P1 = End;

	const FNYVector2f SplineTangent = ComputeSplineTangent(P0, P1);
	const FNYVector2f P0Tangent = Params.StartDirection == EGPD_Output ? SplineTangent : -SplineTangent;
	const FNYVector2f P1Tangent = Params.EndDirection == EGPD_Input ? SplineTangent : -SplineTangent;

	if (Settings->bTreatSplinesLikePins)
	{
		// Distance to consider as an overlap
		const float QueryDistanceTriggerThresholdSquared = FMath::Square(Settings->SplineHoverTolerance + Params.WireThickness * 0.5f);

#if NY_ENGINE_VERSION >= 500
		// Distance to pass the bounding box cull test. This is used for the bCloseToSpline output that can be used as a
		// dead zone to avoid mistakes caused by missing a double-click on a connection.
		const float QueryDistanceForCloseSquared = FMath::Square(FMath::Sqrt(QueryDistanceTriggerThresholdSquared) + Settings->SplineCloseTolerance);
#else
		// Distance to pass the bounding box cull test (may want to expand this later on if we want to do 'closest pin' actions that don't require an exact hit)
		const float QueryDistanceToBoundingBoxSquared = QueryDistanceTriggerThresholdSquared;
#endif

		bool bCloseToSpline = false;
		{
			// The curve will include the endpoints but can extend out of a tight bounds because of the tangents
			// P0Tangent coefficient maximizes to 4/27 at a=1/3, and P1Tangent minimizes to -4/27 at a=2/3.
			constexpr float MaximumTangentContribution = 4.0f / 27.0f;
			FNYBox2f Bounds(ForceInit);

			Bounds += FNYVector2f(P0);
			Bounds += FNYVector2f(P0 + MaximumTangentContribution * P0Tangent);
			Bounds += FNYVector2f(P1);
			Bounds += FNYVector2f(P1 - MaximumTangentContribution * P1Tangent);

#if NY_ENGINE_VERSION >= 500
			bCloseToSpline = Bounds.ComputeSquaredDistanceToPoint(LocalMousePosition) < QueryDistanceForCloseSquared;
#else
			bCloseToSpline = Bounds.ComputeSquaredDistanceToPoint(LocalMousePosition) < QueryDistanceToBoundingBoxSquared;
#endif
		}

		if (bCloseToSpline)
		{
			// Find the closest approach to the spline
			FNYVector2f ClosestPoint(ForceInit);
			float ClosestDistanceSquared = FLT_MAX;

			constexpr int32 NumStepsToTest = 16;
			constexpr float StepInterval = 1.0f / static_cast<float>(NumStepsToTest);
			FNYVector2f Point1 = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, 0.0f);
			for (float TestAlpha = 0.0f; TestAlpha < 1.0f; TestAlpha += StepInterval)
			{
				const FNYVector2f Point2 = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, TestAlpha + StepInterval);

				const FNYVector2f LocalMousePosition2D(LocalMousePosition.X, LocalMousePosition.Y);
				const FNYVector2f ClosestPointToSegment = FMath::ClosestPointOnSegment2D(LocalMousePosition2D, Point1, Point2);
				const float DistanceSquared = (LocalMousePosition2D - ClosestPointToSegment).SizeSquared();

				if (DistanceSquared < ClosestDistanceSquared)
				{
					ClosestDistanceSquared = DistanceSquared;
					ClosestPoint = ClosestPointToSegment;
				}

				Point1 = Point2;
			}

			// Record the overlap
#if NY_ENGINE_VERSION >= 500
			if (ClosestDistanceSquared < QueryDistanceTriggerThresholdSquared)
			{
				if (ClosestDistanceSquared < SplineOverlapResult.GetDistanceSquared())
				{
					const float SquaredDistToPin1 = (Params.AssociatedPin1 != nullptr) ? (P0 - ClosestPoint).SizeSquared() : FLT_MAX;
					const float SquaredDistToPin2 = (Params.AssociatedPin2 != nullptr) ? (P1 - ClosestPoint).SizeSquared() : FLT_MAX;

					SplineOverlapResult = FGraphSplineOverlapResult(Params.AssociatedPin1, Params.AssociatedPin2, ClosestDistanceSquared, SquaredDistToPin1, SquaredDistToPin2, true);
				}
			}
			else if (ClosestDistanceSquared < QueryDistanceForCloseSquared)
			{
				SplineOverlapResult.SetCloseToSpline(true);
			}
#else
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
					SplineOverlapResult = FGraphSplineOverlapResult(
						Params.AssociatedPin1, Params.AssociatedPin1,
						ClosestDistanceSquared,
						SquaredDistToPin1, SquaredDistToPin1);
				}
			}
#endif // NY_ENGINE_VERSION >= 500
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
			const FNYVector2f BubbleSize = BubbleImage->ImageSize * ZoomFactor * 0.1f * Params.WireThickness;

			const float Time = FPlatformTime::Seconds() - GStartTime;
			const float BubbleOffset = FMath::Fmod(Time * BubbleSpeed, BubbleSpacing);
			const int32 NumBubbles = FMath::CeilToInt(SplineLength / BubbleSpacing);
			for (int32 i = 0; i < NumBubbles; ++i)
			{
				const float Distance = (static_cast<float>(i) * BubbleSpacing) + BubbleOffset;
				if (Distance < SplineLength)
				{
					const float Alpha = SplineReparamTable.Eval(Distance, 0.f);
					FNYVector2f BubblePos = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, Alpha);
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
			const FNYVector2f Midpoint = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, MidpointAlpha);

			// Approximate the slope at the midpoint (to orient the midpoint image to the spline)
			const FNYVector2f MidpointPlusE = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, MidpointAlpha + KINDA_SMALL_NUMBER);
			const FNYVector2f MidpointMinusE = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, MidpointAlpha - KINDA_SMALL_NUMBER);
			const FNYVector2f SlopeUnnormalized = MidpointPlusE - MidpointMinusE;

			// Draw the arrow
			const FNYVector2f MidpointDrawPos = Midpoint - MidpointRadius;
			const float AngleInRadians = SlopeUnnormalized.IsNearlyZero() ? 0.0f : FMath::Atan2(SlopeUnnormalized.Y, SlopeUnnormalized.X);

			FSlateDrawElement::MakeRotatedBox(
				DrawElementsList,
				LayerId,
				FPaintGeometry(MidpointDrawPos, MidpointImage->ImageSize * ZoomFactor, ZoomFactor),
				MidpointImage,
				ESlateDrawEffect::None,
				AngleInRadians,
				TOptional<FNYVector2f>(),
				FSlateDrawElement::RelativeToElement,
				Params.WireColor
			);
		}
	}
}

void FDlgGraphConnectionDrawingPolicy::DrawPreviewConnector(
	const FGeometry& PinGeometry,
	const FNYVector2f& StartPoint,
	const FNYVector2f& EndPoint,
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

FNYVector2f FDlgGraphConnectionDrawingPolicy::ComputeSplineTangent(const FNYVector2f& Start, const FNYVector2f& End) const
{
	// Draw a straight line
	const FNYVector2f Delta = End - Start;
	const FNYVector2f NormDelta = Delta.GetSafeNormal();
	return NormDelta;
}

void FDlgGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
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

void FDlgGraphConnectionDrawingPolicy::Internal_DrawLineWithArrow(
	const FNYVector2f& StartAnchorPoint,
	const FNYVector2f& EndAnchorPoint,
	const FConnectionParams& Params)
{
	constexpr float LineSeparationAmount = 4.5f;

	const FNYVector2f DeltaPos = EndAnchorPoint - StartAnchorPoint;
	const FNYVector2f UnitDelta = DeltaPos.GetSafeNormal();
	const FNYVector2f Normal = FNYVector2f(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	// Come up with the final start/end points
	const FNYVector2f DirectionBias = Normal * LineSeparationAmount;
	const FNYVector2f LengthBias = ArrowRadius.X * UnitDelta;
	const FNYVector2f StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
	const FNYVector2f EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

	// Draw a line/spline
	DrawConnection(WireLayerID, StartPoint, EndPoint, Params);

	// Draw the arrow
	if (ArrowImage)
	{
		const FNYVector2f ArrowDrawPos = EndPoint - ArrowRadius;
		const float AngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

		FSlateDrawElement::MakeRotatedBox(
			DrawElementsList,
			ArrowLayerID,
			FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
			ArrowImage,
			ESlateDrawEffect::None,
			AngleInRadians,
			TOptional<FNYVector2f>(),
			FSlateDrawElement::RelativeToElement,
			Params.WireColor
		);
	}
}
