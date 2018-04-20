// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgCondition.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgMemory.h"
#include "DlgNode.h"
#include "DlgContextInternal.h"
#include "DlgReflectionHelper.h"

#include "DlgDialogueParticipant.h"

bool FDlgCondition::EvaluateArray(const TArray<FDlgCondition>& DlgConditionArray, UDlgContextInternal* DlgContext, FName DefaultParticipantName)
{
	bool bHasAnyWeak = false;
	bool bHasSuccessfulWeak = false;

	for (const FDlgCondition& Condition : DlgConditionArray)
	{
		const FName ParticipantName = Condition.ParticipantName == NAME_None ? DefaultParticipantName : Condition.ParticipantName;
		const bool bSatisfied = Condition.Evaluate(DlgContext, DlgContext->GetParticipant(ParticipantName));
		if (Condition.Strength == EDlgConditionStrength::DlgConditionStrengthWeak)
		{
			bHasAnyWeak = true;
			bHasSuccessfulWeak = bHasSuccessfulWeak || bSatisfied;
		}
		else
		{
			if (!bSatisfied)
			{
				return false;
			}
		}
	}

	return (bHasSuccessfulWeak || !bHasAnyWeak);
}

bool FDlgCondition::Evaluate(UDlgContextInternal* DlgContext, UObject* DlgParticipant) const
{
	if (!IsValid(DlgContext) || (IsParticipantInvolved() && !IsParticipantValid(DlgParticipant)))
	{
		return false;
	}
	
	switch (ConditionType)
	{
		case EDlgConditionType::DlgConditionEventCall:
			return IDlgDialogueParticipant::Execute_CheckCondition(DlgParticipant, CallbackName) == bBoolValue;


		case EDlgConditionType::DlgConditionBoolCall:
			return CheckBool(IDlgDialogueParticipant::Execute_GetBoolValue(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::DlgConditionFloatCall:
			return CheckFloat(IDlgDialogueParticipant::Execute_GetFloatValue(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::DlgConditionIntCall:
			return CheckInt(IDlgDialogueParticipant::Execute_GetIntValue(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::DlgConditionNameCall:
			return CheckName(IDlgDialogueParticipant::Execute_GetNameValue(DlgParticipant, CallbackName), DlgContext);


		case EDlgConditionType::DlgConditionClassBoolVariable:
			return CheckBool(UDlgReflectionHelper::GetVariable<UBoolProperty, bool>(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::DlgConditionClassFloatVariable:
			return CheckFloat(UDlgReflectionHelper::GetVariable<UFloatProperty, float>(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::DlgConditionClassIntVariable:
			return CheckInt(UDlgReflectionHelper::GetVariable<UIntProperty, int32>(DlgParticipant, CallbackName), DlgContext);

		case EDlgConditionType::DlgConditionClassNameVariable:
			return CheckName(UDlgReflectionHelper::GetVariable<UNameProperty, FName>(DlgParticipant, CallbackName), DlgContext);


		case EDlgConditionType::DlgConditionNodeVisited:
			if (bLongTermMemory)
			{
				return FDlgMemory::GetInstance()->IsNodeVisited(DlgContext->GetDialogueGuid(), IntValue) == bBoolValue;
			}

			return DlgContext->WasNodeVisitedInThisContext(IntValue) == bBoolValue;

		case EDlgConditionType::DlgConditionHasSatisfiedChild:
			{
				UDlgNode* Node = DlgContext->GetNode(IntValue);
				return Node != nullptr ? Node->HasAnySatisfiedChild(DlgContext, {}) == bBoolValue : false;
			}

		default:
			checkNoEntry();
			return false;
	}
}

bool FDlgCondition::CheckFloat(float Value, UDlgContextInternal* DlgContext) const
{
	float ValueToCheckAgainst = FloatValue;
	if (CompareType == EDlgCompareType::DlgCompareToVariable || CompareType == EDlgCompareType::DlgCompareToClassVariable)
	{
		UObject* OtherParticipant = DlgContext->GetParticipant(OtherParticipantName);
		if (!IsParticipantValid(OtherParticipant))
		{
			return false;
		}

		if (CompareType == EDlgCompareType::DlgCompareToVariable)
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
		case EDlgOperation::DlgEqual:
			return Value == ValueToCheckAgainst;

		case EDlgOperation::DlgGreater:
			return Value > ValueToCheckAgainst;

		case EDlgOperation::DlgGreaterOrEqual:
			return Value >= ValueToCheckAgainst;

		case EDlgOperation::DlgLess:
			return Value < ValueToCheckAgainst;

		case EDlgOperation::DlgLessOrEqual:
			return Value <= ValueToCheckAgainst;

		case EDlgOperation::DlgNotEqual:
			return Value != ValueToCheckAgainst;

		default:
			UE_LOG(LogDlgSystem, Error, TEXT("Invalid Operation in float based condition!"));
			return false;
	}
}

bool FDlgCondition::CheckInt(int32 Value, UDlgContextInternal* DlgContext) const
{
	int32 ValueToCheckAgainst = IntValue;
	if (CompareType == EDlgCompareType::DlgCompareToVariable || CompareType == EDlgCompareType::DlgCompareToClassVariable)
	{
		UObject* OtherParticipant = DlgContext->GetParticipant(OtherParticipantName);
		if (!IsParticipantValid(OtherParticipant))
		{
			return false;
		}

		if (CompareType == EDlgCompareType::DlgCompareToVariable)
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
		case EDlgOperation::DlgEqual:
			return Value == ValueToCheckAgainst;

		case EDlgOperation::DlgGreater:
			return Value > ValueToCheckAgainst;

		case EDlgOperation::DlgGreaterOrEqual:
			return Value >= ValueToCheckAgainst;

		case EDlgOperation::DlgLess:
			return Value < ValueToCheckAgainst;

		case EDlgOperation::DlgLessOrEqual:
			return Value <= ValueToCheckAgainst;

		case EDlgOperation::DlgNotEqual:
			return Value != ValueToCheckAgainst;

		default:
			UE_LOG(LogDlgSystem, Error, TEXT("Invalid Operation in int based condition!"));
			return false;
	}
}

bool FDlgCondition::CheckBool(bool bValue, UDlgContextInternal* DlgContext) const
{
	bool bValueToCheckAgainst = bBoolValue;
	if (CompareType == EDlgCompareType::DlgCompareToVariable || CompareType == EDlgCompareType::DlgCompareToClassVariable)
	{
		UObject* OtherParticipant = DlgContext->GetParticipant(OtherParticipantName);
		if (!IsParticipantValid(OtherParticipant))
		{
			return false;
		}

		if (CompareType == EDlgCompareType::DlgCompareToVariable)
		{
			bValueToCheckAgainst = IDlgDialogueParticipant::Execute_GetBoolValue(OtherParticipant, OtherVariableName);
		}
		else
		{
			bValueToCheckAgainst = UDlgReflectionHelper::GetVariable<UBoolProperty, bool>(OtherParticipant, OtherVariableName);
		}
	}

	return bValue == bValueToCheckAgainst;
}

bool FDlgCondition::CheckName(FName Value, UDlgContextInternal* DlgContext) const
{
	FName ValueToCheckAgainst = NameValue;
	if (CompareType == EDlgCompareType::DlgCompareToVariable || CompareType == EDlgCompareType::DlgCompareToClassVariable)
	{
		UObject* OtherParticipant = DlgContext->GetParticipant(OtherParticipantName);
		if (!IsParticipantValid(OtherParticipant))
		{
			return false;
		}

		if (CompareType == EDlgCompareType::DlgCompareToVariable)
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

bool FDlgCondition::IsParticipantValid(UObject* Participant) const
{
	if (Participant != nullptr)
	{
		return true;
	}

	UE_LOG(LogDlgSystem, Error, TEXT("Condition failed: invalid (nullptr) participant is involved!"));
	return false;
}

bool FDlgCondition::IsParticipantInvolved() const
{
	switch (ConditionType)
	{
		case EDlgConditionType::DlgConditionHasSatisfiedChild:
		case EDlgConditionType::DlgConditionNodeVisited:
			return false;

		default:
			return true;
	}
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
	return Ar;
}
