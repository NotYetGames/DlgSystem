// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "DlgEvent.generated.h"

UENUM()
enum class EDlgEventType : uint8
{
	/** just a notification with an FName parameter */
	DlgEventEvent		UMETA(DisplayName = "Event"),

	/** events to modify basic variable types */
	DlgEventModifyInt	UMETA(DisplayName = "Modify Int"),
	DlgEventModifyFloat UMETA(DisplayName = "Modify Float"),
	DlgEventModifyBool  UMETA(DisplayName = "Modify Bool"),
	DlgEventModifyName  UMETA(DisplayName = "Modify Name")
};


/**
 *  Events are executed via calling IDlgDialogueParticipant methods on dialogue participants
 *  They must be handled in game side, can be used to modify game state based on dialogue
 */
USTRUCT()
struct FDlgEvent
{
	GENERATED_USTRUCT_BODY()

public:
	bool operator==(const FDlgEvent& Other) const;

	/**
	 *  Executes the event
	 *  TargetParticipant is expected to implement IDlgDialogueParticipant interface
	 */
	void Call(UObject* TargetParticipant) const;

public:

	/** Name of the participant (speaker) the event is called on. */
	UPROPERTY(EditAnywhere)
	FName ParticipantName;

	/** Type of the event, can be a simple event or a call to modify a bool/int/float variable */
	UPROPERTY(EditAnywhere)
	EDlgEventType EventType;

	/** Name of the relevant variable or event */
	UPROPERTY(EditAnywhere)
	FName EventName;

	/** The value the participant gets */
	UPROPERTY(EditAnywhere)
	int32 IntValue;

	/** The value the participant gets */
	UPROPERTY(EditAnywhere)
	float FloatValue;

	/** The value the participant gets */
	UPROPERTY(EditAnywhere)
	FName NameValue;

	/** Weather to add the value to the existing one, or simply override it  */
	UPROPERTY(EditAnywhere)
	bool bDelta;

	/** The value the participant gets */
	UPROPERTY(EditAnywhere)
	bool bValue;

public:

	// Operator overload for serialization
	friend FArchive& operator<<(FArchive &Ar, FDlgEvent& DlgEvent);
};


FORCEINLINE bool FDlgEvent::operator==(const FDlgEvent& Other) const
{
	return ParticipantName == Other.ParticipantName &&
		   EventName == Other.EventName &&
		   IntValue == Other.IntValue &&
		   FMath::IsNearlyEqual(FloatValue, Other.FloatValue) &&
		   bDelta == Other.bDelta &&
		   bValue == Other.bValue &&
		   EventType == Other.EventType;
}
