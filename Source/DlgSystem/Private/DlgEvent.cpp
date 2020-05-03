// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgEvent.h"

#include "DlgSystemPrivatePCH.h"
#include "NYReflectionHelper.h"
#include "DlgDialogueParticipant.h"
#include "Logging/DlgLogger.h"

void FDlgEvent::Call(UDlgContext* Context, UObject* TargetParticipant) const
{
	const bool bHasParticipant = ValidateIsParticipantValid(TargetParticipant);

	// We don't care if it has a participant, but warn nonethelss by calling validate it before this
	if (EventType == EDlgEventType::Custom)
	{
		if (CustomEvent == nullptr)
		{
			FDlgLogger::Get().Warning(TEXT("Custom Event is empty (not valid). Ignoring"));
			return;
		}

		CustomEvent->EnterEvent(Context, TargetParticipant);
		return;
	}

	// Must have participant from this point onwards
	if (!bHasParticipant)
	{
		return;
	}
	switch (EventType)
	{
	case EDlgEventType::Event:
		IDlgDialogueParticipant::Execute_OnDialogueEvent(TargetParticipant, Context, EventName);
		break;

	case EDlgEventType::ModifyInt:
		IDlgDialogueParticipant::Execute_ModifyIntValue(TargetParticipant, EventName, bDelta, IntValue);
		break;
	case EDlgEventType::ModifyFloat:
		IDlgDialogueParticipant::Execute_ModifyFloatValue(TargetParticipant, EventName, bDelta, FloatValue);
		break;
	case EDlgEventType::ModifyBool:
		IDlgDialogueParticipant::Execute_ModifyBoolValue(TargetParticipant, EventName, bValue);
		break;
	case EDlgEventType::ModifyName:
		IDlgDialogueParticipant::Execute_ModifyNameValue(TargetParticipant, EventName, NameValue);
		break;

	case EDlgEventType::ModifyClassIntVariable:
		FNYReflectionHelper::ModifyVariable<FNYIntProperty>(TargetParticipant, EventName, IntValue, bDelta);
		break;
	case EDlgEventType::ModifyClassFloatVariable:
		FNYReflectionHelper::ModifyVariable<FNYFloatProperty>(TargetParticipant, EventName, FloatValue, bDelta);
		break;
	case EDlgEventType::ModifyClassBoolVariable:
		FNYReflectionHelper::SetVariable<FNYBoolProperty>(TargetParticipant, EventName, bValue);
		break;
	case EDlgEventType::ModifyClassNameVariable:
		FNYReflectionHelper::SetVariable<FNYNameProperty>(TargetParticipant, EventName, NameValue);
		break;

	default:
		checkNoEntry();
	}
}

bool FDlgEvent::ValidateIsParticipantValid(const UObject* Participant) const
{
	if (IsValid(Participant))
	{
		return true;
	}

	FDlgLogger::Get().Errorf(
		TEXT("Event failed: invalid participant! ParticipantName = %s, EventName = %s"),
		*ParticipantName.ToString(), *EventName.ToString()
	);
	return false;
}

FArchive& operator<<(FArchive& Ar, FDlgEvent& Event)
{
	Ar << Event.ParticipantName;
	Ar << Event.EventName;
	Ar << Event.IntValue;
	Ar << Event.FloatValue;
	Ar << Event.NameValue;
	Ar << Event.bDelta;
	Ar << Event.bValue;
	Ar << Event.EventType;
	Ar << Event.CustomEvent;
	return Ar;
}

bool FDlgEvent::operator==(const FDlgEvent& Other) const
{
	return ParticipantName == Other.ParticipantName &&
		   EventName == Other.EventName &&
		   IntValue == Other.IntValue &&
		   FMath::IsNearlyEqual(FloatValue, Other.FloatValue, KINDA_SMALL_NUMBER) &&
		   bDelta == Other.bDelta &&
		   bValue == Other.bValue &&
		   EventType == Other.EventType &&
		   CustomEvent == Other.CustomEvent;
}
