// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgNewComment_GraphSchemaAction.h"

#include "EdGraphNode_Comment.h"

#include "DlgSystemEditor/DlgEditorUtilities.h"

#define LOCTEXT_NAMESPACE "NewComment_DialogueGraphSchemaAction"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgNewComment_GraphSchemaAction
UEdGraphNode* FDlgNewComment_GraphSchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode/* = true*/)
{
	// Add menu item for creating comment boxes
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	// Wrap comment around other nodes, this makes it possible to select other nodes and press the "C" key on the keyboard.
	FVector2D SpawnLocation = Location;
	FSlateRect Bounds;
	if (FDlgEditorUtilities::GetBoundsForSelectedNodes(ParentGraph, Bounds, 50.0f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation, bSelectNewNode);
}

#undef LOCTEXT_NAMESPACE
