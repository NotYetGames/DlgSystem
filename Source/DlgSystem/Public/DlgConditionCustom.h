// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "DlgObject.h"

#include "DlgConditionCustom.generated.h"

// Abstract base class for a custom
// Defining class via inheriting from UNYTaskBase outside of the plugin is possible both in Blueprint and C++
// Extend this class to define additional data you want to store on your nodes
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew)
class DLGSYSTEM_API UDlgConditionCustom : public UDlgObject
{
	GENERATED_BODY()
public:
	// Checks if the condition is met
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue|Conditions")
	bool IsConditionMet(const UObject* Participant);
	virtual bool IsConditionMet_Implementation(const UObject* Participant)
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
