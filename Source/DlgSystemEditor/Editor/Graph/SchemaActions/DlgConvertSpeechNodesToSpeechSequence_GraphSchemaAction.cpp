// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgConvertSpeechNodesToSpeechSequence_GraphSchemaAction.h"

#include "ScopedTransaction.h"

#include "DlgNewNode_GraphSchemaAction.h"
#include "DlgSystem/DlgDialogue.h"
#include "DlgSystemEditor/DlgEditorUtilities.h"
#include "DlgSystem/Nodes/DlgNode_SpeechSequence.h"
#include "DlgSystem/Nodes/DlgNode_Speech.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"

#define LOCTEXT_NAMESPACE "ConvertSpeechNodesToSpeechSequence_DialogueGraphSchemaAction"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgConvertSpeechNodesToSpeechSequence_GraphSchemaAction
UEdGraphNode* FDlgConvertSpeechNodesToSpeechSequence_GraphSchemaAction::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
	FNYLocationVector2f Location, bool bSelectNewNode/* = true*/)
{
	// Should have been stopped in GetConvertActions
	check(SelectedGraphNodes.Num() > 0);
	const FScopedTransaction Transaction(LOCTEXT("DialogueditorConvertSpeechNodesToSpeechSequence", "Convert Speech Nodes to a Sequence Node"));
	UDlgDialogue* Dialogue = FDlgEditorUtilities::GetDialogueForGraph(ParentGraph);

	// Disable compiling for optimization
	Dialogue->DisableCompileDialogue();

	// Step 1. Create the final speech sequence
	UDialogueGraphNode* GraphNode_SpeechSequence = FDlgNewNode_GraphSchemaAction::SpawnGraphNodeWithDialogueNodeFromTemplate<UDialogueGraphNode>(
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
	FDlgEditorUtilities::ReplaceParentConnectionsToNode(FirstSelectedGraphNode, GraphNode_SpeechSequence);

	// Copy connections from the last node to the speech sequence
	const UDialogueGraphNode* LastSelectedNode = SelectedGraphNodes.Last();
	FDlgEditorUtilities::CopyNodeChildren(LastSelectedNode, GraphNode_SpeechSequence);

	// Step 4. Copy existing enter events and/or conditions from the first node in the sequence
	if (FirstSelectedGraphNode->HasEnterConditions() || FirstSelectedGraphNode->HasEnterEvents())
	{
		const UDlgNode_Speech& DialogueNode_Speech = FirstSelectedGraphNode->GetDialogueNode<UDlgNode_Speech>();
		if (FirstSelectedGraphNode->HasEnterConditions())
		{
			TArray<FDlgCondition> EnterConditions = DialogueNode_Speech.GetNodeEnterConditions();
			for (FDlgCondition& Condition : EnterConditions)
			{
				if (Condition.CustomCondition)
				{
					Condition.CustomCondition = DuplicateObject(Condition.CustomCondition, DialogueNode_SpeechSequence);
				}
			}
			DialogueNode_SpeechSequence->SetNodeEnterConditions(EnterConditions);
		}
		if (FirstSelectedGraphNode->HasEnterEvents())
		{
			TArray<FDlgEvent> EnterEvents = DialogueNode_Speech.GetNodeEnterEvents();
			for (FDlgEvent& Event : EnterEvents)
			{
				if (Event.CustomEvent)
				{
					Event.CustomEvent = DuplicateObject(Event.CustomEvent, DialogueNode_SpeechSequence);
				}
			}
			DialogueNode_SpeechSequence->SetNodeEnterEvents(EnterEvents);
		}
	}

	// Step 5. Remove the nodes, this will break the existing input/output connections of the nodes
	for (UDialogueGraphNode* GraphNode : SelectedGraphNodes)
		FDlgEditorUtilities::RemoveNode(GraphNode);

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
