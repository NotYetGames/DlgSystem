// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgCondition.h"

#include "DlgSystemPrivatePCH.h"
#include "DlgMemory.h"
#include "Nodes/DlgNode.h"
#include "DlgContextInternal.h"
#include "DlgReflectionHelper.h"
#include "Kismet/GameplayStatics.h"
#include "DlgDialogueParticipant.h"
#include "Logging/DlgLogger.h"

bool FDlgCondition::EvaluateArray(const TArray<FDlgCondition>& ConditionsArray, const UDlgContext* Context, FName DefaultParticipantName)
{
	bool bHasAnyWeak = false;
	bool bHasSuccessfulWeak = false;

	for (const FDlgCondition& Condition : ConditionsArray)
	{
		const FName ParticipantName = Condition.ParticipantName == NAME_None ? DefaultParticipantName : Condition.ParticipantName;
		const bool bSatisfied = Condition.IsConditionMet(Context, Context->GetConstParticipant(ParticipantName));
		if (Condition.Strength == EDlgConditionStrength::Weak)
		{
			bHasAnyWeak = true;
			bHasSuccessfulWeak = bHasSuccessfulWeak || bSatisfied;
		}
		else if (!bSatisfied)
		{
			// All must be satisfied
			return false;
		}
	}

	return bHasSuccessfulWeak || !bHasAnyWeak;
}

bool FDlgCondition::IsConditionMet(const UDlgContext* Context, const UObject* Participant) const
{
	if (!IsValid(Context))
	{
		FDlgLogger::Get().Error(TEXT("Condition failed: Dialogue Context is nullptr. How is this even possible???"));
		return false;
	}

	bool bHasParticipant = true;
	if (IsParticipantInvolved())
	{
		bHasParticipant = ValidateIsParticipantValid(Participant, TEXT("IsConditionMet"));
	}

	// We don't care if it has a participant, but warn nonetheless by calling validate it before this
	if (ConditionType == EDlgConditionType::Custom)
	{
		if (CustomCondition == nullptr)
		{
			FDlgLogger::Get().Error(TEXT("Custom Condition is empty (not valid). IsConditionMet returning false."));
			return false;
		}

		return CustomCondition->IsConditionMet(Participant);
	}

	// Must have participant from this point onwards
	if (!bHasParticipant)
	{
		return false;
	}
	switch (ConditionType)
	{
		case EDlgConditionType::EventCall:
			return IDlgDialogueParticipant::Execute_CheckCondition(Participant, CallbackName) == bBoolValue;


		case EDlgConditionType::BoolCall:
			return CheckBool(IDlgDialogueParticipant::Execute_GetBoolValue(Participant, CallbackName), Context);

		case EDlgConditionType::FloatCall:
			return CheckFloat(IDlgDialogueParticipant::Execute_GetFloatValue(Participant, CallbackName), Context);

		case EDlgConditionType::IntCall:
			return CheckInt(IDlgDialogueParticipant::Execute_GetIntValue(Participant, CallbackName), Context);

		case EDlgConditionType::NameCall:
			return CheckName(IDlgDialogueParticipant::Execute_GetNameValue(Participant, CallbackName), Context);


		case EDlgConditionType::ClassBoolVariable:
			return CheckBool(UDlgReflectionHelper::GetVariable<UBoolProperty, bool>(Participant, CallbackName), Context);

		case EDlgConditionType::FloatVariable:
			return CheckFloat(UDlgReflectionHelper::GetVariable<UFloatProperty, float>(Participant, CallbackName), Context);

		case EDlgConditionType::ClassIntVariable:
			return CheckInt(UDlgReflectionHelper::GetVariable<UIntProperty, int32>(Participant, CallbackName), Context);

		case EDlgConditionType::ClassNameVariable:
			return CheckName(UDlgReflectionHelper::GetVariable<UNameProperty, FName>(Participant, CallbackName), Context);


		case EDlgConditionType::WasNodeVisited:
			if (bLongTermMemory)
			{
				return FDlgMemory::GetInstance()->IsNodeVisited(Context->GetDialogueGuid(), IntValue) == bBoolValue;
			}

			return Context->WasNodeVisitedInThisContext(IntValue) == bBoolValue;

		case EDlgConditionType::HasSatisfiedChild:
			{
				const UDlgNode* Node = Context->GetNode(IntValue);
				return Node != nullptr ? Node->HasAnySatisfiedChild(Context, {}) == bBoolValue : false;
			}

		default:
			checkNoEntry();
			return false;
	}
}

bool FDlgCondition::CheckFloat(float Value, const UDlgContext* Context) const
{
	float ValueToCheckAgainst = FloatValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = Context->GetConstParticipant(OtherParticipantName);
		if (!ValidateIsParticipantValid(OtherParticipant, TEXT("CheckFloat")))
		{
			return false;
		}

		if (CompareType == EDlgCompare::ToVariable)
		{
			ValueToCheckAgainst = IDlgDialogueParticipant::Execute_GetFloatValue(OtherParticipant, OtherVariableName);
		}
		else
		{
			ValueToCheckAgainst = UDlgReflectionHelper::GetVariable<UFloatProperty, float>(OtherParticipant, OtherVariableName);
		}
	}

	switch (Operation)
	{
		case EDlgOperation::Equal:
			return FMath::IsNearlyEqual(Value, ValueToCheckAgainst);

		case EDlgOperation::Greater:
			return Value > ValueToCheckAgainst;

		case EDlgOperation::GreaterOrEqual:
			return Value >= ValueToCheckAgainst;

		case EDlgOperation::Less:
			return Value < ValueToCheckAgainst;

		case EDlgOperation::LessOrEqual:
			return Value <= ValueToCheckAgainst;

		case EDlgOperation::NotEqual:
			return !FMath::IsNearlyEqual(Value, ValueToCheckAgainst);

		default:
			FDlgLogger::Get().Error(TEXT("Invalid Operation in float based condition!"));
			return false;
	}
}

