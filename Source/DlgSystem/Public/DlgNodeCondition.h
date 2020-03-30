// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once
#include "UObject/Object.h"

#include "DlgNodeCondition.generated.h"

/**
 *  Abstract base class for node data
 *  Defining class via inheriting from UNYTaskBase outside of the plugin is possible both in Blueprint and C++
 *  Extend this class to define additional data you want to store on your nodes
 */
UCLASS(Blueprintable, BlueprintType, abstract, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DLGSYSTEM_API UDlgNodeCondition : public UObject
{
	GENERATED_BODY()
public:
	UDlgNodeCondition();

	//Called when the event is triggered. 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue Events")
	bool EnterCondition(APlayerController* PlayerController, UObject* Participant);
	
};


