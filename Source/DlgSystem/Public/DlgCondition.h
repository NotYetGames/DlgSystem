// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "DlgConditionCustom.h"


#include "DlgCondition.generated.h"

class IDlgDialogueParticipant;
class UDlgContext;

// Defines the way the condition is interpreted inside a condition array
UENUM(BlueprintType)
enum class EDlgConditionStrength : uint8
{
	/// All strong condition must be satisfied inside the condition array
	// Aka an AND condition
	Strong = 0		UMETA(DisplayName = "Strong Condition (AND Condition)"),

	// At least one of the weak conditions must be satisfied inside the condition array (if there is any)
	// Aka an OR condition
	Weak			UMETA(DisplayName = "Weak Condition (OR Condition)"),
};

// The Type of condition FDlgCondition is based upon
UENUM(BlueprintType)
enum class EDlgConditionType : uint8
{
	// Calls GetIntValue on the Participant
	IntCall = 0		UMETA(DisplayName = "Check Dialogue Int Value"),

	// Calls GetFloatValue on the Participant
	FloatCall		UMETA(DisplayName = "Check Dialogue Float Value"),

	// Calls GetBoolValue on the Participant
	BoolCall		UMETA(DisplayName = "Check Dialogue Bool Value"),

	// Calls GetNameValue on the Participant
	NameCall		UMETA(DisplayName = "Check Dialogue Name Value"),

	// A named condition call.
	// Calls CheckCondition on the Participant
	EventCall		UMETA(DisplayName = "Check Dialogue Named Condition"),

	// Gets the value from the Participant Int Variable
	ClassIntVariable	UMETA(DisplayName = "Check Class Int Variable"),

	// Gets the value from the Participant Float Variable
	ClassFloatVariable	UMETA(DisplayName = "Check Class Float Variable"),

	// Gets the value from the Participant Bool Variable
	ClassBoolVariable	UMETA(DisplayName = "Check Class Bool Variable"),

	// Gets the value from the Participant Name Variable
	ClassNameVariable	UMETA(DisplayName = "Check Class Name Variable"),

	// Checks if the target node was already visited
	WasNodeVisited		UMETA(DisplayName = "Was node already visited?"),

	// Checks if the target node has any satisfied child
	HasSatisfiedChild	UMETA(DisplayName = "Has satisfied child?"),

	// User Defined Condition, calls IsConditionMet on the custom condition object.
	//
	// 1. Create a new Blueprint derived from DlgConditionCustom (or DlgConditionCustomHideCategories)
	// 2. Override IsConditionMet
	// 3. Return true if you want the condition to succeed or false otherwise
	Custom				UMETA(DisplayName = "Custom Condition")
};

// Operation the return value of a IntCall/FloatCall is checked with
UENUM(BlueprintType)
enum class EDlgOperation : uint8
{
	Equal = 0		UMETA(DisplayName = "== (Is Equal To)"),
	NotEqual		UMETA(DisplayName = "!= (Is Not Equal To)"),
	Less			UMETA(DisplayName = "<  (Is Less Than)"),
	LessOrEqual		UMETA(DisplayName = "<= (Is Less Than Or Equal To)"),
	Greater			UMETA(DisplayName = ">  (Is Greater Than)"),
	GreaterOrEqual	UMETA(DisplayName = ">= (Is Greater Than Or Equal To)"),
};

// Type of value the participant's value is checked against
UENUM(BlueprintType)
enum class EDlgCompare : uint8
{
	// Compares against a constat value
	ToConst = 0			UMETA(DisplayName = "Compare to Constant"),

	// Compares against a Dialogue Value
	ToVariable			UMETA(DisplayName = "Compare to Dialogue Value"),

	// Compares against a Participant Class Variable
	ToClassVariable		UMETA(DisplayName = "Compare to Class Variable")
};


// A condition is a logical operation which is evaluated based on a participant or on the local (context based) or global dialogue memory.
// More conditions are stored together in condition arrays in FDlgEdge and in UDlgNode, the node (or the edge's target node) is only visitable
// if the condition array is satisfied
USTRUCT(Blueprintable)
struct DLGSYSTEM_API FDlgCondition
{
	GENERATED_USTRUCT_BODY()

public:
	//
	// ICppStructOps Interface
	//
	bool operator==(const FDlgCondition& Other) const
	{
		return Strength == Other.Strength &&
			ConditionType == Other.ConditionType &&
			ParticipantName == Other.ParticipantName &&
			CallbackName == Other.CallbackName &&
			IntValue == Other.IntValue &&
			FMath::IsNearlyEqual(FloatValue, Other.FloatValue) &&
			NameValue == Other.NameValue &&
			bBoolValue == Other.bBoolValue &&
			bLongTermMemory == Other.bLongTermMemory &&
			Operation == Other.Operation &&
			CompareType == Other.CompareType &&
			OtherParticipantName == Other.OtherParticipantName &&
			OtherVariableName == Other.OtherVariableName &&
			GUID == Other.GUID &&
			CustomCondition == Other.CustomCondition;
	}

	//
	// Own methods
	//

	static bool EvaluateArray(const UDlgContext& Context, const TArray<FDlgCondition>& ConditionsArray, FName DefaultParticipantName = NAME_None);
	bool IsConditionMet(const UDlgContext& Context, const UObject* Participant) const;

