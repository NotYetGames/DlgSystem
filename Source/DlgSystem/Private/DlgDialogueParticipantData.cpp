// Copyright 2017-2018 Csaba Molnar, Daniel Butum
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
		case EDlgConditionType::DlgConditionEventCall:
			Conditions.Add(ConditionName);
			break;

		case EDlgConditionType::DlgConditionIntCall:
			IntVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::DlgConditionFloatCall:
			FloatVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::DlgConditionBoolCall:
			BoolVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::DlgConditionNameCall:
			NameVariableNames.Add(ConditionName);
			break;

		case EDlgConditionType::DlgConditionClassIntVariable:
			ClassIntVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::DlgConditionClassFloatVariable:
			ClassFloatVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::DlgConditionClassBoolVariable:
			ClassBoolVariableNames.Add(ConditionName);
			break;
		case EDlgConditionType::DlgConditionClassNameVariable:
			ClassNameVariableNames.Add(ConditionName);
			break;

		default:
			break;
	}
}

void FDlgParticipantData::AddConditionSecondaryData(const FDlgCondition& Condition)
{
	const FName VariableName = Condition.OtherVariableName;
	if (Condition.CompareType != EDlgCompareType::DlgCompareToConst)
	{
		const bool bClassVariable = Condition.CompareType == EDlgCompareType::DlgCompareToClassVariable;
		switch (Condition.ConditionType)
		{
		case EDlgConditionType::DlgConditionIntCall:
		case EDlgConditionType::DlgConditionClassIntVariable:
			(bClassVariable ? ClassIntVariableNames : IntVariableNames).Add(VariableName);
			break;

		case EDlgConditionType::DlgConditionFloatCall:
		case EDlgConditionType::DlgConditionClassFloatVariable:
			(bClassVariable ? ClassFloatVariableNames : FloatVariableNames).Add(VariableName);
			break;

		case EDlgConditionType::DlgConditionBoolCall:
		case EDlgConditionType::DlgConditionClassBoolVariable:
			(bClassVariable ? ClassBoolVariableNames : BoolVariableNames).Add(VariableName);
			break;
		case EDlgConditionType::DlgConditionNameCall:
		case EDlgConditionType::DlgConditionClassNameVariable:
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
		case EDlgEventType::DlgEventEvent:
			Events.Add(Event.EventName);
			break;

		case EDlgEventType::DlgEventModifyInt:
			IntVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::DlgEventModifyFloat:
			FloatVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::DlgEventModifyBool:
			BoolVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::DlgEventModifyName:
			NameVariableNames.Add(Event.EventName);
			break;

		case EDlgEventType::DlgEventModifyClassIntVariable:
			ClassIntVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::DlgEventModifyClassFloatVariable:
			ClassFloatVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::DlgEventModifyClassBoolVariable:
			ClassBoolVariableNames.Add(Event.EventName);
			break;
		case EDlgEventType::DlgEventModifyClassNameVariable:
			ClassNameVariableNames.Add(Event.EventName);
			break;

		default:
			break;
	}
}

void FDlgParticipantData::AddTextArgumentData(const FDlgTextArgument& TextArgument)
{
	switch (TextArgument.Type)
	{
		case EDlgTextArgumentType::DlgTextArgumentDialogueInt:
			IntVariableNames.Add(TextArgument.VariableName);
			break;

		case EDlgTextArgumentType::DlgTextArgumentClassInt:
			ClassIntVariableNames.Add(TextArgument.VariableName);
			break;

		case EDlgTextArgumentType::DlgTextArgumentDialogueFloat:
			FloatVariableNames.Add(TextArgument.VariableName);

		case EDlgTextArgumentType::DlgTextArgumentClassFloat:
			ClassFloatVariableNames.Add(TextArgument.VariableName);
			break;

		case EDlgTextArgumentType::DlgTextArgumentClassText:
			ClassTextVariableNames.Add(TextArgument.VariableName);
			break;

		default:
			break;
	}
}
