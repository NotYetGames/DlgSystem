// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "DlgSystem/DlgDialogue.h"
#include "DlgSystem/Nodes/DlgNode_Speech.h"
#include "DlgSystem/Nodes/DlgNode_SpeechSequence.h"
#include "DlgSystem/Nodes/DlgNode_End.h"
#include "DlgSystemEditor/Editor/Nodes/DialogueGraphNode.h"
#include "DlgSystemEditor/Editor/Graph/DialogueGraph.h"
#include "DlgSystemEditor/Commandlets/DlgHumanReadableTextCommandlet.h"

class DLGSYSTEMEDITOR_API FDlgJsonDialogueHelper
{
public:
	// Exports a dialogue to the human-readable dialogue JSON format.
	static bool ExportDialogueToJson(const UDlgDialogue* Dialogue, FString& OutJsonString);

	// Imports text, speaker, and edge text updates from JSON into matching existing dialogue nodes only.
	static bool ImportDialogueNodeUpdatesFromJson(const FString& JsonString, UDlgDialogue* Dialogue, FString* OutError = nullptr);

	// Imports text, speaker, and edge text updates from a JSON file into matching existing dialogue nodes only.
	static bool ImportDialogueNodeUpdatesFromJsonFile(const FString& FilePath, UDlgDialogue* Dialogue, FString* OutError = nullptr);

	// Replaces the dialogue nodes and graph with nodes recreated from JSON.
	static bool ImportDialogueFromJsonAsReplacement(const FString& JsonString, UDlgDialogue* Dialogue, FString* OutError = nullptr);

	// Replaces the dialogue nodes and graph with nodes recreated from a JSON file.
	static bool ImportDialogueFromJsonFileAsReplacement(const FString& FilePath, UDlgDialogue* Dialogue, FString* OutError = nullptr);

private:
	static bool ExportDialogueToHumanReadableFormat(const UDlgDialogue& Dialogue, FDlgDialogue_FormatHumanReadable& OutFormat);
	static void ExportNodeEdgesToHumanReadableFormat(const TArray<FDlgEdge>& Edges, TArray<FDlgEdge_FormatHumanReadable>& OutEdges);
	static bool ImportHumanReadableFormatIntoDialogue(const FDlgDialogue_FormatHumanReadable& Format, UDlgDialogue* Dialogue, FString* OutError = nullptr);
	static bool ImportHumanReadableFormatIntoDialogueDestructive(const FDlgDialogue_FormatHumanReadable& Format, UDlgDialogue* Dialogue, FString* OutError = nullptr);
	static bool SetGraphNodesNewEdgesText(UDialogueGraphNode* GraphNode, const TArray<FDlgEdge_FormatHumanReadable>& Edges, int32 NodeIndex, const UDlgDialogue* Dialogue);

	static constexpr int32 RootNodeIndex = INDEX_NONE;
};
