// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"

#include "DlgDialogueParticipant.generated.h"

class UTexture2D;
class UDlgContext;

UINTERFACE(BlueprintType, Blueprintable)
class DLGSYSTEM_API UDlgDialogueParticipant : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};
inline UDlgDialogueParticipant::UDlgDialogueParticipant(const FObjectInitializer& ObjectInitializer) {}

/**
 * Interface that each Dialogue participant must implement
 */
class DLGSYSTEM_API IDlgDialogueParticipant
{
	GENERATED_IINTERFACE_BODY()

	//
	// Participant information
	//

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant")
	FName GetParticipantName() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant")
	FText GetParticipantDisplayName(FName ActiveSpeaker) const;

	/** May be used for formatted node texts, check https://docs.unrealengine.com/en-us/Gameplay/Localization/Formatting for more information */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant")
	ETextGender GetParticipantGender() const;

	/**
	* @param	ActiveSpeaker: name of the active speaker at the time of the call (might or might not this participant)
	* @param	ActiveSpeakerState: state of the active participant (might or might not belong to this participant)
	*			If it is not displayed in editor it has to be turned on in the dialogue settings
	* @return	Participant icon to display
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant")
	UTexture2D* GetParticipantIcon(FName ActiveSpeaker, FName ActiveSpeakerState) const;

	//
	// Conditions
	//

	// Used for the condition type EDlgConditionType::EventCall (Check Dialogue Named Condition)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Condition")
	bool CheckCondition(const UDlgContext* Context, FName ConditionName) const;

	// Used for the condition type EDlgConditionType::FloatCall (Check Dialogue Float Value)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Condition")
	float GetFloatValue(FName ValueName) const;

	// Used for the condition type EDlgConditionType::IntCall (Check Dialogue Int Value)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Condition")
	int32 GetIntValue(FName ValueName) const;

	// Used for the condition type EDlgConditionType::BoolCall (Check Dialogue Bool Value)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Condition")
	bool GetBoolValue(FName ValueName) const;

	// Used for the condition type EDlgConditionType::NameCall (Check Dialogue Name Value)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Condition")
	FName GetNameValue(FName ValueName) const;

	//
	// Events
	//

	// Used for the event type EDlgEventType::Event (Event)
	// @return value: irrelevant
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Event")
	bool OnDialogueEvent(UDlgContext* Context, FName EventName);

	// Used for the event type EDlgEventType::ModifyFloat (Modify Dialogue Float Value)
	// @param	bDelta Whether we expect the value to be set or modified
	// @return	Irrelevant, ignored
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Event")
	bool ModifyFloatValue(FName ValueName, bool bDelta, float Value);

	// Used for the event type EDlgEventType::ModifyInt (Modify Dialogue Int Value)
	// @param	bDelta Whether we expect the value to be set or modified
	// @return	Irrelevant, ignored
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Event")
	bool ModifyIntValue(FName ValueName, bool bDelta, int32 Value);

	// Used for the event type EDlgEventType::ModifyBool (Modify Dialogue Bool Value)
	// @return value: irrelevant
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Event")
	bool ModifyBoolValue(FName ValueName, bool bNewValue);

	// Used for the event type EDlgEventType::ModifyName (Modify Dialogue Name Value)
	// @return value: irrelevant
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dialogue|Participant|Event")
	bool ModifyNameValue(FName ValueName, FName NameValue);
};
