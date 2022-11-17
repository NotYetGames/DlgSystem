// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "DlgObject.h"

#include "DlgConditionCustom.generated.h"

class UDlgContext;

// Abstract base class for a custom condition
// Extend this class to define additional data you want to store
//
// 1. Override IsConditionMet
// 2. Return true if you want the condition to succeed or false otherwise
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew)
class DLGSYSTEM_API UDlgConditionCustom : public UDlgObject
{
	GENERATED_BODY()
public:
	// Checks if the condition is met
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue")
	bool IsConditionMet(const UDlgContext* Context, const UObject* Participant);
	virtual bool IsConditionMet_Implementation(const UDlgContext* Context, const UObject* Participant)
	{
		return false;
	}
};

// This is the same as UDlgConditionCustom but it does NOT show the categories
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, CollapseCategories)
class DLGSYSTEM_API UDlgConditionCustomHideCategories : public UDlgConditionCustom
{
	GENERATED_BODY()
};
