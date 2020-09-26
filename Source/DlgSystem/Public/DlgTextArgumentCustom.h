// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "DlgObject.h"

#include "DlgTextArgumentCustom.generated.h"

class UDlgContext;


// Abstract base class for a custom text argument
// Extend this class to define additional data you want to store
//
// 1. Override GetText
// 2. Return the new Text for the specified text argument
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew)
class DLGSYSTEM_API UDlgTextArgumentCustom : public UDlgObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue")
	FText GetText(const UDlgContext* Context, const UObject* Participant, const FString& DisplayStringParam);
	virtual FText GetText_Implementation(const UDlgContext* Context, const UObject* Participant, const FString& DisplayStringParam)
	{
		return FText::GetEmpty();
	}
};

// This is the same as UDlgTextArgumentCustom but it does NOT show the categories
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, CollapseCategories)
class DLGSYSTEM_API UDlgTextArgumentCustomHideCategories : public UDlgTextArgumentCustom
{
	GENERATED_BODY()
};
