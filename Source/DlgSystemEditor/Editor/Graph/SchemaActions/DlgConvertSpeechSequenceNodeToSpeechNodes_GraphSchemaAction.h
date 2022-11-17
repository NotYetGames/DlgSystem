// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "DlgConvertSpeechSequenceNodeToSpeechNodes_GraphSchemaAction.generated.h"

class UDialogueGraphNode;
class UEdGraph;

/**
 * Action to convert a Speech Sequence node to a list of Speech nodes.
 * This is the opposite of the FDlgConvertSpeechNodesToSpeechSequence_GraphSchemaAction.
 */
USTRUCT()
struct DLGSYSTEMEDITOR_API FDlgConvertSpeechSequenceNodeToSpeechNodes_GraphSchemaAction : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	FDlgConvertSpeechSequenceNodeToSpeechNodes_GraphSchemaAction() : FEdGraphSchemaAction() {}
	FDlgConvertSpeechSequenceNodeToSpeechNodes_GraphSchemaAction(UDialogueGraphNode* InSelectedSpeechSequenceGraphNode)
		: FEdGraphSchemaAction(), SelectedSpeechSequenceGraphNode(InSelectedSpeechSequenceGraphNode) {}

	FDlgConvertSpeechSequenceNodeToSpeechNodes_GraphSchemaAction(
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
