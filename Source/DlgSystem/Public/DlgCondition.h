// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "DlgCondition.generated.h"

class IDlgDialogueParticipant;

/**
 *  Defines the way the condition is interpreted inside a condition array
 */
UENUM()
enum class EDlgConditionStrength : uint8
{
	/** All strong condition must be satisfied inside the condition array */
	DlgConditionStrengthStrong		UMETA(DisplayName = "Strong Condition"),
	/** At least one of the weak conditions must be satisfied inside the condition array (if there is any) */
	DlgConditionStrengthWeak		UMETA(DisplayName = "Weak Condition"),
};

UENUM()
enum class EDlgConditionType : uint8
{
	/** A logical operation on a requested int variable acquired via the IDlgParticipant getter function */
	DlgConditionIntCall		UMETA(DisplayName = "Check int call"),
	/** A logical operation on a requested float variable acquired via the IDlgParticipant getter function */
	DlgConditionFloatCall	UMETA(DisplayName = "Check float call"),
	/** A logical operation on a requested bool variable acquired via the IDlgParticipant getter function */
	DlgConditionBoolCall	UMETA(DisplayName = "Check bool call"),
	/** A logical operation on a requested name variable acquired via the IDlgParticipant getter function */
	DlgConditionNameCall	UMETA(DisplayName = "Check name call"),

	/** A named condition call on the selected Participant */
	DlgConditionEventCall	UMETA(DisplayName = "Check named condition"),


	/** A logical operation on an variables acquired from the object using the UClass */
	DlgConditionClassIntVariable	UMETA(DisplayName = "Check class int variable"),
	DlgConditionClassFloatVariable	UMETA(DisplayName = "Check class float variable"),
	DlgConditionClassBoolVariable	UMETA(DisplayName = "Check class bool variable"),
	DlgConditionClassNameVariable	UMETA(DisplayName = "Check class name variable"),


	/** Status check of the selected node index */
	DlgConditionNodeVisited UMETA(DisplayName = "Was node already visited"),
	/** Checks if target node has any satisfied child */
	DlgConditionHasSatisfiedChild UMETA(DisplayName = "Has satisfied child")
};

/**
 * Operation the return value of a DlgConditionIntCall/DlgConditionFloatCall is checked with
 */
UENUM()
enum class EDlgOperation : uint8
{
	DlgEqual			UMETA(DisplayName = "Is Equal To"),
	DlgNotEqual			UMETA(DisplayName = "Is Not Equal To"),
	DlgLess				UMETA(DisplayName = "Is Less Than"),
	DlgLessOrEqual		UMETA(DisplayName = "Is Less Than Or Equal To"),
	DlgGreater			UMETA(DisplayName = "Is Greater Than"),
	DlgGreaterOrEqual	UMETA(DisplayName = "Is Greater Than Or Equal To"),
};

/**
 *  A condition is a logical operation which is evaluated based on a participant or on the local (context based) or global dialogue memory.
 *  More conditions are stored together in condition arrays in FDlgEdge and in UDlgNode, the node (or the edge's target node) is only visitable
 *  if the condition array is satisfied
 */
USTRUCT()
struct DLGSYSTEM_API FDlgCondition
{
	GENERATED_USTRUCT_BODY()

public:
	bool operator==(const FDlgCondition& Other) const;

	static bool EvaluateArray(const TArray<FDlgCondition>& DlgConditionArray, class UDlgContextInternal* DlgContext, FName DefaultParticipantName = NAME_None);

	bool Evaluate(class UDlgContextInternal* DlgContext, UObject* DlgParticipant) const;

	bool CheckFloat(float Value) const;
	bool CheckInt(int32 Value) const;

public:

	/** Defines the way the condition is interpreted inside the condition array */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	EDlgConditionStrength Strength;

	/** Type of the condition, defines the behavior */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	EDlgConditionType ConditionType;

	/** Name of the participant (speaker) the event is called on. */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	FName ParticipantName;

	/** Name of the variable or event, passed in the function call to the participant */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	FName CallbackName;

	/** The desired operation on the selected variable */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	EDlgOperation Operation;

	/** Node index for "node already visited" condition, the value the participant's int is checked against otherwise */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	int32 IntValue;

	/** Float the particpant's float is checked against */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	float FloatValue;

	/** FName the particpant's name is checked against */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	FName NameValue;

	/** Weather the result defined by the other params has to be true or false in order for this condition to be satisfied */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	bool bBoolValue = true;

	/** Weather to check if the node was visited at all (in the long term), set it to false to check if it was visited in the actual dialogue context */
	UPROPERTY(EditAnywhere, Category = DlgConditionData)
	bool bLongTermMemory = true;

public:

	// Operator overload for serialization
	friend FArchive& operator<<(FArchive &Ar, FDlgCondition& DlgCondition);
};


FORCEINLINE bool FDlgCondition::operator==(const FDlgCondition& Other) const
{
	return	Strength == Other.Strength &&
			ParticipantName == Other.ParticipantName &&
			CallbackName == Other.CallbackName &&
			IntValue == Other.IntValue &&
			FMath::IsNearlyEqual(FloatValue, Other.FloatValue, KINDA_SMALL_NUMBER) &&
			NameValue == Other.NameValue &&
			bBoolValue == Other.bBoolValue &&
			bLongTermMemory == Other.bLongTermMemory &&
			Operation == Other.Operation &&
			ConditionType == Other.ConditionType;
}
