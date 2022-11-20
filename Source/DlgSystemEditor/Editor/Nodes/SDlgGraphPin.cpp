// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "SDlgGraphPin.h"

#include "SGraphPanel.h"
#include "SGraphNode.h"
#include "Math/UnitConversion.h"
#include "GraphEditorDragDropAction.h"
#include "DragAndDrop/AssetDragDropOp.h"

#include "DialogueGraphNode_Edge.h"
#include "DialogueGraphNode.h"


#define LOCTEXT_NAMESPACE "SDlgGraphPin"

/////////////////////////////////////////////////////
// SDlgGraphPin
void SDlgGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	check(InPin);
	SetCursor(EMouseCursor::Default);
	bShowLabel = true;
	GraphPinObj = InPin;

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &Self::GetPinBorder)
		.BorderBackgroundColor(this, &Self::GetPinColor)
		.OnMouseButtonDown(this, &Self::OnPinMouseDown)
		.Cursor(this, &Self::GetPinCursor)
		.Padding(FMargin(10.f))
	);

	TAttribute<FText> ToolTipAttribute = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &Self::GetTooltipText));
	SetToolTipText(ToolTipAttribute);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin SGraphPin Interface
FReply SDlgGraphPin::OnPinMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
	bIsMovingLinks = false;
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && IsEditingEnabled())
	{
		if (GraphPinObj->LinkedTo.Num() > 0)
		{
			if (MouseEvent.IsAltDown())
			{
				return OnAltAndLeftMouseButtonDown(SenderGeometry, MouseEvent);
			}

			if (MouseEvent.IsControlDown())
			{
				return OnCtrlAndLeftMouseButtonDown(SenderGeometry, MouseEvent);
			}
		}

		// Regular drag operation, to create new links
		if (GetIsConnectable())
		{
			// Start a drag-drop on the pin
			TSharedPtr<SGraphNode> ThisOwnerNodeWidget = OwnerNodePtr.Pin();
			if (ensure(ThisOwnerNodeWidget.IsValid()))
			{
				TArray<TSharedRef<SGraphPin>> DragPinArray;
				DragPinArray.Add(SharedThis(this));
				return FReply::Handled().BeginDragDrop(SpawnPinDragEvent(ThisOwnerNodeWidget->GetOwnerPanel().ToSharedRef(), DragPinArray));
			}

			return FReply::Unhandled();
		}

		// It's not connectible, but we don't want anything above us to process this left click.
		return FReply::Handled();
	}
	return FReply::Unhandled();
}
// End SGraphPin Interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin SWidget interface
FReply SDlgGraphPin::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Unhandled();
}

FReply SDlgGraphPin::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return Super::OnMouseMove(MyGeometry, MouseEvent);
}

void SDlgGraphPin::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
#if NY_ENGINE_VERSION >= 500
	const bool bLocalIsHovered = IsHovered();
#else
	const bool bLocalIsHovered = bIsHovered;
#endif

	if (!bLocalIsHovered && GraphPinObj && !GraphPinObj->IsPendingKill() && GraphPinObj->GetOuter())
	{
		TSharedPtr<SGraphPanel> OwnerPanelPtr = OwnerNodePtr.Pin()->GetOwnerPanel();
		check(OwnerPanelPtr.IsValid());

		// Will be removed in Super::OnMouseLeave
		// Add current pin
		HoverPinSet.Add(GraphPinObj);
		OwnerPanelPtr->AddPinToHoverSet(GraphPinObj);

		if (GraphPinObj->LinkedTo.Num() > 0)
		{
			// Pin belongs to an actual node
			if (UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(GraphPinObj->GetOwningNode()))
			{
				if (GraphPinObj->Direction == EGPD_Output)
				{
					// output pin mark all
					for (UDialogueGraphNode* ChildNode : GraphNode->GetChildNodes())
					{
						HoverPinSet.Add(ChildNode->GetInputPin());
						OwnerPanelPtr->AddPinToHoverSet(ChildNode->GetInputPin());
					}
				}
				else
				{
					// input pin
				}
			}
		}
	}

	SCompoundWidget::OnMouseEnter(MyGeometry, MouseEvent);
}

void SDlgGraphPin::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::OnMouseLeave(MouseEvent);
}

//
// DRAG AND DROP (DragDrop)
// Used by the TargetPin
//
void SDlgGraphPin::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	// NOTE: Super default implementation will not work properly with nodes that are not GetIsConnectable
#if NY_ENGINE_VERSION < 500
	Super::OnDragEnter(MyGeometry, DragDropEvent);
