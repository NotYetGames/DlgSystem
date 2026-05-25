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
	/** Serializes a dialogue into the existing human-readable dialogue structure, then writes it as JSON. */
	static bool ExportDialogueToJson(const UDlgDialogue* Dialogue, FString& OutJsonString);

	/** Imports JSON as non-overwriting text, speaker, and edge updates into the existing dialogue graph. */
	static bool ImportDialogueFromJson(const FString& JsonString, UDlgDialogue* Dialogue, FString* OutError = nullptr);

	/** File-based wrapper for ImportDialogueFromJson. */
	static bool ImportDialogueFromJsonFile(const FString& FilePath, UDlgDialogue* Dialogue, FString* OutError = nullptr);

	/** Unsupported because this JSON format does not preserve every dialogue node type and node-specific property. */
	static bool ImportDialogueFromJsonDestructive(const FString& JsonString, UDlgDialogue* Dialogue, FString* OutError = nullptr);

	/** File-based wrapper for the unsupported full-overwrite import path. */
	static bool ImportDialogueFromJsonFileDestructive(const FString& FilePath, UDlgDialogue* Dialogue, FString* OutError = nullptr);

private:
	static bool ExportDialogueToHumanReadableFormat(const UDlgDialogue& Dialogue, FDlgDialogue_FormatHumanReadable& OutFormat);
	static void ExportNodeEdgesToHumanReadableFormat(const TArray<FDlgEdge>& Edges, TArray<FDlgEdge_FormatHumanReadable>& OutEdges);
	static bool ImportHumanReadableFormatIntoDialogue(const FDlgDialogue_FormatHumanReadable& Format, UDlgDialogue* Dialogue, FString* OutError = nullptr);
	static bool SetGraphNodesNewEdgesText(UDialogueGraphNode* GraphNode, const TArray<FDlgEdge_FormatHumanReadable>& Edges, int32 NodeIndex, const UDlgDialogue* Dialogue);

	static constexpr int32 RootNodeIndex = INDEX_NONE;
};
