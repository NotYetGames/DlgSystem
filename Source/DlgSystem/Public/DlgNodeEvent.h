// Copyright 2017-2019 Csaba Molnar, Daniel Butum
#pragma once
#include "UObject/Object.h"

#include "DlgNodeEvent.generated.h"

/**
 *  Abstract base class for node data
 *  Defining class via inheriting from UNYTaskBase outside of the plugin is possible both in Blueprint and C++
 *  Extend this class to define additional data you want to store on your nodes
 */
UCLASS(Blueprintable, BlueprintType, abstract, EditInlineNew, HideCategories = ("DoNotShow"), CollapseCategories, AutoExpandCategories = ("Default"))
class DLGSYSTEM_API UDlgNodeEvent : public UObject
{
	GENERATED_BODY()
public:
	UDlgNodeEvent();

	//Called when the event is triggered. 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue Events")
	void EnterEvent(APlayerController* PlayerController, UObject* Participant);
	
};


