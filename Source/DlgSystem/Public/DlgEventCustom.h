// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "UObject/Object.h"

#include "DlgEventCustom.generated.h"

class UDlgContext;

// Abstract base class for node data
// Defining class via inheriting from UNYTaskBase outside of the plugin is possible both in Blueprint and C++
// Extend this class to define additional data you want to store on your nodes
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, HideCategories = ("DoNotShow"), AutoExpandCategories = ("Default"))
class DLGSYSTEM_API UDlgEventCustom : public UObject
{
	GENERATED_BODY()

public:
	// UObject interface
	UWorld* GetWorld() const override;

	// Called when the event is triggered.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = DialogueEvents, DisplayName = "Enter")
	void EnterEvent(UDlgContext* Context, UObject* Participant);
	void EnterEvent_Implementation(UDlgContext* Context, UObject* Participant) {}
};

// This is the same as UDlgEventCustom but it does NOT show the categories
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DLGSYSTEM_API UDlgEventCustomHideCategories : public UDlgEventCustom
{
	GENERATED_BODY()
};
