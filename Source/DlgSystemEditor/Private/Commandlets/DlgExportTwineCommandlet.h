// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Commandlets/Commandlet.h"

#include "DlgExportTwineCommandlet.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogDlgExportTwineCommandlet, All, All);

class UDlgDialogue;
class UDlgNode;
class UDialogueGraphNode;
struct FDlgEdge;


UCLASS()
class UDlgExportTwineCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UDlgExportTwineCommandlet();

public:

	//~ UCommandlet interface
	int32 Main(const FString& Params) override;

	FString CreateTwineStoryData(const FString& Name, const FGuid& DialogueGuid, int32 StartNodeIndex, const FString& PassagesData);

	FString CreateTwinePassageDataFromNode(const UDlgDialogue& Dialogue, const UDlgNode& Node, int32 NodeIndex);
	FString CreateTwinePassageDataLinksFromEdges(const UDlgDialogue& Dialogue, const TArray<FDlgEdge>& Edges, bool bNoTextOnEdges = false);

	FString CreateTwinePassageData(int32 Pid, const FString& Name, const FString& Tags, const FIntPoint& Position, const FIntPoint& Size, const FString& Content);

	FString CreateTwineCustomCss();

	FORCEINLINE FIntPoint GraphNodeToTwineCanvas(int32 PositionX, int32 PositionY)
	{
		// Twine Graph canvas always starts from 0,0 - there is not negative position
		const int32 NewX = FMath::Abs(MinimumGraphX) + PositionX;
		const int32 NewY = FMath::Abs(MinimumGraphY) + PositionY;
		return FIntPoint(NewX, NewY);
	}

	bool GetBoxThatConflicts(const FBox2D& Box, FBox2D& OutConflict);
	FIntPoint GetNonConflictingPointFor(const FIntPoint& InPoint, const FIntPoint& Size, const FIntPoint& Padding);

	static FString CreateTwineTagColorsData();

	FString GetNodeNameFromNode(const UDlgNode& Node, int32 NodeIndex, bool bIsRootNode = false);

	FORCEINLINE static FString& EscapeHtml(FString& String)
	{
		String.ReplaceInline(TEXT("&"), TEXT("&amp;"), ESearchCase::CaseSensitive);
		String.ReplaceInline(TEXT("\""), TEXT("&quot;"), ESearchCase::CaseSensitive);
		String.ReplaceInline(TEXT("'"), TEXT("&apos;"), ESearchCase::CaseSensitive);
		String.ReplaceInline(TEXT("<"), TEXT("&lt;"), ESearchCase::CaseSensitive);
		String.ReplaceInline(TEXT(">"), TEXT("&gt;"), ESearchCase::CaseSensitive);
		return String;
	}

	FORCEINLINE static FString EscapeHtml(const FString& String)
	{
		FString NewString = String;
		NewString.ReplaceInline(TEXT("&"), TEXT("&amp;"), ESearchCase::CaseSensitive);
		NewString.ReplaceInline(TEXT("\""), TEXT("&quot;"), ESearchCase::CaseSensitive);
		NewString.ReplaceInline(TEXT("'"), TEXT("&apos;"), ESearchCase::CaseSensitive);
		NewString.ReplaceInline(TEXT("<"), TEXT("&lt;"), ESearchCase::CaseSensitive);
		NewString.ReplaceInline(TEXT(">"), TEXT("&gt;"), ESearchCase::CaseSensitive);
		return NewString;
	}

protected:
	static void InitTwinetagNodesColors();

protected:
	FString OutputDirectory;

	// Flatten files to the same directory
	bool bFlatten = false;

	// used to compute the proper size
	int32 MinimumGraphX = 0;
	int32 MinimumGraphY = 0;

	// Stop overlapping nodes
	TArray<FBox2D> CurrentNodesAreas;

	// Maps from:
	// Key: NodeTagName
	// Value: Color name https://twine2.neocities.org/#type_colour
	static TMap<FString, FString> TwineTagNodesColorsMap;

	static const FIntPoint SizeSmall;
	static const FIntPoint SizeWide;
	static const FIntPoint SizeTall;
	static const FIntPoint SizeLarge;

	static const FString TagNodeStart;
	static const FString TagNodeEnd;
	static const FString TagNodeVirtualParent;
	static const FString TagNodeSpeech;
	static const FString TagNodeSpeechSequence;
	static const FString TagNodeSelectorFirst;
	static const FString TagNodeSelectorRandom;
};
