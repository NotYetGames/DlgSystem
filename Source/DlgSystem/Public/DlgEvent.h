// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "DlgEvent.generated.h"

UENUM()
enum class EDlgEventType : uint8
{
	/** just a notification with an FName parameter */
	DlgEventEvent						UMETA(DisplayName = "Event"),

	/** events to modify basic variable types. Calls the interface methods */
	DlgEventModifyInt					UMETA(DisplayName = "Modify Int"),
	DlgEventModifyFloat					UMETA(DisplayName = "Modify Float"),
	DlgEventModifyBool					UMETA(DisplayName = "Modify Bool"),
	DlgEventModifyName					UMETA(DisplayName = "Modify Name"),

	/** events to modify the variable of the participant UObject by using its UClass */
	DlgEventModifyClassIntVariable		UMETA(DisplayName = "Modify class int variable"),
	DlgEventModifyClassFloatVariable	UMETA(DisplayName = "Modify class float variable"),
	DlgEventModifyClassBoolVariable		UMETA(DisplayName = "Modify class bool variable"),
	DlgEventModifyClassNameVariable		UMETA(DisplayName = "Modify class name variable")
};


/**
 *  Events are executed via calling IDlgDialogueParticipant methods on dialogue participants
 *  They must be handled in game side, can be used to modify game state based on dialogue
 */
USTRUCT()
struct DLGSYSTEM_API FDlgEvent
{
	GENERATED_USTRUCT_BODY()

public:
	bool operator==(const FDlgEvent& Other) const;

	/**
	 * Executes the event
	 * TargetParticipant is expected to implement IDlgDialogueParticipant interface
	 */
	void Call(UObject* TargetParticipant) const;

protected:
	bool ValidateIsParticipantValid(const UObject* Participant) const;

public:

	/** Name of the participant (speaker) the event is called on. */
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	FName ParticipantName;

	/** Type of the event, can be a simple event or a call to modify a bool/int/float variable */
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	EDlgEventType EventType = EDlgEventType::DlgEventEvent;

	/** Name of the relevant variable or event */
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	FName EventName;

	/** The value the participant gets */
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	int32 IntValue = 0;

	/** The value the participant gets */
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	float FloatValue = 0.f;

	/** The value the participant gets */
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	FName NameValue;

	/** Weather to add the value to the existing one, or simply override it  */
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	bool bDelta = false;

	/** The value the participant gets */
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	bool bValue = false;

public:

	// Operator overload for serialization
	friend FArchive& operator<<(FArchive &Ar, FDlgEvent& DlgEvent);
};


FORCEINLINE bool FDlgEvent::operator==(const FDlgEvent& Other) const
{
	return ParticipantName == Other.ParticipantName &&
		   EventName == Other.EventName &&
		   IntValue == Other.IntValue &&
		   FMath::IsNearlyEqual(FloatValue, Other.FloatValue, KINDA_SMALL_NUMBER) &&
		   bDelta == Other.bDelta &&
		   bValue == Other.bValue &&
		   EventType == Other.EventType;
}
