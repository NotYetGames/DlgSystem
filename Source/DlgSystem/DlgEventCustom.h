// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "DlgObject.h"

#include "DlgEventCustom.generated.h"

class UDlgContext;

// Abstract base class for a custom event
// Extend this class to define additional data you want to store
//
// 1. Override EnterEvent
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew)
class DLGSYSTEM_API UDlgEventCustom : public UDlgObject
{
	GENERATED_BODY()
public:
	// Called when the event is triggered.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue", DisplayName = "Enter")
	void EnterEvent(UDlgContext* Context, UObject* Participant);
	virtual void EnterEvent_Implementation(UDlgContext* Context, UObject* Participant) {}

	// Display text for editor graph node
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Dialogue")
	FString GetEditorDisplayString(UDlgDialogue* OwnerDialogue, FName ParticipantName);
	virtual FString GetEditorDisplayString_Implementation(UDlgDialogue* OwnerDialogue, FName ParticipantName)
	{
		const FString TargetPreFix = ParticipantName != NAME_None ? FString::Printf(TEXT("[%s] "), *ParticipantName.ToString()) : TEXT("");
#if WITH_EDITOR
		return TargetPreFix + GetClass()->GetDisplayNameText().ToString();
#else
		return TargetPreFix + GetName();
#endif
	}
};

// This is the same as UDlgEventCustom but it does NOT show the categories
UCLASS(Blueprintable, BlueprintType, Abstract, EditInlineNew, CollapseCategories)
class DLGSYSTEM_API UDlgEventCustomHideCategories : public UDlgEventCustom
{
	GENERATED_BODY()
};
