// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "NewComment_DialogueGraphSchemaAction.h"

#include "EdGraphNode_Comment.h"

#include "DialogueEditorUtilities.h"

#define LOCTEXT_NAMESPACE "NewComment_DialogueGraphSchemaAction"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FNewComment_DialogueGraphSchemaAction
UEdGraphNode* FNewComment_DialogueGraphSchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode/* = true*/)
{
	// Add menu item for creating comment boxes
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	// Wrap comment around other nodes, this makes it possible to select other nodes and press the "C" key on the keyboard.
	FVector2D SpawnLocation = Location;
	FSlateRect Bounds;
	if (FDialogueEditorUtilities::GetBoundsForSelectedNodes(ParentGraph, Bounds, 50.0f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation, bSelectNewNode);
}

#undef LOCTEXT_NAMESPACE
