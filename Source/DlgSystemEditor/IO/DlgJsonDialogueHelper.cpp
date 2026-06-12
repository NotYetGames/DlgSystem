// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgJsonDialogueHelper.h"

#include "DlgSystem/IO/DlgJsonWriter.h"
#include "DlgSystem/IO/DlgJsonParser.h"
#include "DlgSystem/DlgHelper.h"
#include "DlgSystem/Nodes/DlgNode_Start.h"

bool FDlgJsonDialogueHelper::ExportDialogueToJson(const UDlgDialogue* Dialogue, FString& OutJsonString)
{
	if (!IsValid(Dialogue))
	{
		return false;
	}

	FDlgDialogue_FormatHumanReadable ExportFormat;
	if (!ExportDialogueToHumanReadableFormat(*Dialogue, ExportFormat))
	{
		return false;
	}

	FDlgJsonWriter JsonWriter;
	JsonWriter.Write(FDlgDialogue_FormatHumanReadable::StaticStruct(), &ExportFormat);
	OutJsonString = JsonWriter.GetAsString();
	return !OutJsonString.IsEmpty();
}

bool FDlgJsonDialogueHelper::ImportDialogueNodeUpdatesFromJson(const FString& JsonString, UDlgDialogue* Dialogue, FString* OutError)
{
	if (!IsValid(Dialogue))
	{
		if (OutError) *OutError = TEXT("Invalid dialogue asset");
		return false;
	}
	if (JsonString.IsEmpty())
	{
		if (OutError) *OutError = TEXT("Empty JSON string");
		return false;
	}

	FDlgJsonParser JsonParser;
	JsonParser.InitializeParserFromString(JsonString);
	if (!JsonParser.IsValidFile())
	{
		if (OutError) *OutError = TEXT("Invalid JSON format");
		return false;
	}

	FDlgDialogue_FormatHumanReadable HumanFormat;
	JsonParser.ReadAllProperty(FDlgDialogue_FormatHumanReadable::StaticStruct(), &HumanFormat);
	if (!JsonParser.IsValidFile())
	{
		if (OutError) *OutError = TEXT("Failed to parse dialogue format from JSON");
		return false;
	}

	return ImportHumanReadableFormatIntoDialogue(HumanFormat, Dialogue, OutError);
}

bool FDlgJsonDialogueHelper::ImportDialogueNodeUpdatesFromJsonFile(const FString& FilePath, UDlgDialogue* Dialogue, FString* OutError)
{
	if (!IsValid(Dialogue))
	{
		if (OutError) *OutError = TEXT("Invalid dialogue asset");
		return false;
	}
	if (FilePath.IsEmpty())
	{
		if (OutError) *OutError = TEXT("Empty file path");
		return false;
	}

	FDlgJsonParser JsonParser;
	JsonParser.InitializeParser(FilePath);
	if (!JsonParser.IsValidFile())
	{
		if (OutError) *OutError = FString::Printf(TEXT("Failed to read/parse JSON file: %s"), *FilePath);
		return false;
	}

	FDlgDialogue_FormatHumanReadable HumanFormat;
	JsonParser.ReadAllProperty(FDlgDialogue_FormatHumanReadable::StaticStruct(), &HumanFormat);
	if (!JsonParser.IsValidFile())
	{
		if (OutError) *OutError = TEXT("Failed to parse dialogue format from JSON file");
		return false;
	}

	return ImportHumanReadableFormatIntoDialogue(HumanFormat, Dialogue, OutError);
}

