// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgDialogueParticipantData.h"

#include "UObject/DevObjectVersion.h"

#include "DlgEvent.h"
#include "DlgCondition.h"
#include "DlgTextArgument.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgParticipantData::AddConditionPrimaryData(const FDlgCondition& Condition)
{
	const EDlgConditionType ConditionType = Condition.ConditionType;
	const FName ConditionName = Condition.CallbackName;

	switch (ConditionType)
	{
		case EDlgConditionType::EventCall:
			Conditions.Add(ConditionName);
			break;

		case EDlgConditionType::IntCall:
			IntVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::FloatCall:
			FloatVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::BoolCall:
			BoolVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::NameCall:
			NameVariableNames.Add(ConditionName);
			break;

		case EDlgConditionType::ClassIntVariable:
			ClassIntVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::ClassFloatVariable:
			ClassFloatVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::ClassBoolVariable:
			ClassBoolVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::ClassNameVariable:
			ClassNameVariableNames.Add(ConditionName);
			break;

		case EDlgConditionType::Custom:
			if (Condition.CustomCondition)
			{
				CustomConditions.Add(Condition.CustomCondition->GetClass());
			}
			break;

		default:
			break;
	}
}

void FDlgParticipantData::AddConditionSecondaryData(const FDlgCondition& Condition)
{
	const FName VariableName = Condition.OtherVariableName;
	if (Condition.CompareType != EDlgCompare::ToConst)
	{
		const bool bClassVariable = Condition.CompareType == EDlgCompare::ToClassVariable;
		switch (Condition.ConditionType)
		{
		case EDlgConditionType::IntCall:
		case EDlgConditionType::ClassIntVariable:
			(bClassVariable ? ClassIntVariableNames : IntVariableNames).Add(VariableName);
			break;

		case EDlgConditionType::FloatCall:
		case EDlgConditionType::ClassFloatVariable:
			(bClassVariable ? ClassFloatVariableNames : FloatVariableNames).Add(VariableName);
			break;

		case EDlgConditionType::BoolCall:
		case EDlgConditionType::ClassBoolVariable:
			(bClassVariable ? ClassBoolVariableNames : BoolVariableNames).Add(VariableName);
			break;
		case EDlgConditionType::NameCall:
		case EDlgConditionType::ClassNameVariable:
			(bClassVariable ? ClassNameVariableNames : NameVariableNames).Add(VariableName);
			break;

		default:
			break;
		}
	}
}

void FDlgParticipantData::AddEventData(const FDlgEvent& Event)
{
	switch (Event.EventType)
	{
		case EDlgEventType::Event:
			Events.Add(Event.EventName);
			break;

		case EDlgEventType::ModifyInt:
			IntVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::ModifyFloat:
			FloatVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::ModifyBool:
			BoolVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::ModifyName:
			NameVariableNames.Add(Event.EventName);
			break;

		case EDlgEventType::ModifyClassIntVariable:
			ClassIntVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::ModifyClassFloatVariable:
			ClassFloatVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::ModifyClassBoolVariable:
			ClassBoolVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::ModifyClassNameVariable:
			ClassNameVariableNames.Add(Event.EventName);
			break;

		case EDlgEventType::Custom:
			if (Event.CustomEvent)
			{
				CustomEvents.Add(Event.CustomEvent->GetClass());
			}
			break;

		default:
			break;
	}
}

void FDlgParticipantData::AddTextArgumentData(const FDlgTextArgument& TextArgument)
{
	switch (TextArgument.Type)
	{
		case EDlgTextArgumentType::DialogueInt:
			IntVariableNames.Add(TextArgument.VariableName);
			break;
		case EDlgTextArgumentType::DialogueFloat:
			FloatVariableNames.Add(TextArgument.VariableName);

		case EDlgTextArgumentType::ClassInt:
			ClassIntVariableNames.Add(TextArgument.VariableName);
			break;
		case EDlgTextArgumentType::ClassFloat:
			ClassFloatVariableNames.Add(TextArgument.VariableName);
			break;
		case EDlgTextArgumentType::ClassText:
			ClassTextVariableNames.Add(TextArgument.VariableName);
			break;

		case EDlgTextArgumentType::Custom:
			if (TextArgument.CustomTextArgument)
			{
				CustomTextArguments.Add(TextArgument.CustomTextArgument->GetClass());
			}
			break;

		default:
			break;
	}
}
