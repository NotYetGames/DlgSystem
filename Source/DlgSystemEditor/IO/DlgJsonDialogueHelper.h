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
	static bool ExportDialogueToJson(const UDlgDialogue* Dialogue, FString& OutJsonString);
	static bool ImportDialogueFromJson(const FString& JsonString, UDlgDialogue* Dialogue, FString* OutError = nullptr);
	static bool ImportDialogueFromJsonFile(const FString& FilePath, UDlgDialogue* Dialogue, FString* OutError = nullptr);
	static bool ImportDialogueFromJsonDestructive(const FString& JsonString, UDlgDialogue* Dialogue, FString* OutError = nullptr);
	static bool ImportDialogueFromJsonFileDestructive(const FString& FilePath, UDlgDialogue* Dialogue, FString* OutError = nullptr);

private:
	static bool ExportDialogueToHumanReadableFormat(const UDlgDialogue& Dialogue, FDlgDialogue_FormatHumanReadable& OutFormat);
	static void ExportNodeEdgesToHumanReadableFormat(const TArray<FDlgEdge>& Edges, TArray<FDlgEdge_FormatHumanReadable>& OutEdges);
	static bool ImportHumanReadableFormatIntoDialogue(const FDlgDialogue_FormatHumanReadable& Format, UDlgDialogue* Dialogue, FString* OutError = nullptr);
	static bool ImportHumanReadableFormatIntoDialogueDestructive(const FDlgDialogue_FormatHumanReadable& Format, UDlgDialogue* Dialogue, FString* OutError = nullptr);
	static bool SetGraphNodesNewEdgesText(UDialogueGraphNode* GraphNode, const TArray<FDlgEdge_FormatHumanReadable>& Edges, int32 NodeIndex, const UDlgDialogue* Dialogue);

	static constexpr int32 RootNodeIndex = INDEX_NONE;
};