bool FDlgJsonDialogueHelper::ImportDialogueFromJsonAsReplacement(const FString& JsonString, UDlgDialogue* Dialogue, FString* OutError)
{
	if (!IsValid(Dialogue))
	{
		if (OutError) *OutError = TEXT("Invalid dialogue asset");
		return false;
	}
	if (JsonString.IsEmpty())
	{
		if (OutError) *OutError = TEXT("Empty JSON string");
		return false;
	}

	FDlgJsonParser JsonParser;
	JsonParser.InitializeParserFromString(JsonString);
	if (!JsonParser.IsValidFile())
	{
		if (OutError) *OutError = TEXT("Invalid JSON format");
		return false;
	}

	FDlgDialogue_FormatHumanReadable HumanFormat;
	JsonParser.ReadAllProperty(FDlgDialogue_FormatHumanReadable::StaticStruct(), &HumanFormat);
	if (!JsonParser.IsValidFile())
	{
		if (OutError) *OutError = TEXT("Failed to parse dialogue format from JSON");
		return false;
	}

	return ImportHumanReadableFormatIntoDialogueDestructive(HumanFormat, Dialogue, OutError);
}

bool FDlgJsonDialogueHelper::ImportDialogueFromJsonFileAsReplacement(const FString& FilePath, UDlgDialogue* Dialogue, FString* OutError)
{
	if (!IsValid(Dialogue))
	{
		if (OutError) *OutError = TEXT("Invalid dialogue asset");
		return false;
	}
	if (FilePath.IsEmpty())
	{
		if (OutError) *OutError = TEXT("Empty file path");
		return false;
	}

	FDlgJsonParser JsonParser;
	JsonParser.InitializeParser(FilePath);
	if (!JsonParser.IsValidFile())
	{
		if (OutError) *OutError = FString::Printf(TEXT("Failed to read/parse JSON file: %s"), *FilePath);
		return false;
	}

	FDlgDialogue_FormatHumanReadable HumanFormat;
	JsonParser.ReadAllProperty(FDlgDialogue_FormatHumanReadable::StaticStruct(), &HumanFormat);
	if (!JsonParser.IsValidFile())
	{
		if (OutError) *OutError = TEXT("Failed to parse dialogue format from JSON file");
		return false;
	}

	return ImportHumanReadableFormatIntoDialogueDestructive(HumanFormat, Dialogue, OutError);
}

bool FDlgJsonDialogueHelper::ExportDialogueToHumanReadableFormat(const UDlgDialogue& Dialogue, FDlgDialogue_FormatHumanReadable& OutFormat)
{
	OutFormat.DialogueName = Dialogue.GetDialogueFName();
	OutFormat.DialogueGUID = Dialogue.GetGUID();

	// Root Nodes
	const TArray<UDlgNode*> StartNodes = Dialogue.GetStartNodes();
	for (int32 i = 0; i < StartNodes.Num(); ++i)
	{
		FDlgNodeSpeech_FormatHumanReadable RootNode;
		RootNode.NodeIndex = RootNodeIndex * (i + 1);
		ExportNodeEdgesToHumanReadableFormat(StartNodes[i]->GetNodeChildren(), RootNode.Edges);
		OutFormat.SpeechNodes.Add(RootNode);
	}

	const TArray<UDlgNode*>& Nodes = Dialogue.GetNodes();
	for (int32 NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++)
	{
		const UDlgNode* Node = Nodes[NodeIndex];
		if (const UDlgNode_Speech* NodeSpeech = Cast<UDlgNode_Speech>(Node))
		{
			FDlgNodeSpeech_FormatHumanReadable ExportNode;
			ExportNode.NodeIndex = NodeIndex;
			ExportNode.Speaker = NodeSpeech->GetNodeParticipantName();
			ExportNode.Text = NodeSpeech->GetNodeUnformattedText();
			ExportNodeEdgesToHumanReadableFormat(Node->GetNodeChildren(), ExportNode.Edges);
			OutFormat.SpeechNodes.Add(ExportNode);
		}
		else if (const UDlgNode_SpeechSequence* NodeSpeechSequence = Cast<UDlgNode_SpeechSequence>(Node))
		{
			FDlgNodeSpeechSequence_FormatHumanReadable ExportNode;
			ExportNode.NodeIndex = NodeIndex;
			ExportNode.Speaker = NodeSpeechSequence->GetNodeParticipantName();
			for (const FDlgSpeechSequenceEntry& Entry : NodeSpeechSequence->GetNodeSpeechSequence())
			{
				FDlgSpeechSequenceEntry_FormatHumanReadable ExportEntry;
				ExportEntry.EdgeText = Entry.EdgeText;
				ExportEntry.Text = Entry.Text;
				ExportEntry.Speaker = Entry.Speaker;
				ExportNode.Sequence.Add(ExportEntry);
			}
			ExportNodeEdgesToHumanReadableFormat(Node->GetNodeChildren(), ExportNode.Edges);
			OutFormat.SpeechSequenceNodes.Add(ExportNode);
		}
		else
		{
			// Other node types (End, Start, Proxy, Selector, Custom, etc.)
			// Export as empty speech node so edges are preserved
			FDlgNodeSpeech_FormatHumanReadable ExportNode;
			ExportNode.NodeIndex = NodeIndex;
			ExportNode.Speaker = NAME_None;
			ExportNode.Text = FText::GetEmpty();
			ExportNodeEdgesToHumanReadableFormat(Node->GetNodeChildren(), ExportNode.Edges);
			OutFormat.SpeechNodes.Add(ExportNode);
		}

		if (const UDialogueGraphNode_Base* GraphNode = Cast<UDialogueGraphNode_Base>(Node->GetGraphNode()))
		{
			GraphNode->CheckAll();
		}
	}

	return true;
}

