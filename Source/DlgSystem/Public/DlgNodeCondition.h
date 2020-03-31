// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once
#include "UObject/Object.h"

#include "DlgNodeCondition.generated.h"

// Abstract base class for node data
// Defining class via inheriting from UNYTaskBase outside of the plugin is possible both in Blueprint and C++
// Extend this class to define additional data you want to store on your nodes
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DLGSYSTEM_API UDlgNodeCondition : public UObject
{
	GENERATED_BODY()
public:
	// UObject interface
	UWorld* GetWorld() const override
	{
		if (HasAnyFlags(RF_ArchetypeObject | RF_ClassDefaultObject))
		{
			return nullptr;
		}
		return CachedWorld;
	}

	// Checks if the condition is met
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = DialogueConditions)
	bool IsConditionMet(const UObject* Participant);

public:
	// This is just the last used world, it will be set right before any event is called
	UPROPERTY(Transient)
	UWorld* CachedWorld = nullptr;
};
