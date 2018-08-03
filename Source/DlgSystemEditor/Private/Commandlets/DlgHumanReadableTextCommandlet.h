// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Commandlets/Commandlet.h"

#include "DlgHumanReadableTextCommandlet.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogDlgHumanReadableTextCommandlet, All, All);

class UDlgDialogue;
class UDlgNode;

USTRUCT()
struct FDlgNodeContext_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()
public:
	// Some context
	UPROPERTY()
	FName Speaker;

	UPROPERTY()
	TArray<int32> ParentNodeIndices;

	UPROPERTY()
	TArray<int32> ChildNodeIndices;
};

// Variant of the FDlgEdge that is human readable
USTRUCT()
struct FDlgEdge_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()

public:
	// Metadata
	UPROPERTY()
	int32 TargetNodeIndex = INDEX_NONE;

	UPROPERTY()
	FText Text;
};


// Variant of the FDlgEdge that also tells us from what node to what node
USTRUCT()
struct FDlgEdgeOrphan_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()

public:
	// Metadata
	UPROPERTY()
	int32 SourceNodeIndex = INDEX_NONE;

	UPROPERTY()
	int32 TargetNodeIndex = INDEX_NONE;

	UPROPERTY()
	FText Text;
};


// Variant of the FDlgSpeechSequenceEntry that is human readable
USTRUCT()
struct FDlgSpeechSequenceEntry_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()
public:
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
	// Metadata
	UPROPERTY()
	int32 NodeIndex = INDEX_NONE;

	UPROPERTY()
	FDlgNodeContext_FormatHumanReadable Context;

	UPROPERTY()
	TArray<FDlgSpeechSequenceEntry_FormatHumanReadable> Sequence;
};


// Variant of the UDlgNode_Speech that is human readable
USTRUCT()
struct FDlgNodeSpeech_FormatHumanReadable
{
	GENERATED_USTRUCT_BODY()

public:
	// Metadata, NodeIndex
	UPROPERTY()
	int32 NodeIndex = INDEX_NONE;

	UPROPERTY()
	FDlgNodeContext_FormatHumanReadable Context;

	UPROPERTY()
	FText Text;
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
	FGuid DialogueGuid;

	UPROPERTY()
	TArray<FDlgNodeSpeech_FormatHumanReadable> SpeechNodes;

	UPROPERTY()
	TArray<FDlgNodeSpeechSequence_FormatHumanReadable> SpeechSequenceNodes;

	// Edges from other nodes
	UPROPERTY()
	TArray<FDlgEdgeOrphan_FormatHumanReadable> SpeechEdges;
};


UCLASS()
class UDlgHumanReadableTextCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UDlgHumanReadableTextCommandlet();

public:

	//~ UCommandlet interface
	int32 Main(const FString& Params) override;

	// Own methods
	void Export();
	void Import();

	static bool ExportDialogueToHumanReadableFormat(const UDlgDialogue& Dialogue, FDlgDialogue_FormatHumanReadable& OutFormat);
	static bool ExportNodeToContext(const UDlgNode* Node, FDlgNodeContext_FormatHumanReadable& OutContext);


	// Tells us if the edge text is default
	static bool IsEdgeTextDefault(const FText& EdgeText);

protected:
	FString OutputInputDirectory;
};