void FDlgJsonDialogueHelper::ExportNodeEdgesToHumanReadableFormat(const TArray<FDlgEdge>& Edges, TArray<FDlgEdge_FormatHumanReadable>& OutEdges)
{
	for (const FDlgEdge& Edge : Edges)
	{
		if (!Edge.IsValid())
		{
			continue;
		}

		FDlgEdge_FormatHumanReadable ExportEdge;
		ExportEdge.TargetNodeIndex = Edge.TargetIndex;
		ExportEdge.Text = Edge.GetUnformattedText();
		OutEdges.Add(ExportEdge);
	}
}

bool FDlgJsonDialogueHelper::ImportHumanReadableFormatIntoDialogue(const FDlgDialogue_FormatHumanReadable& Format, UDlgDialogue* Dialogue, FString* OutError)
{
	verify(Dialogue);

	bool bModified = false;
	int32 ProcessedNodes = 0;
	int32 MismatchedNodes = 0;

	if (Format.SpeechNodes.Num() == 0 && Format.SpeechSequenceNodes.Num() == 0)
	{
		if (OutError) *OutError = TEXT("No speech nodes or speech sequence nodes found in JSON");
		return false;
	}

	// Speech nodes
	for (const FDlgNodeSpeech_FormatHumanReadable& HumanNode : Format.SpeechNodes)
	{
		if (!HumanNode.IsValid())
		{
			continue;
		}

		const bool bIsRootNode = HumanNode.NodeIndex <= RootNodeIndex;
		const int32 StartNodeIndex = FMath::Abs(HumanNode.NodeIndex) - 1;
		UDlgNode* Node = nullptr;
		if (bIsRootNode)
		{
			const TArray<UDlgNode*>& StartNodes = Dialogue->GetStartNodes();
			Node = StartNodes.IsValidIndex(StartNodeIndex) ? StartNodes[StartNodeIndex] : nullptr;
		}
		else
		{
			Node = Dialogue->GetMutableNodeFromIndex(HumanNode.NodeIndex);
		}

		if (Node == nullptr)
		{
			if (bIsRootNode)
			{
				MismatchedNodes++;
				if (OutError && OutError->IsEmpty())
				{
					*OutError = FString::Printf(TEXT("Start node index %d from JSON not found in dialogue (dialogue has %d start nodes)"),
						StartNodeIndex, Dialogue->GetStartNodes().Num());
				}
			}
			else
			{
				MismatchedNodes++;
				if (OutError && OutError->IsEmpty())
				{
					*OutError = FString::Printf(TEXT("Node index %d from JSON not found in dialogue (dialogue has %d nodes)"),
						HumanNode.NodeIndex, Dialogue->GetNodes().Num());
				}
			}
			continue;
		}

		if (!bIsRootNode)
		{
			UDlgNode_Speech* NodeSpeech = Cast<UDlgNode_Speech>(Node);
			if (NodeSpeech != nullptr)
			{
				if (!NodeSpeech->GetNodeUnformattedText().EqualTo(HumanNode.Text))
				{
					NodeSpeech->SetNodeText(HumanNode.Text);
					bModified = true;
				}

				if (!NodeSpeech->GetNodeParticipantName().IsEqual(HumanNode.Speaker, ENameCase::CaseSensitive))
				{
					NodeSpeech->SetNodeParticipantName(HumanNode.Speaker);
					bModified = true;
				}
			}
			else
			{
				// Node exists but is not a speech node (e.g. End node) - that's ok, just skip text update
			}
		}

		UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(Node->GetGraphNode());
		if (GraphNode == nullptr)
		{
			continue;
		}

		if (SetGraphNodesNewEdgesText(GraphNode, HumanNode.Edges, HumanNode.NodeIndex, Dialogue))
		{
			bModified = true;
		}
		GraphNode->CheckAll();
		ProcessedNodes++;
	}

	// Speech sequence nodes
	for (const FDlgNodeSpeechSequence_FormatHumanReadable& HumanSpeechSequence : Format.SpeechSequenceNodes)
	{
		if (!HumanSpeechSequence.IsValid())
		{
			continue;
		}

		UDlgNode* Node = Dialogue->GetMutableNodeFromIndex(HumanSpeechSequence.NodeIndex);
		if (Node == nullptr)
		{
			MismatchedNodes++;
			if (OutError && OutError->IsEmpty())
			{
				*OutError = FString::Printf(TEXT("Speech sequence node index %d from JSON not found in dialogue (dialogue has %d nodes)"),
					HumanSpeechSequence.NodeIndex, Dialogue->GetNodes().Num());
			}
			continue;
		}

		UDlgNode_SpeechSequence* NodeSpeechSequence = Cast<UDlgNode_SpeechSequence>(Node);
		if (NodeSpeechSequence == nullptr)
		{
			MismatchedNodes++;
			if (OutError && OutError->IsEmpty())
			{
				*OutError = FString::Printf(TEXT("Node index %d is not a speech sequence node in the dialogue"), HumanSpeechSequence.NodeIndex);
			}
			continue;
		}

		if (!NodeSpeechSequence->GetNodeParticipantName().IsEqual(HumanSpeechSequence.Speaker, ENameCase::CaseSensitive))
		{
			NodeSpeechSequence->SetNodeParticipantName(HumanSpeechSequence.Speaker);
			bModified = true;
		}

		TArray<FDlgSpeechSequenceEntry>& SequenceArray = *NodeSpeechSequence->GetMutableNodeSpeechSequence();
		for (int32 SequenceIndex = 0; SequenceIndex < SequenceArray.Num() && SequenceIndex < HumanSpeechSequence.Sequence.Num(); SequenceIndex++)
		{
			const FDlgSpeechSequenceEntry_FormatHumanReadable& HumanSequence = HumanSpeechSequence.Sequence[SequenceIndex];

			if (!SequenceArray[SequenceIndex].EdgeText.EqualTo(HumanSequence.EdgeText))
			{
				SequenceArray[SequenceIndex].EdgeText = HumanSequence.EdgeText;
				bModified = true;
			}

			if (!SequenceArray[SequenceIndex].Speaker.IsEqual(HumanSequence.Speaker, ENameCase::CaseSensitive))
			{
				SequenceArray[SequenceIndex].Speaker = HumanSequence.Speaker;
				bModified = true;
			}

			if (!SequenceArray[SequenceIndex].Text.EqualTo(HumanSequence.Text))
			{
				SequenceArray[SequenceIndex].Text = HumanSequence.Text;
				bModified = true;
			}
		}

		UDialogueGraphNode* GraphNode = Cast<UDialogueGraphNode>(Node->GetGraphNode());
		if (GraphNode == nullptr)
		{
			continue;
		}

		if (SetGraphNodesNewEdgesText(GraphNode, HumanSpeechSequence.Edges, HumanSpeechSequence.NodeIndex, Dialogue))
		{
			bModified = true;
		}
		GraphNode->CheckAll();
		ProcessedNodes++;
	}

	if (bModified)
	{
		Dialogue->Modify();
		Dialogue->MarkPackageDirty();
	}

	if (ProcessedNodes == 0)
	{
		if (OutError && OutError->IsEmpty())
		{
			if (MismatchedNodes > 0)
			{
				*OutError = FString::Printf(TEXT("Import failed. %d node(s) from JSON did not match the dialogue structure."), MismatchedNodes);
			}
			else
			{
				*OutError = TEXT("Import completed but no nodes were processed. The JSON file may be empty or all nodes were invalid.");
			}
		}
		return false;
	}

	return true;
}

