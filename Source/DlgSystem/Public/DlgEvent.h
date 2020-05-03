// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DlgEventCustom.h"

#include "DlgEvent.generated.h"

class UDlgContext;

UENUM()
enum class EDlgEventType : uint8
{
	// Just a notification with an FName parameter on the Participant
	Event						UMETA(DisplayName = "Event"),

	// Events to modify basic variable types. Calls the interface methods on the Participant
	ModifyInt					UMETA(DisplayName = "Modify Int"),
	ModifyFloat					UMETA(DisplayName = "Modify Float"),
	ModifyBool					UMETA(DisplayName = "Modify Bool"),
	ModifyName					UMETA(DisplayName = "Modify Name"),

	// Events to modify the variable of the participant UObject by using its UClass
	ModifyClassIntVariable		UMETA(DisplayName = "Modify class Int variable"),
	ModifyClassFloatVariable	UMETA(DisplayName = "Modify class Float variable"),
	ModifyClassBoolVariable		UMETA(DisplayName = "Modify class Bool variable"),
	ModifyClassNameVariable		UMETA(DisplayName = "Modify class Name variable"),

	// User Defined
	Custom						UMETA(DisplayName = "Custom Event")
};


// Events are executed via calling IDlgDialogueParticipant methods on dialogue participants
// They must be handled in game side, can be used to modify game state based on dialogue
USTRUCT(Blueprintable, BlueprintType)
struct DLGSYSTEM_API FDlgEvent
{
	GENERATED_USTRUCT_BODY()

public:
	// Executes the event
	// TargetParticipant is expected to implement IDlgDialogueParticipant interface
	void Call(UDlgContext* Context, UObject* TargetParticipant) const;

	bool operator==(const FDlgEvent& Other) const;
	friend FArchive& operator<<(FArchive &Ar, FDlgEvent& Event);

protected:
	bool ValidateIsParticipantValid(const UObject* Participant) const;

public:
	// Name of the participant (speaker) the event is called on.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueEventData)
	FName ParticipantName;

	// Type of the event, can be a simple event or a call to modify a bool/int/float variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueEventData)
	EDlgEventType EventType = EDlgEventType::Event;

	// Name of the relevant variable or event
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueEventData)
	FName EventName;

	// The value the participant gets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueEventData)
	int32 IntValue = 0;

	// The value the participant gets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueEventData)
	float FloatValue = 0.f;

	// The value the participant gets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueEventData)
	FName NameValue;

	// Weather to add the value to the existing one, or simply override it
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueEventData)
	bool bDelta = false;

	// The value the participant gets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DialogueEventData)
	bool bValue = false;

	// The custom Event you must extend via blueprint
	UPROPERTY(EditAnywhere, Instanced, Category = DialogueEventData)
	UDlgEventCustom* CustomEvent;
};