#else
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid())
	{
		return;
	}

	// Is someone dragging a connection?
	if (Operation->IsOfType<FGraphEditorDragDropAction>())
	{
		// Ensure that the pin is valid before using it
		if(GraphPinObj != NULL && !GraphPinObj->IsPendingKill() && GraphPinObj->GetOuter() != NULL && GraphPinObj->GetOuter()->IsA(UEdGraphNode::StaticClass()))
		{
			// Inform the Drag and Drop operation that we are hovering over this pin.
			TSharedPtr<FGraphEditorDragDropAction> DragConnectionOp = StaticCastSharedPtr<FGraphEditorDragDropAction>(Operation);
			DragConnectionOp->SetHoveredPin(GraphPinObj);
		}

		// Pins treat being dragged over the same as being hovered outside of drag and drop if they know how to respond to the drag action.
		SBorder::OnMouseEnter(MyGeometry, DragDropEvent);
	}
#endif  // NY_ENGINE_VERSION < 500
}

void SDlgGraphPin::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	// NOTE: Super default implementation will not work properly with nodes that are not GetIsConnectable
#if NY_ENGINE_VERSION < 500
	Super::OnDragLeave(DragDropEvent);
#else
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid())
	{
		return;
	}

	// Is someone dragging a connection?
	if (Operation->IsOfType<FGraphEditorDragDropAction>())
	{
		// Inform the Drag and Drop operation that we are not hovering any pins
		TSharedPtr<FGraphEditorDragDropAction> DragConnectionOp = StaticCastSharedPtr<FGraphEditorDragDropAction>(Operation);
		DragConnectionOp->SetHoveredPin(nullptr);

		SBorder::OnMouseLeave(DragDropEvent);
	}

	else if (Operation->IsOfType<FAssetDragDropOp>())
	{
		TSharedPtr<FAssetDragDropOp> AssetOp = StaticCastSharedPtr<FAssetDragDropOp>(Operation);
		AssetOp->ResetToDefaultToolTip();
	}
#endif // NY_ENGINE_VERSION < 500
}

FReply SDlgGraphPin::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	return Super::OnDragOver(MyGeometry, DragDropEvent);
}

FReply SDlgGraphPin::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	// NOTE: Super default implementation will not work properly with nodes that are not GetIsConnectable
#if NY_ENGINE_VERSION < 500
	return Super::OnDrop(MyGeometry, DragDropEvent);
#else
	TSharedPtr<SGraphNode> NodeWidget = OwnerNodePtr.Pin();
	bool bReadOnly = NodeWidget.IsValid() ? !NodeWidget->IsNodeEditable() : false;

	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid() || bReadOnly)
	{
		return FReply::Unhandled();
	}

	// Is someone dropping a connection onto this pin?
	if (Operation->IsOfType<FGraphEditorDragDropAction>())
	{
		TSharedPtr<FGraphEditorDragDropAction> DragConnectionOp = StaticCastSharedPtr<FGraphEditorDragDropAction>(Operation);

		FVector2D NodeAddPosition = FVector2D::ZeroVector;
		TSharedPtr<SGraphNode> OwnerNode = OwnerNodePtr.Pin();
		if (OwnerNode.IsValid())
		{
			NodeAddPosition	= OwnerNode->GetPosition() + FVector2D(MyGeometry.Position);

			//Don't have access to bounding information for node, using fixed offset that should work for most cases.
			const float FixedOffset = 200.0f;

			//Line it up vertically with pin
			NodeAddPosition.Y += MyGeometry.Size.Y;

			// if the pin widget is nested into another compound
			if (MyGeometry.Position == FVector2f::ZeroVector)
			{
				FVector2D PinOffsetPosition = FVector2D(MyGeometry.AbsolutePosition) - FVector2D(NodeWidget->GetTickSpaceGeometry().AbsolutePosition);
				NodeAddPosition = OwnerNode->GetPosition() + PinOffsetPosition;
			}

			if(GetDirection() == EEdGraphPinDirection::EGPD_Input)
			{
				//left side just offset by fixed amount
				//@TODO: knowing the width of the node we are about to create would allow us to line this up more precisely,
				//       but this information is not available currently
				NodeAddPosition.X -= FixedOffset;
			}
			else
			{
				//right side we need the width of the pin + fixed amount because our reference position is the upper left corner of pin(which is variable length)
				NodeAddPosition.X += MyGeometry.Size.X + FixedOffset;
			}

		}

		return DragConnectionOp->DroppedOnPin(DragDropEvent.GetScreenSpacePosition(), NodeAddPosition);
	}
	// handle dropping an asset on the pin
	else if (Operation->IsOfType<FAssetDragDropOp>() && NodeWidget.IsValid())
	{
		UEdGraphNode* Node = NodeWidget->GetNodeObj();
		if(Node != NULL && Node->GetSchema() != NULL)
		{
			TSharedPtr<FAssetDragDropOp> AssetOp = StaticCastSharedPtr<FAssetDragDropOp>(Operation);
			if (AssetOp->HasAssets())
			{
				Node->GetSchema()->DroppedAssetsOnPin(AssetOp->GetAssets(), NodeWidget->GetPosition() + FVector2D(MyGeometry.Position), GraphPinObj);
			}
		}
		return FReply::Handled();
	}

	return FReply::Unhandled();
