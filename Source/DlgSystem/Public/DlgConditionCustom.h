// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "UObject/Object.h"

#include "DlgConditionCustom.generated.h"

// Abstract base class for a custom
// Defining class via inheriting from UNYTaskBase outside of the plugin is possible both in Blueprint and C++
// Extend this class to define additional data you want to store on your nodes
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, HideCategories = ("DoNotShow"), AutoExpandCategories = ("Default"))
class DLGSYSTEM_API UDlgConditionCustom : public UObject
{
	GENERATED_BODY()
public:
	// UObject interface
	UWorld* GetWorld() const override;

	// Checks if the condition is met
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = DialogueConditions)
	bool IsConditionMet(const UObject* Participant);
	bool IsConditionMet_Implementation(const UObject* Participant)
	{
		return false;
	}
};


// This is the same as UDlgConditionCustom but it does NOT show the categories
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DLGSYSTEM_API UDlgConditionCustomHideCategories : public UDlgConditionCustom
{
	GENERATED_BODY()
};
