// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once
#include "DlgNodeCondition.h"
#include "DlgCondition.generated.h"

class IDlgDialogueParticipant;
class UDlgContext;

// Defines the way the condition is interpreted inside a condition array
UENUM()
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
UENUM()
enum class EDlgConditionType : uint8
{
	// A logical operation on a requested int variable acquired via the IDlgParticipant getter function
	IntCall = 0		UMETA(DisplayName = "Check int call"),

	// A logical operation on a requested float variable acquired via the IDlgParticipant getter function
	FloatCall		UMETA(DisplayName = "Check float call"),

	// A logical operation on a requested bool variable acquired via the IDlgParticipant getter function
	BoolCall		UMETA(DisplayName = "Check bool call"),

	// A logical operation on a requested name variable acquired via the IDlgParticipant getter function
	NameCall		UMETA(DisplayName = "Check name call"),

	// A named condition call on the selected Participant
	EventCall		UMETA(DisplayName = "Check named condition"),

	// A logical operation on an variables acquired from the object using the UClass
	ClassIntVariable	UMETA(DisplayName = "Check class int variable"),
	FloatVariable		UMETA(DisplayName = "Check class float variable"),
	ClassBoolVariable	UMETA(DisplayName = "Check class bool variable"),
	ClassNameVariable	UMETA(DisplayName = "Check class name variable"),

	// Status check of the selected node index
	WasNodeVisited		UMETA(DisplayName = "Was node already visited"),

	// Checks if target node has any satisfied child
	HasSatisfiedChild	UMETA(DisplayName = "Has satisfied child")
};

// Operation the return value of a IntCall/FloatCall is checked with
UENUM()
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
UENUM()
enum class EDlgCompare : uint8
{
	ToConst = 0			UMETA(DisplayName = "Compare to Constant"),
	ToVariable			UMETA(DisplayName = "Compare to Variable"),
	ToClassVariable		UMETA(DisplayName = "Compare to Class Variable")
};


// A condition is a logical operation which is evaluated based on a participant or on the local (context based) or global dialogue memory.
// More conditions are stored together in condition arrays in FDlgEdge and in UDlgNode, the node (or the edge's target node) is only visitable
// if the condition array is satisfied
USTRUCT(Blueprintable, BlueprintType)
struct DLGSYSTEM_API FDlgCondition
{
	GENERATED_USTRUCT_BODY()

public:
	bool operator==(const FDlgCondition& Other) const;

	static bool EvaluateArray(const TArray<FDlgCondition>& ConditionsArray, const UDlgContext* Context, FName DefaultParticipantName = NAME_None);
	bool IsConditionMet(const UDlgContext* Context, const UObject* Participant) const;

	bool IsSecondParticipantInvolved() const;

protected:
	//
	// Helper functions doing the check on the primary value based on EDlgCompare
	//

	bool CheckFloat(float Value, const UDlgContext* Context) const;
	bool CheckInt(int32 Value, const UDlgContext* Context) const;
	bool CheckBool(bool bValue, const UDlgContext* Context) const;
	bool CheckName(FName Value, const UDlgContext* Context) const;

	// Checks Participant, prints warning if it is nullptr
	bool ValidateIsParticipantValid(const UObject* Participant, const FString& ContextMessage) const;

	// returns true if ParticipantName has to belong to match with a valid Participant in order for the condition type to work */
	bool IsParticipantInvolved() const;

public:
	// Defines the way the condition is interpreted inside the condition array
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	EDlgConditionStrength Strength = EDlgConditionStrength::Strong;

	// Type of the condition, defines the behavior
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	EDlgConditionType ConditionType = EDlgConditionType::IntCall;

	// Name of the participant (speaker) the event is called on.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	FName ParticipantName;

	// Name of the variable or event, passed in the function call to the participant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	FName CallbackName;

	// The desired operation on the selected variable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	EDlgOperation Operation = EDlgOperation::Equal;

	// Type of value to check against
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	EDlgCompare CompareType = EDlgCompare::ToConst;


	// Name of the other participant (speaker) the check is performed against (with some compare types)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	FName OtherParticipantName;

	// Name of the variable of the other participant the value is checked against (with some compare types)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	FName OtherVariableName;


	// Node index for "node already visited" condition, the value the participant's int is checked against otherwise
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	int32 IntValue = 0;

	// Float the participants float is checked against
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	float FloatValue = 0.f;

	// FName the participants name is checked against
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	FName NameValue;

	// Weather the result defined by the other params has to be true or false in order for this condition to be satisfied
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	bool bBoolValue = true;

	// Weather to check if the node was visited at all (in the long term).
	// Set it to false to check if it was visited in the actual dialogue context
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	bool bLongTermMemory = true;

public:
	// Operator overload for serialization
	friend FArchive& operator<<(FArchive &Ar, FDlgCondition& DlgCondition);
};


USTRUCT(Blueprintable, BlueprintType)
struct DLGSYSTEM_API FDlgCustomCondition
{
	GENERATED_USTRUCT_BODY()

public:
	bool IsValid() const { return Condition != nullptr; }

	static bool EvaluateArray(const TArray<FDlgCustomCondition>& ConditionsArray, const UDlgContext* Context, FName DefaultParticipantName = NAME_None);
	bool IsConditionMet(const UDlgContext* Context, const UObject* Participant) const;

	bool operator==(const FDlgCustomCondition& Other) const;
	friend FArchive& operator<<(FArchive& Ar, FDlgCustomCondition& Condition);

public:
	// Name of the participant (speaker) the condition is passed on
	UPROPERTY(EditAnywhere, Category = DialogueEventData)
	FName ParticipantName;

	// The custom Condition you must extend via blueprint
	UPROPERTY(Instanced, EditAnywhere, BlueprintReadOnly, Category = DialogueConditionData)
	UDlgNodeCondition* Condition;
};