#endif // NY_ENGINE_VERSION < 500
}

FSlateColor SDlgGraphPin::GetPinColor() const
{
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	const FSlateColor& DefaultPinColor = IsHovered() ? Settings->BorderHoveredBackgroundColor : Settings->BorderBackgroundColor;
	if (GraphPinObj == nullptr)
	{
		return DefaultPinColor;
	}

	if (UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(GraphPinObj->GetOwningNode()))
	{
		if (!GraphNode->CanHaveOutputConnections())
		{
			return IsHovered() ? Settings->BorderHoveredBackgroundColorNoChildren : Settings->BorderBackgroundColorNoChildren;
		}

		if (GraphNode->ShouldUseBorderHighlight())
		{
			return Settings->BorderBackgroundColorHighlighted;
		}
	}

	return DefaultPinColor;
}
// End SWidget interface
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Begin own functions
FReply SDlgGraphPin::OnAltAndLeftMouseButtonDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
	// Alt-Left clicking will break that pin, if alt clicking on the wire, otherwise it will break all pins
	TSharedPtr<SGraphNode> ThisOwnerNodeWidget = OwnerNodePtr.Pin();
	check(ThisOwnerNodeWidget.IsValid());
	const UDialogueGraphSchema* Schema = CastChecked<UDialogueGraphSchema>(GraphPinObj->GetSchema());
	const FVector2D& MouseLocation = MouseEvent.GetScreenSpacePosition();

	// Is the click inside the node?
	if (FDlgEditorUtilities::IsPointInsideGeometry(MouseLocation, ThisOwnerNodeWidget->GetCachedGeometry()))
	{
		// break all links
		Schema->BreakPinLinks(*GraphPinObj, true);
	}
	else
	{
		// Find the appropriate pin, if there are multiple children
		UEdGraphPin* ToPin = GraphPinObj->LinkedTo[0];
		if (GraphPinObj->LinkedTo.Num() > 1)
		{
			ToPin = GetBestLinkedToPinFromSplineMousePosition(MouseLocation);
		}

		Schema->BreakLinkTo(GraphPinObj, ToPin, true);
	}

	return FReply::Handled();
}

FReply SDlgGraphPin::OnCtrlAndLeftMouseButtonDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
	// Control-Left clicking will break all existing connections to a pin
	// Note: that for some nodes, this can cause reconstruction. In that case, pins we had previously linked to may now be destroyed.
	//	   So the break MUST come after the SpawnPinDragEvent(), since that acquires handles from PinArray (the pins need to be
	//	   around for us to construct valid handles from).
	TSharedPtr<SGraphNode> ThisOwnerNodeWidget = OwnerNodePtr.Pin();
	check(ThisOwnerNodeWidget.IsValid());
	TSharedPtr<SGraphPanel> OwnerPanelPtr = ThisOwnerNodeWidget->GetOwnerPanel();
	check(OwnerPanelPtr.IsValid());

	TArray<TSharedRef<SGraphPin>> DragPinArray;
	DragPinArray.Add(SharedThis(this));

	// Find the appropriate pin, if there are multiple children
	UEdGraphPin* ToPin = GraphPinObj->LinkedTo[0];
	if (GraphPinObj->LinkedTo.Num() > 1)
	{
		ToPin = GetBestLinkedToPinFromSplineMousePosition(MouseEvent.GetScreenSpacePosition());
	}

	// Start drag operation
	// Kill the current connection
	if (UDialogueGraphNode_Edge* GraphEdge = Cast<UDialogueGraphNode_Edge>(ToPin->GetOwningNode()))
	{
		// true almost all the times, but just to be sure in the future
		FDlgEditorUtilities::SetLastTargetGraphEdgeBeforeDrag(GraphEdge->GetGraph(), GraphEdge);
	}

	const UDialogueGraphSchema* Schema = CastChecked<UDialogueGraphSchema>(GraphPinObj->GetSchema());
	Schema->BreakLinkTo(GraphPinObj, ToPin, true);

	bIsMovingLinks = true;
	// Handled by SGraphPanel::OnDrop
	return FReply::Handled().BeginDragDrop(SpawnPinDragEvent(ThisOwnerNodeWidget->GetOwnerPanel().ToSharedRef(), DragPinArray));
}

