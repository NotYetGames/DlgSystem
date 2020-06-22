// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DlgEventCustom.h"

#include "DlgEvent.generated.h"

class UDlgContext;

UENUM(BlueprintType)
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
USTRUCT(BlueprintType)
struct DLGSYSTEM_API FDlgEvent
{
	GENERATED_USTRUCT_BODY()

public:
	//
	// ICppStructOps Interface
	//

	bool operator==(const FDlgEvent& Other) const
	{
		return ParticipantName == Other.ParticipantName &&
			EventName == Other.EventName &&
			IntValue == Other.IntValue &&
			FMath::IsNearlyEqual(FloatValue, Other.FloatValue, KINDA_SMALL_NUMBER) &&
			bDelta == Other.bDelta &&
			bValue == Other.bValue &&
			EventType == Other.EventType &&
			CustomEvent == Other.CustomEvent;
	}

	//
	// Own methods
	//

	// Executes the event
	// TargetParticipant is expected to implement IDlgDialogueParticipant interface
	void Call(UDlgContext& Context, UObject* TargetParticipant) const;

	static FString EventTypeToString(EDlgEventType Type);

protected:
	bool ValidateIsParticipantValid(const UDlgContext& Context, const FString& ContextString, const UObject* Participant) const;

public:
	// Name of the participant (speaker) the event is called on.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
	FName ParticipantName;

	// Type of the event, can be a simple event or a call to modify a bool/int/float variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
	EDlgEventType EventType = EDlgEventType::Event;

	// Name of the relevant variable or event
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
	FName EventName;

	// The value the participant gets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
	int32 IntValue = 0;

	// The value the participant gets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
	float FloatValue = 0.f;

	// The value the participant gets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
	FName NameValue;

	// Weather to add the value to the existing one, or simply override it
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
	bool bDelta = false;

	// The value the participant gets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Event")
	bool bValue = false;

	// The custom Event you must extend via blueprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Dialogue|Event")
	UDlgEventCustom* CustomEvent = nullptr;
};

template<>
struct TStructOpsTypeTraits<FDlgEvent> : public TStructOpsTypeTraitsBase2<FDlgEvent>
{
	enum
	{
		WithIdenticalViaEquality = true
    };
};
