// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgCondition.h"

#include "DlgConstants.h"
#include "DlgMemory.h"
#include "DlgContext.h"
#include "Nodes/DlgNode.h"
#include "NYReflectionHelper.h"
#include "Kismet/GameplayStatics.h"
#include "DlgDialogueParticipant.h"
#include "DlgHelper.h"
#include "Logging/DlgLogger.h"

bool FDlgCondition::EvaluateArray(const UDlgContext& Context, const TArray<FDlgCondition>& ConditionsArray, FName DefaultParticipantName)
{
	bool bHasAnyWeak = false;
	bool bHasSuccessfulWeak = false;

	for (const FDlgCondition& Condition : ConditionsArray)
	{
		const FName ParticipantName = Condition.ParticipantName == NAME_None ? DefaultParticipantName : Condition.ParticipantName;
		const bool bSatisfied = Condition.IsConditionMet(Context, Context.GetParticipant(ParticipantName));
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

bool FDlgCondition::IsConditionMet(const UDlgContext& Context, const UObject* Participant) const
{
	bool bHasParticipant = true;
	if (IsParticipantInvolved())
	{
		bHasParticipant = ValidateIsParticipantValid(Context, TEXT("IsConditionMet"), Participant);
	}

	// We don't care if it has a participant, but warn nonetheless by calling validate it before this
	if (ConditionType == EDlgConditionType::Custom)
	{
		if (CustomCondition == nullptr)
		{
			FDlgLogger::Get().Errorf(
				TEXT("Custom Condition is empty (not valid). IsConditionMet returning false.\nContext:\n\t%s, Participant = %s"),
				*Context.GetContextString(), Participant ? *Participant->GetPathName() : TEXT("INVALID")
			);
			return false;
		}

		return CustomCondition->IsConditionMet(&Context, Participant);
	}

	// Must have participant from this point onwards
	if (!bHasParticipant)
	{
		return false;
	}
	switch (ConditionType)
	{
		case EDlgConditionType::EventCall:
			return IDlgDialogueParticipant::Execute_CheckCondition(Participant, &Context, CallbackName) == bBoolValue;

		case EDlgConditionType::BoolCall:
			return CheckBool(Context, IDlgDialogueParticipant::Execute_GetBoolValue(Participant, CallbackName));

		case EDlgConditionType::FloatCall:
			return CheckFloat(Context, IDlgDialogueParticipant::Execute_GetFloatValue(Participant, CallbackName));

		case EDlgConditionType::IntCall:
			return CheckInt(Context, IDlgDialogueParticipant::Execute_GetIntValue(Participant, CallbackName));

		case EDlgConditionType::NameCall:
			return CheckName(Context, IDlgDialogueParticipant::Execute_GetNameValue(Participant, CallbackName));


		case EDlgConditionType::ClassBoolVariable:
			return CheckBool(Context, FNYReflectionHelper::GetVariable<FBoolProperty, bool>(Participant, CallbackName));

		case EDlgConditionType::ClassFloatVariable:
			return CheckFloat(Context, FNYReflectionHelper::GetVariable<FFloatProperty, float>(Participant, CallbackName));

		case EDlgConditionType::ClassIntVariable:
			return CheckInt(Context, FNYReflectionHelper::GetVariable<FIntProperty, int32>(Participant, CallbackName));

		case EDlgConditionType::ClassNameVariable:
			return CheckName(Context, FNYReflectionHelper::GetVariable<FNameProperty, FName>(Participant, CallbackName));


		case EDlgConditionType::WasNodeVisited:
			return Context.IsNodeVisited(IntValue, GUID, !bLongTermMemory) == bBoolValue;

		case EDlgConditionType::HasSatisfiedChild:
			{
				// Use the GUID if it is valid as it is more reliable
				const UDlgNode* Node = GUID.IsValid() ? Context.GetNodeFromGUID(GUID) : Context.GetNodeFromIndex(IntValue);
				return Node != nullptr ? Node->HasAnySatisfiedChild(Context, {}) == bBoolValue : false;
			}

		default:
			checkNoEntry();
			return false;
	}
}

bool FDlgCondition::CheckFloat(const UDlgContext& Context, float Value) const
{
	float ValueToCheckAgainst = FloatValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = Context.GetParticipant(OtherParticipantName);
		if (!ValidateIsParticipantValid(Context, TEXT("CheckFloat"), OtherParticipant))
		{
			return false;
		}

		if (CompareType == EDlgCompare::ToVariable)
		{
			ValueToCheckAgainst = IDlgDialogueParticipant::Execute_GetFloatValue(OtherParticipant, OtherVariableName);
		}
		else
		{
			ValueToCheckAgainst = FNYReflectionHelper::GetVariable<FFloatProperty, float>(OtherParticipant, OtherVariableName);
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
			FDlgLogger::Get().Errorf(
				TEXT("Invalid Operation in float based condition.\nContext:\n\t%s"),
				*Context.GetContextString()
			);
			return false;
	}
}

bool FDlgCondition::CheckInt(const UDlgContext& Context, int32 Value) const
{
	int32 ValueToCheckAgainst = IntValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = Context.GetParticipant(OtherParticipantName);
		if (!ValidateIsParticipantValid(Context, TEXT("CheckInt"), OtherParticipant))
		{
			return false;
		}

		if (CompareType == EDlgCompare::ToVariable)
		{
			ValueToCheckAgainst = IDlgDialogueParticipant::Execute_GetIntValue(OtherParticipant, OtherVariableName);
		}
		else
		{
			ValueToCheckAgainst = FNYReflectionHelper::GetVariable<FIntProperty, int32>(OtherParticipant, OtherVariableName);
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
			FDlgLogger::Get().Errorf(
				TEXT("Invalid Operation in int based condition.\nContext:\n\t%s"),
				*Context.GetContextString()
			);
			return false;
	}
}

bool FDlgCondition::CheckBool(const UDlgContext& Context, bool bValue) const
{
	bool bResult = bValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = Context.GetParticipant(OtherParticipantName);
		if (!ValidateIsParticipantValid(Context, TEXT("CheckBool"), OtherParticipant))
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
			bValueToCheckAgainst = FNYReflectionHelper::GetVariable<FBoolProperty, bool>(OtherParticipant, OtherVariableName);
		}

		// Check if value matches other variable
		bResult = bValue == bValueToCheckAgainst;
	}

	return bResult == bBoolValue;
}

