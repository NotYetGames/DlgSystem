// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgEvent.h"

#include "DlgSystemPrivatePCH.h"
#include "DlgContext.h"
#include "NYReflectionHelper.h"
#include "DlgDialogueParticipant.h"
#include "DlgHelper.h"
#include "Logging/DlgLogger.h"

void FDlgEvent::Call(UDlgContext& Context, UObject* TargetParticipant) const
{
	const bool bHasParticipant = ValidateIsParticipantValid(Context, TEXT("Call"), TargetParticipant);

	// We don't care if it has a participant, but warn nonetheless by calling validate it before this
	if (EventType == EDlgEventType::Custom)
	{
		if (CustomEvent == nullptr)
		{
			FDlgLogger::Get().Warningf(
				TEXT("Custom Event is empty (not valid). Ignoring. Context:\n\t%s, TargetParticipant = %s"),
				*Context.GetContextString(), TargetParticipant ? *TargetParticipant->GetPathName() : TEXT("INVALID")
			);
			return;
		}

		CustomEvent->EnterEvent(&Context, TargetParticipant);
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
			IDlgDialogueParticipant::Execute_OnDialogueEvent(TargetParticipant, &Context, EventName);
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

bool FDlgEvent::ValidateIsParticipantValid(const UDlgContext& Context, const FString& ContextString, const UObject* Participant) const
{
	if (IsValid(Participant))
	{
		return true;
	}

	FDlgLogger::Get().Errorf(
		TEXT("%s Event FAILED because the PARTICIPANT is INVALID. \nContext:\n\t%s, ParticipantName = %s, EventType = %s, EventName = %s"),
		*ContextString, *Context.GetContextString(), *ParticipantName.ToString(), *EventTypeToString(EventType), *EventName.ToString()
	);
	return false;
}

FString FDlgEvent::EventTypeToString(EDlgEventType Type)
{
	FString EnumValue;
	if (FDlgHelper::ConvertEnumToString<EDlgEventType>(TEXT("EDlgEventType"), Type, false, EnumValue))
		return EnumValue;

	return EnumValue;
}
