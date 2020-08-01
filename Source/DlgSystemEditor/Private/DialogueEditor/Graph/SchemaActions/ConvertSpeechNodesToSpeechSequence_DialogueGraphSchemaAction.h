// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "ConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction.generated.h"

class UDialogueGraphNode;
class UEdGraph;

/**
 * Action to convert Speech nodes to a Speech Sequence Node.
 * This is the opposite of the FConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction.
 */
USTRUCT()
struct FConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction() : FEdGraphSchemaAction() {}
	FConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction(const TArray<UDialogueGraphNode*>& InSelectedGraphNodes)
        : FEdGraphSchemaAction(), SelectedGraphNodes(InSelectedGraphNodes) {}

	FConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction(
		const FText& InNodeCategory,
		const FText& InMenuDesc,
		const FText& InToolTip,
		int32 InGrouping,
		TArray<UDialogueGraphNode*> InSelectedGraphNodes
	) : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), SelectedGraphNodes(InSelectedGraphNodes) {}

	//~ Begin FEdGraphSchemaAction Interface
	UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
	//~ End FEdGraphSchemaAction Interface

private:
	// Nodes selected that we are going to convert
	TArray<UDialogueGraphNode*> SelectedGraphNodes;
};
