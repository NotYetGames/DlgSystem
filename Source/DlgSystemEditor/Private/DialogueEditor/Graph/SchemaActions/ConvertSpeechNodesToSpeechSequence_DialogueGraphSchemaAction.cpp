// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "ConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction.h"

#include "ScopedTransaction.h"

#include "NewNode_DialogueGraphSchemaAction.h"
#include "DlgDialogue.h"
#include "DialogueEditorUtilities.h"
#include "Nodes/DlgNode_SpeechSequence.h"
#include "Nodes/DlgNode_Speech.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"

#define LOCTEXT_NAMESPACE "ConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction
UEdGraphNode* FConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	const FVector2D Location, bool bSelectNewNode/* = true*/)
{
	// Should have been stopped in GetConvertActions
	check(SelectedGraphNodes.Num() > 0);
	const FScopedTransaction Transaction(LOCTEXT("DialogueditorConvertSpeechNodesToSpeechSequence", "Convert Speech Nodes to a Sequence Node"));
	UDlgDialogue* Dialogue = FDialogueEditorUtilities::GetDialogueForGraph(ParentGraph);

	// Disable compiling for optimization
	Dialogue->DisableCompileDialogue();

	// Step 1. Create the final speech sequence
	UDialogueGraphNode* GraphNode_SpeechSequence = FNewNode_DialogueGraphSchemaAction::SpawnGraphNodeWithDialogueNodeFromTemplate<UDialogueGraphNode>(
			ParentGraph, UDlgNode_SpeechSequence::StaticClass(), Location, bSelectNewNode);

	// Step 2. Copy all selected graph nodes into the speech sequence
	UDlgNode_SpeechSequence* DialogueNode_SpeechSequence =
		GraphNode_SpeechSequence->GetMutableDialogueNode<UDlgNode_SpeechSequence>();
	TArray<FDlgSpeechSequenceEntry>* SpeechSequence = DialogueNode_SpeechSequence->GetMutableNodeSpeechSequence();
	for (UDialogueGraphNode* GraphNode : SelectedGraphNodes)
	{
		const UDlgNode_Speech& DialogueNode_Speech = GraphNode->GetDialogueNode<UDlgNode_Speech>();
		auto SequenceEntry = FDlgSpeechSequenceEntry{};
		SequenceEntry.Speaker = DialogueNode_Speech.GetNodeParticipantName();
		SequenceEntry.Text = DialogueNode_Speech.GetNodeText();
		SequenceEntry.NodeData = DialogueNode_Speech.GetNodeData();
		SequenceEntry.SpeakerState = DialogueNode_Speech.GetSpeakerState();
		SequenceEntry.VoiceSoundWave = DialogueNode_Speech.GetNodeVoiceSoundBase();
		SequenceEntry.VoiceDialogueWave = DialogueNode_Speech.GetNodeVoiceDialogueWave();
		SequenceEntry.GenericData = DialogueNode_Speech.GetNodeGenericData();

		// Set edge if any
		const TArray<FDlgEdge>& Children = DialogueNode_Speech.GetNodeChildren();
		// Get only the first edge, as there should be only one anyways (besides the last node)
		if (Children.Num() > 0)
		{
			SequenceEntry.EdgeText = Children[0].GetUnformattedText();
		}

		SpeechSequence->Add(SequenceEntry);
	}
	DialogueNode_SpeechSequence->AutoGenerateInnerEdges();

	// Step 3. Copy existing connections from the first and last node in the sequence
	// Replace connections from parents
	const UDialogueGraphNode* FirstSelectedGraphNode = SelectedGraphNodes[0];
	FDialogueEditorUtilities::ReplaceParentConnectionsToNode(FirstSelectedGraphNode, GraphNode_SpeechSequence);

	// Copy connections from the last node to the speech sequence
	const UDialogueGraphNode* LastSelectedNode = SelectedGraphNodes.Last();
	FDialogueEditorUtilities::CopyNodeChildren(LastSelectedNode, GraphNode_SpeechSequence);

	// Step 4. Remove the nodes, this will break the existing input/output connections of the nodes
	for (UDialogueGraphNode* GraphNode : SelectedGraphNodes)
		FDialogueEditorUtilities::RemoveNode(GraphNode);

#if DO_CHECK
	GraphNode_SpeechSequence->CheckAll();
	for (UDialogueGraphNode* ParentNode : GraphNode_SpeechSequence->GetParentNodes())
	{
		ParentNode->CheckAll();
	}
	for (UDialogueGraphNode* ChildNode : GraphNode_SpeechSequence->GetChildNodes())
	{
		ChildNode->CheckAll();
	}
#endif
	Dialogue->EnableCompileDialogue();
	Dialogue->CompileDialogueNodesFromGraphNodes();
	Dialogue->PostEditChange();
	Dialogue->MarkPackageDirty();
	ParentGraph->NotifyGraphChanged();

	return GraphNode_SpeechSequence;
}

#undef LOCTEXT_NAMESPACE