bool FDlgJsonDialogueHelper::ImportHumanReadableFormatIntoDialogueDestructive(
	const FDlgDialogue_FormatHumanReadable& Format, UDlgDialogue* Dialogue, FString* OutError)
{
	verify(Dialogue);

	if (Format.SpeechNodes.Num() == 0 && Format.SpeechSequenceNodes.Num() == 0)
	{
		if (OutError) *OutError = TEXT("No speech nodes or speech sequence nodes found in JSON");
		return false;
	}

	// Clear existing graph and dialogue data
	// IMPORTANT: Clear graph first to remove stale graph node references
	if (Dialogue->GetGraph())
	{
		Dialogue->ClearGraph();
	}
	Dialogue->SetStartNodes(TArray<UDlgNode*>());
	Dialogue->SetNodes(TArray<UDlgNode*>());

	// Maps JSON node indices to rebuilt dialogue data.
	TMap<int32, UDlgNode*> CreatedNodes;
	TMap<int32, int32> OldNodeIndexToNewNodeIndex;
	TMap<int32, TArray<FDlgEdge_FormatHumanReadable>> NodeEdges;

	// First pass: create all nodes
	for (const FDlgNodeSpeech_FormatHumanReadable& HumanNode : Format.SpeechNodes)
	{
		if (!HumanNode.IsValid())
		{
			continue;
		}

		const bool bIsRootNode = HumanNode.NodeIndex <= RootNodeIndex;
		if (bIsRootNode)
		{
			// Create start node
			UDlgNode_Start* StartNode = Dialogue->ConstructDialogueNode<UDlgNode_Start>();
			Dialogue->AddStartNode(StartNode);
			CreatedNodes.Add(HumanNode.NodeIndex, StartNode);
			NodeEdges.Add(HumanNode.NodeIndex, HumanNode.Edges);
		}
		else
		{
			// Create speech node
			UDlgNode_Speech* SpeechNode = Dialogue->ConstructDialogueNode<UDlgNode_Speech>();
			SpeechNode->SetNodeParticipantName(HumanNode.Speaker);
			SpeechNode->SetNodeText(HumanNode.Text);
			const int32 NewNodeIndex = Dialogue->AddNode(SpeechNode);
			CreatedNodes.Add(HumanNode.NodeIndex, SpeechNode);
			OldNodeIndexToNewNodeIndex.Add(HumanNode.NodeIndex, NewNodeIndex);
			NodeEdges.Add(HumanNode.NodeIndex, HumanNode.Edges);
		}
	}

	for (const FDlgNodeSpeechSequence_FormatHumanReadable& HumanSpeechSequence : Format.SpeechSequenceNodes)
	{
		if (!HumanSpeechSequence.IsValid())
		{
			continue;
		}

		UDlgNode_SpeechSequence* SequenceNode = Dialogue->ConstructDialogueNode<UDlgNode_SpeechSequence>();
		SequenceNode->SetNodeParticipantName(HumanSpeechSequence.Speaker);
		
		TArray<FDlgSpeechSequenceEntry>& SequenceArray = *SequenceNode->GetMutableNodeSpeechSequence();
		SequenceArray.Empty();
		for (const FDlgSpeechSequenceEntry_FormatHumanReadable& HumanSequence : HumanSpeechSequence.Sequence)
		{
			FDlgSpeechSequenceEntry Entry;
			Entry.EdgeText = HumanSequence.EdgeText;
			Entry.Text = HumanSequence.Text;
			Entry.Speaker = HumanSequence.Speaker;
			SequenceArray.Add(Entry);
		}
		
		const int32 NewNodeIndex = Dialogue->AddNode(SequenceNode);
		CreatedNodes.Add(HumanSpeechSequence.NodeIndex, SequenceNode);
		OldNodeIndexToNewNodeIndex.Add(HumanSpeechSequence.NodeIndex, NewNodeIndex);
		NodeEdges.Add(HumanSpeechSequence.NodeIndex, HumanSpeechSequence.Edges);
	}

	// Second pass: wire up edges
	for (const auto& Pair : NodeEdges)
	{
		const int32 SourceIndex = Pair.Key;
		const TArray<FDlgEdge_FormatHumanReadable>& Edges = Pair.Value;
		
		UDlgNode** SourceNodePtr = CreatedNodes.Find(SourceIndex);
		if (!SourceNodePtr || !*SourceNodePtr)
		{
			continue;
		}
		
		UDlgNode* SourceNode = *SourceNodePtr;
		for (const FDlgEdge_FormatHumanReadable& HumanEdge : Edges)
		{
			UDlgNode** TargetNodePtr = CreatedNodes.Find(HumanEdge.TargetNodeIndex);
			if (!TargetNodePtr || !*TargetNodePtr)
			{
				// Target node doesn't exist - create an End node as fallback
				UDlgNode_End* EndNode = Dialogue->ConstructDialogueNode<UDlgNode_End>();
				const int32 NewNodeIndex = Dialogue->AddNode(EndNode);
				CreatedNodes.Add(HumanEdge.TargetNodeIndex, EndNode);
				OldNodeIndexToNewNodeIndex.Add(HumanEdge.TargetNodeIndex, NewNodeIndex);
				TargetNodePtr = CreatedNodes.Find(HumanEdge.TargetNodeIndex);
			}
			
			if (TargetNodePtr && *TargetNodePtr)
			{
				const int32* NewTargetIndex = OldNodeIndexToNewNodeIndex.Find(HumanEdge.TargetNodeIndex);
				if (NewTargetIndex == nullptr)
				{
					continue;
				}

				FDlgEdge Edge;
				Edge.TargetIndex = *NewTargetIndex;
				Edge.SetText(HumanEdge.Text);
				SourceNode->AddNodeChild(Edge);
			}
		}
	}

	Dialogue->Modify();
	Dialogue->MarkPackageDirty();
	
	// Rebuild the graph from the new dialogue nodes
	if (UDialogueGraph* DialogueGraph = Cast<UDialogueGraph>(Dialogue->GetGraph()))
	{
		DialogueGraph->RemoveAllNodes();
		DialogueGraph->CreateGraphNodesFromDialogue();
		DialogueGraph->LinkGraphNodesFromDialogue();
		DialogueGraph->AutoPositionGraphNodes();
	}
	
	return true;
}

bool FDlgJsonDialogueHelper::SetGraphNodesNewEdgesText(UDialogueGraphNode* GraphNode, const TArray<FDlgEdge_FormatHumanReadable>& Edges, int32 NodeIndex, const UDlgDialogue* Dialogue)
{
	bool bModified = false;

	for (const FDlgEdge_FormatHumanReadable& HumanEdge : Edges)
	{
		const int32 EdgeIndex = GraphNode->GetChildEdgeIndexForChildNodeIndex(HumanEdge.TargetNodeIndex);
		if (EdgeIndex < 0)
		{
			continue;
		}

		if (!GraphNode->GetDialogueNode().GetNodeChildren()[EdgeIndex].GetUnformattedText().EqualTo(HumanEdge.Text))
		{
			GraphNode->SetEdgeTextAt(EdgeIndex, HumanEdge.Text);
			bModified = true;
		}
	}

	return bModified;
}