FText SDlgGraphPin::GetTooltipText() const
{
	FText HoverText = FText::GetEmpty();
	return HoverText;
	// TODO find a way to only show this when hovering over the wire but not over the node
//	const UEdGraphNode* GraphNode = GraphPinObj && !GraphPinObj->IsPendingKill() ? GraphPinObj->GetOwningNodeUnchecked() : nullptr;
//	if (GraphNode != nullptr)
//	{
//		FString HoverStr;
//		GraphNode->GetPinHoverText(*GraphPinObj, /*out*/HoverStr);
//		if (!HoverStr.IsEmpty())
//		{
//			HoverText = FText::FromString(HoverStr);
//		}
//	}
//
//	return HoverText;
}

UEdGraphPin* SDlgGraphPin::GetBestLinkedToPinFromSplineMousePosition(const FVector2D& MousePosition) const
{
	/*
	ASSUMPTION: that the MousePosition is on a spline (wire) or near a wire
	Notation:
		M - The position of the mouse Vector (aka MousePosition)
		P - ThisGraphNode Vector location (aka ThisGraphNodeClosestPosition)
		C - GraphNode child location (aka ClosestPointOnChild)

	The (GraphEdge, GraphNode child) pair with the angle(MC, MP) closest to 180 degrees corresponds to this MousePosition.
	Basically this tries to find the edge that has MousePosition on its spline/wire.

	Crude illustration:
         ____                   ____
        |   |         E1        |   |
        | P |----M------------->| C |
        |___|                   |___|
          |
          | E2
          |
         _V__
        |   |
        | D |
        |___|
	*/

	const UDialogueGraphNode* ThisGraphNode = CastChecked<UDialogueGraphNode>(GraphPinObj->GetOwningNode());
	TSharedPtr<SGraphNode> ThisGraphNodeWidget = OwnerNodePtr.Pin();
	check(ThisGraphNodeWidget.IsValid());

	// Find P and MP
	const FVector2D ThisGraphNodeClosestPosition = FGeometryHelper::FindClosestPointOnGeom(
		ThisGraphNodeWidget->GetCachedGeometry(),
		MousePosition
	);
	const FVector2D MP = (ThisGraphNodeClosestPosition - MousePosition).GetSafeNormal();

	// Iterate over all edges, find the best one
	const TArray<UDialogueGraphNode_Edge*> ChildGraphEdges = ThisGraphNode->GetChildEdgeNodes();
	int32 BestEdgeIndex = 0;

	// Dot product is always positive here, because we do not care about direction
	float BestDotProduct = 0.0f;
	const int32 ChildNum = ChildGraphEdges.Num();
	for (int32 ChildIndex = 0; ChildIndex < ChildNum; ChildIndex++)
	{
		UDialogueGraphNode_Edge* ChildEdge = ChildGraphEdges[ChildIndex];
		TSharedPtr<SGraphNode> ChildNodeWidget = ChildEdge->GetChildNode()->GetNodeWidget();
		check(ChildNodeWidget.IsValid());

		// Find C (aka ClosestPointOnChild)
		const FVector2D ClosestPointOnChild = FGeometryHelper::FindClosestPointOnGeom(ChildNodeWidget->GetCachedGeometry(), MousePosition);

		// Find angle between vectors
		const FVector2D MC = (ClosestPointOnChild - MousePosition).GetSafeNormal();
		const float DotProduct = FMath::Abs(FVector2D::DotProduct(MC, MP));
		if (DotProduct > BestDotProduct)
		{
			// found new angle approaching 180 degrees
			BestDotProduct = DotProduct;
			BestEdgeIndex = ChildIndex;
		}
	}

	return GraphPinObj->LinkedTo[BestEdgeIndex];;
}
// End own functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#undef LOCTEXT_NAMESPACE