bool FDlgCondition::CheckName(const UDlgContext& Context, FName Value) const
{
	FName ValueToCheckAgainst = NameValue;
	if (CompareType == EDlgCompare::ToVariable || CompareType == EDlgCompare::ToClassVariable)
	{
		const UObject* OtherParticipant = Context.GetParticipant(OtherParticipantName);
		if (!ValidateIsParticipantValid(Context, TEXT("CheckName"), OtherParticipant))
		{
			return false;
		}

		if (CompareType == EDlgCompare::ToVariable)
		{
			ValueToCheckAgainst = IDlgDialogueParticipant::Execute_GetNameValue(OtherParticipant, OtherVariableName);
		}
		else
		{
			ValueToCheckAgainst = FNYReflectionHelper::GetVariable<FNameProperty, FName>(OtherParticipant, OtherVariableName);
		}
	}

	const bool bResult = ValueToCheckAgainst == Value;
	return bResult == bBoolValue;
}

bool FDlgCondition::ValidateIsParticipantValid(const UDlgContext& Context, const FString& ContextString, const UObject* Participant) const
{
	if (IsValid(Participant))
	{
		return true;
	}

	FDlgLogger::Get().Errorf(
		TEXT("%s FAILED because the PARTICIPANT is INVALID.\nContext:\n\t%s, ConditionType = %s"),
		*ContextString, *Context.GetContextString(), *ConditionTypeToString(ConditionType)
	);
	return false;
}

bool FDlgCondition::IsParticipantInvolved() const
{
	return ConditionType != EDlgConditionType::HasSatisfiedChild
		&& ConditionType != EDlgConditionType::WasNodeVisited;
}

bool FDlgCondition::IsSecondParticipantInvolved() const
{
	// Second participant requires first participant
	return CompareType != EDlgCompare::ToConst && IsParticipantInvolved();
}

FString FDlgCondition::ConditionTypeToString(EDlgConditionType Type)
{
	FString EnumValue;
	if (FDlgHelper::ConvertEnumToString<EDlgConditionType>(TEXT("EDlgConditionType"), Type, false, EnumValue))
		return EnumValue;

	return EnumValue;
}
