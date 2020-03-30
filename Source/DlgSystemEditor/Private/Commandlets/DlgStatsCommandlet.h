// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Commandlets/Commandlet.h"

#include "DlgStatsCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgStatsCommandlet, All, All);


class UDlgDialogue;
class UDlgNode;


struct FDlgStatsDialogue
{
public:
	int32 WordCount = 0;

	FDlgStatsDialogue& operator+=(const FDlgStatsDialogue& Other)
	{
		WordCount += Other.WordCount;
		return *this;
	}

};


UCLASS()
class UDlgStatsCommandlet: public UCommandlet
{
	GENERATED_BODY()

public:
	UDlgStatsCommandlet();

public:

	//~ UCommandlet interface
	int32 Main(const FString& Params) override;

	bool GetStatsForDialogue(const UDlgDialogue& Dialogue, FDlgStatsDialogue& OutStats);
	int32 GetNodeWordCount(const UDlgNode& Node) const;

	int32 GetStringWordCount(const FString& String) const;
	int32 GetFNameWordCount(const FName Name) const { return GetStringWordCount(Name.ToString()); }
	int32 GetTextWordCount(const FText& Text) const { return GetStringWordCount(Text.ToString()); }
};