	// returns true if ParticipantName has to belong to match with a valid Participant in order for the condition type to work */
	bool IsParticipantInvolved() const;
	bool IsSecondParticipantInvolved() const;

	// Does this Condition have a IntValue which is in fact a NodeIndex
	static bool HasNodeIndex(EDlgConditionType ConditionType)
	{
		return ConditionType == EDlgConditionType::WasNodeVisited
			|| ConditionType == EDlgConditionType::HasSatisfiedChild;
	}

	// Is this a Condition which has a Dialogue Value
	// NOTE: without EDlgConditionType::EventCall, for that call HasParticipantInterfaceValue
	static bool HasDialogueValue(EDlgConditionType Type)
	{
		return Type == EDlgConditionType::BoolCall
		    || Type == EDlgConditionType::FloatCall
		    || Type == EDlgConditionType::IntCall
		    || Type == EDlgConditionType::NameCall;
	}

	// Same as HasDialogueValue but also Has the Event
	static bool HasParticipantInterfaceValue(EDlgConditionType Type)
	{
		return Type == EDlgConditionType::EventCall || HasDialogueValue(Type);
	}

	// Is this a Condition which has a Class Variable
	static bool HasClassVariable(EDlgConditionType Type)
	{
		return Type == EDlgConditionType::ClassBoolVariable
            || Type == EDlgConditionType::ClassFloatVariable
            || Type == EDlgConditionType::ClassIntVariable
            || Type == EDlgConditionType::ClassNameVariable;
	}

	// Does the type for FirstType and SecondType match?
	// Aka are both for int, float, bool, name?
	static bool IsSameValueType(EDlgConditionType FirstType, EDlgConditionType SecondType)
	{
		if (FirstType == EDlgConditionType::BoolCall || FirstType == EDlgConditionType::ClassBoolVariable)
		{
			return SecondType == EDlgConditionType::BoolCall || SecondType == EDlgConditionType::ClassBoolVariable;
		}
		if (FirstType == EDlgConditionType::IntCall || FirstType == EDlgConditionType::ClassIntVariable)
		{
			return SecondType == EDlgConditionType::IntCall || SecondType == EDlgConditionType::ClassIntVariable;
		}
		if (FirstType == EDlgConditionType::FloatCall || FirstType == EDlgConditionType::ClassFloatVariable)
		{
			return SecondType == EDlgConditionType::FloatCall || SecondType == EDlgConditionType::ClassFloatVariable;
		}
		if (FirstType == EDlgConditionType::NameCall || FirstType == EDlgConditionType::ClassNameVariable)
		{
			return SecondType == EDlgConditionType::NameCall || SecondType == EDlgConditionType::ClassNameVariable;
		}

		return false;
	}

	static FString ConditionTypeToString(EDlgConditionType Type);

protected:
	//
	// Helper functions doing the check on the primary value based on EDlgCompare
	//

	bool CheckFloat(const UDlgContext& Context, float Value) const;
	bool CheckInt(const UDlgContext& Context, int32 Value) const;
	bool CheckBool(const UDlgContext& Context, bool bValue) const;
	bool CheckName(const UDlgContext& Context, FName Value) const;

	// Checks Participant, prints warning if it is nullptr
	bool ValidateIsParticipantValid(const UDlgContext& Context, const FString& ContextString, const UObject* Participant) const;

public:
	// Defines the way the condition is interpreted inside the condition array
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	EDlgConditionStrength Strength = EDlgConditionStrength::Strong;

	// Type of the condition, defines the behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	EDlgConditionType ConditionType = EDlgConditionType::IntCall;

	// Name of the participant (speaker) the event is called on.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	FName ParticipantName;

	// Name of the variable or event, passed in the function call to the participant
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	FName CallbackName;

	// The desired operation on the selected variable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	EDlgOperation Operation = EDlgOperation::Equal;

	// Type of value to check against
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	EDlgCompare CompareType = EDlgCompare::ToConst;

	// Name of the other participant (speaker) the check is performed against (with some compare types)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	FName OtherParticipantName;

	// Name of the variable of the other participant the value is checked against (with some compare types)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	FName OtherVariableName;

	// Node index for "node already visited" condition, the value the participant's int is checked against otherwise
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	int32 IntValue = 0;

	// Float the participants float is checked against
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	float FloatValue = 0.f;

	// FName the participants name is checked against
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	FName NameValue;

	// Weather the result defined by the other params has to be true or false in order for this condition to be satisfied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	bool bBoolValue = true;

	// Weather to check if the node was visited at all (in the long term).
	// Set it to false to check if it was visited in the actual dialogue context
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	bool bLongTermMemory = true;

	// GUID for the Node, used for "node already visited"
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	FGuid GUID;

	// User Defined Condition, calls IsConditionMet on the custom condition object.
	//
	// 1. Create a new Blueprint derived from DlgConditionCustom (or DlgConditionCustomHideCategories)
	// 2. Override IsConditionMet
	// 3. Return true if you want the condition to succeed or false otherwise
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Condition")
	UDlgConditionCustom* CustomCondition = nullptr;
};

template<>
struct TStructOpsTypeTraits<FDlgCondition> : public TStructOpsTypeTraitsBase2<FDlgCondition>
{
	enum
	{
		WithIdenticalViaEquality = true
	};
};