bool FDlgCondition::CheckInt(int32 Value, const UDlgContext* Context) const
{
	int32 ValueToCheckAgainst = IntValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = Context->GetConstParticipant(OtherParticipantName);
		if (!ValidateIsParticipantValid(OtherParticipant, TEXT("CheckInt")))
		{
			return false;
		}

		if (CompareType == EDlgCompare::ToVariable)
		{
			ValueToCheckAgainst = IDlgDialogueParticipant::Execute_GetIntValue(OtherParticipant, OtherVariableName);
		}
		else
		{
			ValueToCheckAgainst = UDlgReflectionHelper::GetVariable<UIntProperty, int32>(OtherParticipant, OtherVariableName);
		}
	}

	switch (Operation)
	{
		case EDlgOperation::Equal:
			return Value == ValueToCheckAgainst;

		case EDlgOperation::Greater:
			return Value > ValueToCheckAgainst;

		case EDlgOperation::GreaterOrEqual:
			return Value >= ValueToCheckAgainst;

		case EDlgOperation::Less:
			return Value < ValueToCheckAgainst;

		case EDlgOperation::LessOrEqual:
			return Value <= ValueToCheckAgainst;

		case EDlgOperation::NotEqual:
			return Value != ValueToCheckAgainst;

		default:
			FDlgLogger::Get().Error(TEXT("Invalid Operation in int based condition!"));
			return false;
	}
}

bool FDlgCondition::CheckBool(bool bValue, const UDlgContext* Context) const
{
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = Context->GetConstParticipant(OtherParticipantName);
		if (!ValidateIsParticipantValid(OtherParticipant, TEXT("CheckBool")))
		{
			return false;
		}

		bool bValueToCheckAgainst;
		if (CompareType == EDlgCompare::ToVariable)
		{
			bValueToCheckAgainst = IDlgDialogueParticipant::Execute_GetBoolValue(OtherParticipant, OtherVariableName);
		}
		else
		{
			bValueToCheckAgainst = UDlgReflectionHelper::GetVariable<UBoolProperty, bool>(OtherParticipant, OtherVariableName);
		}

		return (bValue == bValueToCheckAgainst) == bBoolValue;
	}

	return bValue == bBoolValue;
}

bool FDlgCondition::CheckName(FName Value, const UDlgContext* Context) const
{
	FName ValueToCheckAgainst = NameValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = Context->GetConstParticipant(OtherParticipantName);
		if (!ValidateIsParticipantValid(OtherParticipant, TEXT("CheckName")))
		{
			return false;
		}

		if (CompareType == EDlgCompare::ToVariable)
		{
			ValueToCheckAgainst = IDlgDialogueParticipant::Execute_GetNameValue(OtherParticipant, OtherVariableName);
		}
		else
		{
			ValueToCheckAgainst = UDlgReflectionHelper::GetVariable<UNameProperty, FName>(OtherParticipant, OtherVariableName);
		}
	}

	return (ValueToCheckAgainst == Value) == bBoolValue;
}

bool FDlgCondition::ValidateIsParticipantValid(const UObject* Participant, const FString& ContextMessage) const
{
	if (IsValid(Participant))
	{
		return true;
	}

	FDlgLogger::Get().Errorf(
		TEXT("Condition failed: invalid participant! ParticipantName = %s, ConditionName = %s with Context = %s"),
		*ParticipantName.ToString(), *CallbackName.ToString(), *ContextMessage
	);
	return false;
}

bool FDlgCondition::IsParticipantInvolved() const
{
	switch (ConditionType)
	{
		case EDlgConditionType::HasSatisfiedChild:
		case EDlgConditionType::WasNodeVisited:
			return false;

		default:
			return true;
	}
}

bool FDlgCondition::IsSecondParticipantInvolved() const
{
	return ConditionType != EDlgConditionType::WasNodeVisited
		&& ConditionType != EDlgConditionType::HasSatisfiedChild
		&& CompareType != EDlgCompare::ToConst;
}

FArchive& operator<<(FArchive &Ar, FDlgCondition& Condition)
{
	Ar << Condition.Strength;
	Ar << Condition.ParticipantName;
	Ar << Condition.CallbackName;
	Ar << Condition.IntValue;
	Ar << Condition.FloatValue;
	Ar << Condition.NameValue;
	Ar << Condition.bBoolValue;
	Ar << Condition.Operation;
	Ar << Condition.ConditionType;
	Ar << Condition.bLongTermMemory;
	Ar << Condition.CompareType;
	Ar << Condition.OtherParticipantName;
	Ar << Condition.OtherVariableName;
	Ar << Condition.CustomCondition;
	return Ar;
}

bool FDlgCondition::operator==(const FDlgCondition& Other) const
{
	return	Strength == Other.Strength &&
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
			CustomCondition == Other.CustomCondition;
}
