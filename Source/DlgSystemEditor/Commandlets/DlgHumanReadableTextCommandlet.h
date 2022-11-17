// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once


#include "Commandlets/Commandlet.h"
#include "DlgSystem/DlgEdge.h"

#include "DlgHumanReadableTextCommandlet.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogDlgHumanReadableTextCommandlet, All, All);

class UDlgDialogue;
class UDlgNode;
class UDialogueGraphNode;
class UDlgSystemSettings;

USTRUCT()
struct FDlgNodeContext_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	TArray<int32> ParentNodeIndices;

	UPROPERTY()
	TArray<int32> ChildNodeIndices;
};


// Variant of the FDlgEdge that also tells us from what node to what node
USTRUCT()
struct FDlgEdgeOrphan_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()

public:
	// Metadata
	UPROPERTY()
	int32 SourceNodeIndex = INDEX_NONE - 1;

	UPROPERTY()
	int32 TargetNodeIndex = INDEX_NONE - 1;

	UPROPERTY()
	FText Text;
};


// Variant of the FDlgEdge that is human readable
USTRUCT()
struct FDlgEdge_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()

public:
	// Metadata
	UPROPERTY()
	int32 TargetNodeIndex = INDEX_NONE - 1;

	UPROPERTY()
	FText Text;
};


// Variant of the FDlgSpeechSequenceEntry that is human readable
USTRUCT()
struct FDlgSpeechSequenceEntry_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()
public:
	// ParticipantName
	UPROPERTY()
	FName Speaker;

	UPROPERTY()
	FText Text;

	UPROPERTY()
	FText EdgeText;
};


// Variant of the UDlgNode_SpeechSequence that is human readable
USTRUCT()
struct FDlgNodeSpeechSequence_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()
public:
	// INDEX_NONE is root node
	bool IsValid() const { return NodeIndex >= INDEX_NONE; }

public:
	// Metadata
	UPROPERTY()
	int32 NodeIndex = INDEX_NONE - 1;

	UPROPERTY()
	FName Speaker;

	UPROPERTY()
	TArray<FDlgSpeechSequenceEntry_FormatHumanReadable> Sequence;

	UPROPERTY()
	TArray<FDlgEdge_FormatHumanReadable> Edges;
};


// Variant of the UDlgNode_Speech that is human readable
USTRUCT()
struct FDlgNodeSpeech_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()

public:
	// INDEX_NONE is root node
	bool IsValid() const { return NodeIndex >= INDEX_NONE; }

public:
	// Metadata, NodeIndex
	UPROPERTY()
	int32 NodeIndex = INDEX_NONE - 1;

	UPROPERTY()
	FName Speaker;

	UPROPERTY()
	FText Text;

	UPROPERTY()
	TArray<FDlgEdge_FormatHumanReadable> Edges;
};


// Variant of the UDlgDialogue that is human readable
USTRUCT()
struct FDlgDialogue_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	FName DialogueName;

	UPROPERTY()
	FGuid DialogueGUID;

	UPROPERTY()
	TArray<FDlgNodeSpeech_FormatHumanReadable> SpeechNodes;

	UPROPERTY()
	TArray<FDlgNodeSpeechSequence_FormatHumanReadable> SpeechSequenceNodes;
};


UCLASS()
class UDlgHumanReadableTextCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UDlgHumanReadableTextCommandlet();

public:

	//~ UCommandlet interface
	int32 Main(const FString& Params) override;

	bool ExportDialogueToHumanReadableFormat(const UDlgDialogue& Dialogue, FDlgDialogue_FormatHumanReadable& OutFormat);

	bool ImportHumanReadableFormatIntoDialogue(const FDlgDialogue_FormatHumanReadable& Format, UDlgDialogue* Dialogue);

	// Tells us if the edge text is default
	bool IsEdgeTextDefault(const FText& EdgeText);

protected:
	// Own methods
	int32 Export();
	int32 Import();

	static bool ExportNodeToContext(const UDlgNode* Node, FDlgNodeContext_FormatHumanReadable& OutContext);
	static void ExportNodeEdgesToHumanReadableFormat(const TArray<FDlgEdge>& Edges, TArray<FDlgEdge_FormatHumanReadable>& OutEdges);
	static bool SetGraphNodesNewEdgesText(UDialogueGraphNode* GraphNode, const TArray<FDlgEdge_FormatHumanReadable>& Edges, int32 NodeIndex, const UDlgDialogue* Dialogue);

protected:
	FString OutputInputDirectory;

	TArray<UPackage*> PackagesToSave;
	const UDlgSystemSettings* Settings = nullptr;

	bool bSaveAllDialogues = false;
	bool bExport = false;
	bool bImport = false;

	static constexpr int32 RootNodeIndex = INDEX_NONE;
	static const TCHAR* FileExtension;
};
