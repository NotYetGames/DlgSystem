// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgEvent.h"
#include "DlgReflectionHelper.h"
#include "DlgDialogueParticipant.h"

void FDlgEvent::Call(UObject* TargetParticipant) const
{
	if (!IsValid(TargetParticipant))
	{
		return;
	}

	switch (EventType)
	{
	case EDlgEventType::DlgEventEvent:
		IDlgDialogueParticipant::Execute_OnDialogueEvent(TargetParticipant, EventName);
		break;

	case EDlgEventType::DlgEventModifyInt:
		IDlgDialogueParticipant::Execute_ModifyIntValue(TargetParticipant, EventName, bDelta, IntValue);
		break;
	case EDlgEventType::DlgEventModifyFloat:
		IDlgDialogueParticipant::Execute_ModifyFloatValue(TargetParticipant, EventName, bDelta, FloatValue);
		break;
	case EDlgEventType::DlgEventModifyBool:
		IDlgDialogueParticipant::Execute_ModifyBoolValue(TargetParticipant, EventName, bValue);
		break;
	case EDlgEventType::DlgEventModifyName:
		IDlgDialogueParticipant::Execute_ModifyNameValue(TargetParticipant, EventName, NameValue);
		break;

	case EDlgEventType::DlgEventModifyClassIntVariable:
		UDlgReflectionHelper::ModifyVariable<UIntProperty>(TargetParticipant, EventName, IntValue, bDelta);
		break;
	case EDlgEventType::DlgEventModifyClassFloatVariable:
		UDlgReflectionHelper::ModifyVariable<UFloatProperty>(TargetParticipant, EventName, FloatValue, bDelta);
		break;
	case EDlgEventType::DlgEventModifyClassBoolVariable:
		UDlgReflectionHelper::SetVariable<UBoolProperty>(TargetParticipant, EventName, bValue);
		break;
	case EDlgEventType::DlgEventModifyClassNameVariable:
		UDlgReflectionHelper::SetVariable<UNameProperty>(TargetParticipant, EventName, NameValue);
		break;

	default:
		checkNoEntry();
	}
}

FArchive& operator<<(FArchive &Ar, FDlgEvent& DlgEvent)
{
	Ar << DlgEvent.ParticipantName;
	Ar << DlgEvent.EventName;
	Ar << DlgEvent.IntValue;
	Ar << DlgEvent.FloatValue;
	Ar << DlgEvent.NameValue;
	Ar << DlgEvent.bDelta;
	Ar << DlgEvent.bValue;
	Ar << DlgEvent.EventType;
	return Ar;
}
