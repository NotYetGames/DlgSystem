// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "ConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction.h"

#include "ScopedTransaction.h"

#include "NewNode_DialogueGraphSchemaAction.h"
#include "DlgDialogue.h"
#include "DialogueEditorUtilities.h"
#include "Nodes/DlgNode_SpeechSequence.h"
#include "Nodes/DlgNode_Speech.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"

#define LOCTEXT_NAMESPACE "ConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction
UEdGraphNode* FConvertSpeechSequenceNodeToSpeechNodes_DialogueGraphSchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode/* = false*/)
{
	check(SelectedSpeechSequenceGraphNode);
	check(SelectedSpeechSequenceGraphNode->IsSpeechSequenceNode());
	const FScopedTransaction Transaction(LOCTEXT("DialogueEditoConvertSpeechSequenceNodeToSpeechNodes", "Dialogue Editor: Convert Speech Sequence to Speech Nodes"));

	UDlgDialogue* Dialogue = FDialogueEditorUtilities::GetDialogueForGraph(ParentGraph);
	const UEdGraphSchema* GraphSchema = ParentGraph->GetSchema();
	const FVector2D PositionOffset(0.f, GetDefault<UDlgSystemSettings>()->OffsetBetweenRowsY);
	FVector2D Position = Location;

	const UDlgNode_SpeechSequence& SpeechSequence_DialogueNode = SelectedSpeechSequenceGraphNode->GetDialogueNode<UDlgNode_SpeechSequence>();
	const TArray<FDlgSpeechSequenceEntry>& SpeechSequenceEntries = SpeechSequence_DialogueNode.GetNodeSpeechSequence();
	check(SpeechSequenceEntries.Num() > 0);

	// Disable compiling for optimization
	Dialogue->DisableCompileDialogue();

	// Step 1. Create and position the final speech nodes array
	TArray<UDialogueGraphNode*> SpeechNodes;
	SpeechNodes.Reserve(SpeechSequenceEntries.Num());
	const int32 NodesNum = SpeechSequenceEntries.Num();
	for (int32 NodeIndex = 0; NodeIndex < NodesNum; NodeIndex++)
	{
		UDialogueGraphNode* Speech_GraphNode =
			FNewNode_DialogueGraphSchemaAction::SpawnGraphNodeWithDialogueNodeFromTemplate<UDialogueGraphNode>(
				ParentGraph, UDlgNode_Speech::StaticClass(), Position, bSelectNewNode);
		SpeechNodes.Add(Speech_GraphNode);

		// Advance down
		Position += PositionOffset;
	}
	check(SpeechNodes.Num() == SpeechSequenceEntries.Num());

	// Step 2. Link the nodes to each other and copy the data from the speech sequence
	for (int32 NodeIndex = 0; NodeIndex < NodesNum; NodeIndex++)
	{
		const FDlgSpeechSequenceEntry& SequenceEntry = SpeechSequenceEntries[NodeIndex];
		UDialogueGraphNode* Speech_GraphNode = SpeechNodes[NodeIndex];
		UDlgNode_Speech* Speech_DialogueNode = Speech_GraphNode->GetMutableDialogueNode<UDlgNode_Speech>();

		// Copy data
		Speech_DialogueNode->SetNodeParticipantName(SequenceEntry.Speaker);
		Speech_DialogueNode->SetNodeText(SequenceEntry.Text);
		Speech_DialogueNode->SetNodeData(SequenceEntry.NodeData);
		Speech_DialogueNode->SetSpeakerState(SequenceEntry.SpeakerState);
		Speech_DialogueNode->SetVoiceSoundBase(SequenceEntry.VoiceSoundWave);
		Speech_DialogueNode->SetVoiceDialogueWave(SequenceEntry.VoiceDialogueWave);
		Speech_DialogueNode->SetGenericData(SequenceEntry.GenericData);

		// Create edge to next node
		if (NodeIndex + 1 < NodesNum)
		{
			// Connect to next node
			UDialogueGraphNode* Speech_NextGraphNode = SpeechNodes[NodeIndex + 1];
			verify(GraphSchema->TryCreateConnection(Speech_GraphNode->GetOutputPin(), Speech_NextGraphNode->GetInputPin()));

			// Set the edge in the Dialogue. Should only be an edge/child
			Speech_GraphNode->SetEdgeTextAt(0, SequenceEntry.EdgeText);
		}
	}

	// Step 3. Copy existing connection to the first and last node.
	// Replace connections from parents
	const UDialogueGraphNode* FirstNode = SpeechNodes[0];
	FDialogueEditorUtilities::ReplaceParentConnectionsToNode(SelectedSpeechSequenceGraphNode, FirstNode);

	// Copy connections to the last Node
	UDialogueGraphNode* LastNode = SpeechNodes.Last();
	FDialogueEditorUtilities::CopyNodeChildren(SelectedSpeechSequenceGraphNode, LastNode);

	// Step 4. Remove the speech sequence node
	FDialogueEditorUtilities::RemoveNode(SelectedSpeechSequenceGraphNode);

#if DO_CHECK
	for (UDialogueGraphNode* ParentNode : SpeechNodes[0]->GetParentNodes())
	{
		ParentNode->CheckAll();
	}
	for (const UDialogueGraphNode* GraphNode : SpeechNodes)
	{
		GraphNode->CheckAll();
	}
	for (UDialogueGraphNode* ChildNode : SpeechNodes.Last()->GetChildNodes())
	{
		ChildNode->CheckAll();
	}
#endif

	Dialogue->EnableCompileDialogue();
	Dialogue->CompileDialogueNodesFromGraphNodes();
	Dialogue->PostEditChange();
	Dialogue->MarkPackageDirty();
	ParentGraph->NotifyGraphChanged();

	// Return the reference to the first node
	return SpeechNodes[0];
}

#undef LOCTEXT_NAMESPACE
