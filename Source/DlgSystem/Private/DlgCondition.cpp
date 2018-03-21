// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgCondition.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgMemory.h"
#include "DlgNode.h"
#include "DlgContextInternal.h"

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
	if (DlgContext == nullptr)
	{
		return false;
	}

	auto CheckParticipant = [](UObject* DlgParticipantParam) -> bool
	{
		if (DlgParticipantParam != nullptr)
		{
			return true;
		}

		UE_LOG(LogDlgSystem, Error, TEXT("Condition failed: called on invalid (null) participant!"));
		return false;
	};

	switch (ConditionType)
	{
		case EDlgConditionType::DlgConditionEventCall:
			return CheckParticipant(DlgParticipant) && IDlgDialogueParticipant::Execute_CheckCondition(DlgParticipant, CallbackName) == bBoolValue;

		case EDlgConditionType::DlgConditionBoolCall:
			return CheckParticipant(DlgParticipant) && IDlgDialogueParticipant::Execute_GetBoolValue(DlgParticipant, CallbackName) == bBoolValue;

		case EDlgConditionType::DlgConditionFloatCall:
			return CheckParticipant(DlgParticipant) && CheckFloat(IDlgDialogueParticipant::Execute_GetFloatValue(DlgParticipant, CallbackName));

		case EDlgConditionType::DlgConditionIntCall:
			return CheckParticipant(DlgParticipant) && CheckInt(IDlgDialogueParticipant::Execute_GetIntValue(DlgParticipant, CallbackName));

		case EDlgConditionType::DlgConditionNameCall:
			return CheckParticipant(DlgParticipant) && (IDlgDialogueParticipant::Execute_GetNameValue(DlgParticipant, CallbackName) == NameValue) == bBoolValue;

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

bool FDlgCondition::CheckFloat(float Value) const
{
	switch (Operation)
	{
		case EDlgOperation::DlgEqual:
			return Value == FloatValue;

		case EDlgOperation::DlgGreater:
			return Value > FloatValue;

		case EDlgOperation::DlgGreaterOrEqual:
			return Value >= FloatValue;

		case EDlgOperation::DlgLess:
			return Value < FloatValue;

		case EDlgOperation::DlgLessOrEqual:
			return Value <= FloatValue;

		case EDlgOperation::DlgNotEqual:
			return Value != FloatValue;

		default:
			UE_LOG(LogDlgSystem, Error, TEXT("Invalid Operation in float based condition!"));
			return false;
	}
}

bool FDlgCondition::CheckInt(int32 Value) const
{
	switch (Operation)
	{
		case EDlgOperation::DlgEqual:
			return Value == IntValue;

		case EDlgOperation::DlgGreater:
			return Value > IntValue;

		case EDlgOperation::DlgGreaterOrEqual:
			return Value >= IntValue;

		case EDlgOperation::DlgLess:
			return Value < IntValue;

		case EDlgOperation::DlgLessOrEqual:
			return Value <= IntValue;

		case EDlgOperation::DlgNotEqual:
			return Value != IntValue;

		default:
			UE_LOG(LogDlgSystem, Error, TEXT("Invalid Operation in int based condition!"));
			return false;
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
