// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgCondition.h"

#include "DlgSystemPrivatePCH.h"
#include "DlgMemory.h"
#include "Nodes/DlgNode.h"
#include "DlgContextInternal.h"
#include "DlgReflectionHelper.h"

#include "DlgDialogueParticipant.h"
#include "Logging/DlgLogger.h"

bool FDlgCondition::EvaluateArray(const TArray<FDlgCondition>& DlgConditionArray, const UDlgContextInternal* DlgContext, FName DefaultParticipantName)
{
	bool bHasAnyWeak = false;
	bool bHasSuccessfulWeak = false;

	for (const FDlgCondition& Condition : DlgConditionArray)
	{
		const FName ParticipantName = Condition.ParticipantName == NAME_None ? DefaultParticipantName : Condition.ParticipantName;
		const bool bSatisfied = Condition.Evaluate(DlgContext, DlgContext->GetConstParticipant(ParticipantName));
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

bool FDlgCondition::Evaluate(const UDlgContextInternal* DlgContext, const UObject* DlgParticipant) const
{
	if (!IsValid(DlgContext) || (IsParticipantInvolved() && !ValidateIsParticipantValid(DlgParticipant, TEXT("Evaluate"))))
	{
		return false;
	}

	switch (ConditionType)
	{
		case EDlgConditionType::EventCall:
			return IDlgDialogueParticipant::Execute_CheckCondition(DlgParticipant, CallbackName) == bBoolValue;


		case EDlgConditionType::BoolCall:
			return CheckBool(IDlgDialogueParticipant::Execute_GetBoolValue(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::FloatCall:
			return CheckFloat(IDlgDialogueParticipant::Execute_GetFloatValue(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::IntCall:
			return CheckInt(IDlgDialogueParticipant::Execute_GetIntValue(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::NameCall:
			return CheckName(IDlgDialogueParticipant::Execute_GetNameValue(DlgParticipant, CallbackName), DlgContext);


		case EDlgConditionType::ClassBoolVariable:
			return CheckBool(UDlgReflectionHelper::GetVariable<UBoolProperty, bool>(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::FloatVariable:
			return CheckFloat(UDlgReflectionHelper::GetVariable<UFloatProperty, float>(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::ClassIntVariable:
			return CheckInt(UDlgReflectionHelper::GetVariable<UIntProperty, int32>(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::ClassNameVariable:
			return CheckName(UDlgReflectionHelper::GetVariable<UNameProperty, FName>(DlgParticipant, CallbackName), DlgContext);


		case EDlgConditionType::WasNodeVisited:
			if (bLongTermMemory)
			{
				return FDlgMemory::GetInstance()->IsNodeVisited(DlgContext->GetDialogueGuid(), IntValue) == bBoolValue;
			}

			return DlgContext->WasNodeVisitedInThisContext(IntValue) == bBoolValue;

		case EDlgConditionType::HasSatisfiedChild:
			{
				const UDlgNode* Node = DlgContext->GetNode(IntValue);
				return Node != nullptr ? Node->HasAnySatisfiedChild(DlgContext, {}) == bBoolValue : false;
			}

		default:
			checkNoEntry();
			return false;
	}
}

bool FDlgCondition::CheckFloat(float Value, const UDlgContextInternal* DlgContext) const
{
	float ValueToCheckAgainst = FloatValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = DlgContext->GetConstParticipant(OtherParticipantName);
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

bool FDlgCondition::CheckInt(int32 Value, const UDlgContextInternal* DlgContext) const
{
	int32 ValueToCheckAgainst = IntValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = DlgContext->GetConstParticipant(OtherParticipantName);
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

bool FDlgCondition::CheckBool(bool bValue, const UDlgContextInternal* DlgContext) const
{
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = DlgContext->GetConstParticipant(OtherParticipantName);
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

bool FDlgCondition::CheckName(FName Value, const UDlgContextInternal* DlgContext) const
{
	FName ValueToCheckAgainst = NameValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = DlgContext->GetConstParticipant(OtherParticipantName);
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

FArchive& operator<<(FArchive &Ar, FDlgCondition& DlgCondition)
{
	Ar << DlgCondition.Strength;
	Ar << DlgCondition.ParticipantName;
	Ar << DlgCondition.CallbackName;
	Ar << DlgCondition.IntValue;
	Ar << DlgCondition.FloatValue;
	Ar << DlgCondition.NameValue;
	Ar << DlgCondition.bBoolValue;
	Ar << DlgCondition.Operation;
	Ar << DlgCondition.ConditionType;
	Ar << DlgCondition.bLongTermMemory;
	Ar << DlgCondition.CompareType;
	Ar << DlgCondition.OtherParticipantName;
	Ar << DlgCondition.OtherVariableName;
	return Ar;
}
