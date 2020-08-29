// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "ConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction.generated.h"

class UDialogueGraphNode;
class UEdGraph;

/**
 * Action to convert a Speech Sequence node to a list of Speech nodes.
 * This is the opposite of the FConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction.
 */
USTRUCT()
struct FConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction() : FEdGraphSchemaAction() {}
	FConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction(UDialogueGraphNode* InSelectedSpeechSequenceGraphNode)
		: FEdGraphSchemaAction(), SelectedSpeechSequenceGraphNode(InSelectedSpeechSequenceGraphNode) {}

	FConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction(
		const FText& InNodeCategory,
		const FText& InMenuDesc,
		const FText& InToolTip,
		int32 InGrouping,
		UDialogueGraphNode* InSelectedSpeechSequenceGraphNode
	) : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), SelectedSpeechSequenceGraphNode(InSelectedSpeechSequenceGraphNode) {}

	//~ Begin FEdGraphSchemaAction Interface
	UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = false) override;
	//~ End FEdGraphSchemaAction Interface

private:
	// The speech sequence selected
	UDialogueGraphNode* SelectedSpeechSequenceGraphNode = nullptr;
};
