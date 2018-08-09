
// Copyright 2017-2018 Csaba Molnar, Daniel Butum
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

	FString CreateTwineStoryData(const FString& Name, const FGuid& DialogueGuid, const int32 StartNodeIndex, const FString& PassagesData);

	FString CreateTwinePassageDataFromNode(const UDlgDialogue& Dialogue, const UDlgNode& Node, const int32 NodeIndex);
	FString CreateTwinePassageDataLinksFromEdges(const UDlgDialogue& Dialogue, const TArray<FDlgEdge>& Edges, const bool bNoTextOnEdges = false);

	FString CreateTwinePassageData(const int32 Pid, const FString& Name, const FString& Tags, const FIntPoint& Position, const FIntPoint& Size, const FString& Content);

	FString CreateTwineCustomCss();

	FORCEINLINE FIntPoint GraphNodeToTwineCanvas(const int32 PositionX, const int32 PositionY)
	{
		// Twine Graph canvas always starts from 0,0 - there is not negative position
		// TODO proper translate
		//const int32 NewX = PositionX < 0 ? FMath::Abs(PositionX) + 200 : PositionX + PositionX + 400;
		//const int32 NewY = PositionY < 0 ? FMath::Abs(PositionY) + 200 : PositionY + PositionY/2 + 200;
		int32 NewX = MinimumGraphX < 0 ? FMath::Abs(MinimumGraphX) + PositionX : PositionX;
		int32 NewY = MinimumGraphY < 0 ? FMath::Abs(MinimumGraphY) + PositionY : PositionY;

		return FIntPoint(NewX + 200, NewY + 200);
	}

	static FString CreateTwineTagColorsData();

	FString GetNodeNameFromNode(const UDlgNode& Node, const int32 NodeIndex, const bool bIsRootNode = false);

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

	// used to compute the proper size
	int32 MinimumGraphX = 0;
	int32 MinimumGraphY = 0;

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
