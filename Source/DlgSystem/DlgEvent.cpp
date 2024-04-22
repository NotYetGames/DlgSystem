// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgEvent.h"

#include "DlgConstants.h"
#include "DlgContext.h"
#include "NYReflectionHelper.h"
#include "DlgDialogueParticipant.h"
#include "DlgHelper.h"
#include "Logging/DlgLogger.h"

void FDlgEvent::Call(UDlgContext& Context, const FString& ContextString, UObject* Participant) const
{
	const bool bHasParticipant = ValidateIsParticipantValid(
		Context,
		FString::Printf(TEXT("%s::Call"), *ContextString),
		Participant
	);

	// We don't care if it has a participant, but warn nonetheless by calling validate it before this
	if (EventType == EDlgEventType::Custom)
	{
		if (CustomEvent == nullptr)
		{
			FDlgLogger::Get().Warningf(
				TEXT("Custom Event is empty (not valid). Ignoring. Context:\n\t%s, Participant = %s"),
				*Context.GetContextString(), Participant ? *Participant->GetPathName() : TEXT("INVALID")
			);
			return;
		}

		CustomEvent->EnterEvent(&Context, Participant);
		return;
	}

	// Must have participant from this point onwards
	if (MustHaveParticipant() && !bHasParticipant)
	{
		return;
	}
	switch (EventType)
	{
		case EDlgEventType::Event:
			IDlgDialogueParticipant::Execute_OnDialogueEvent(Participant, &Context, EventName);
			break;
		case EDlgEventType::ModifyInt:
			IDlgDialogueParticipant::Execute_ModifyIntValue(Participant, EventName, bDelta, IntValue);
			break;
		case EDlgEventType::ModifyFloat:
			IDlgDialogueParticipant::Execute_ModifyFloatValue(Participant, EventName, bDelta, FloatValue);
			break;
		case EDlgEventType::ModifyBool:
			IDlgDialogueParticipant::Execute_ModifyBoolValue(Participant, EventName, bValue);
			break;
		case EDlgEventType::ModifyName:
			IDlgDialogueParticipant::Execute_ModifyNameValue(Participant, EventName, NameValue);
			break;

		case EDlgEventType::ModifyClassIntVariable:
			FNYReflectionHelper::ModifyVariable<FIntProperty>(Participant, EventName, IntValue, bDelta);
			break;
		case EDlgEventType::ModifyClassFloatVariable:
			FNYReflectionHelper::ModifyVariable<FDoubleProperty>(Participant, EventName, FloatValue, bDelta);
			break;
		case EDlgEventType::ModifyClassBoolVariable:
			FNYReflectionHelper::SetVariable<FBoolProperty>(Participant, EventName, bValue);
			break;
		case EDlgEventType::ModifyClassNameVariable:
			FNYReflectionHelper::SetVariable<FNameProperty>(Participant, EventName, NameValue);
			break;

		case EDlgEventType::UnrealFunction:
			CallUnrealFunction(Context, ContextString, Participant);
			break;

		default:
			checkNoEntry();
	}
}

FString FDlgEvent::GetEditorDisplayString(UDlgDialogue* OwnerDialogue) const
{
	const FString TargetPreFix = (ParticipantName != NAME_None) ? (FString(TEXT("[")) + ParticipantName.ToString() + FString(TEXT("] "))) : TEXT("");

	auto GetSignCharIfNegative = [](auto Number) -> FString
	{
		return (Number < 0) ? TEXT("-") : TEXT("");
	};
	auto GetSignChar = [](auto Number) -> FString
	{
		return (Number < 0) ? TEXT("-") : TEXT("+");
	};

	auto CreateValueModificationText = [&](const FString& VariableName, auto Number, const FString& ValueAsString) -> FString
	{
			if (bDelta)
			{
				return TargetPreFix + VariableName + TEXT(" ") + GetSignChar(Number) + TEXT("= ") + ValueAsString;
			}
			else
			{
				return TargetPreFix + VariableName + TEXT(" = ") + GetSignCharIfNegative(Number) + ValueAsString;
			}
	};

	switch (EventType)
	{
		case EDlgEventType::Event:
			return TargetPreFix + TEXT("Call DlgEvent ") + EventName.ToString();

		case EDlgEventType::ModifyInt:
			return CreateValueModificationText(EventName.ToString(), IntValue, FString::FromInt(IntValue));

		case EDlgEventType::ModifyFloat:
			return CreateValueModificationText(EventName.ToString(), FloatValue, FString::SanitizeFloat(FloatValue));

		case EDlgEventType::ModifyBool:
			return TargetPreFix + EventName.ToString() + TEXT(" = ") + (bValue ? TEXT("True") : TEXT("False"));

		case EDlgEventType::ModifyName:
			return TargetPreFix + EventName.ToString() + TEXT(" = ") + NameValue.ToString();

		case EDlgEventType::ModifyClassIntVariable:
			return CreateValueModificationText(TEXT("C ") + EventName.ToString(), IntValue, FString::FromInt(IntValue));

		case EDlgEventType::ModifyClassFloatVariable:
			return CreateValueModificationText(TEXT("C ") + EventName.ToString(), FloatValue, FString::SanitizeFloat(FloatValue));

		case EDlgEventType::ModifyClassBoolVariable:
			return TargetPreFix + TEXT("C ") + EventName.ToString() + TEXT(" = ") + (bValue ? TEXT("True") : TEXT("False"));

		case EDlgEventType::ModifyClassNameVariable:
			return TargetPreFix + TEXT("C ") + EventName.ToString() + TEXT(" = ") + NameValue.ToString();

		case EDlgEventType::Custom:
			return CustomEvent == nullptr ? TEXT("Invalid") : CustomEvent->GetEditorDisplayString(OwnerDialogue, ParticipantName);

		case EDlgEventType::UnrealFunction:
			return TargetPreFix + TEXT("Call Function ") + EventName.ToString();

		default:
			return TEXT("TODO");
	}
}

bool FDlgEvent::ValidateIsParticipantValid(const UDlgContext& Context, const FString& ContextString, const UObject* Participant) const
{
	if (IsValid(Participant))
	{
		return true;
	}

	if (MustHaveParticipant())
	{
		FDlgLogger::Get().Errorf(
			TEXT("%s - Event FAILED because the PARTICIPANT is INVALID. \nContext:\n\t%s, \n\tParticipantName = %s, EventType = %s, EventName = %s, CustomEvent = %s"),
			*ContextString, *Context.GetContextString(), *ParticipantName.ToString(), *EventTypeToString(EventType), *EventName.ToString(), *GetCustomEventName()
		);
	}
	else
	{
		FDlgLogger::Get().Warningf(
			TEXT("%s - Event WARNING because the PARTICIPANT is INVALID. The call will NOT FAIL, but the participant is not present. \nContext:\n\t%s, \n\tParticipantName = %s, EventType = %s, EventName = %s, CustomEvent = %s"),
			*ContextString, *Context.GetContextString(), *ParticipantName.ToString(), *EventTypeToString(EventType), *EventName.ToString(), *GetCustomEventName()
		);
	}

	return false;
}

FString FDlgEvent::EventTypeToString(EDlgEventType Type)
{
	FString EnumValue;
	FDlgHelper::ConvertEnumToString<EDlgEventType>(TEXT("EDlgEventType"), Type, false, EnumValue);
	return EnumValue;
}

void FDlgEvent::CallUnrealFunction(UDlgContext& Context, const FString& ContextString, UObject* Participant) const
{
	if (!IsValid(Participant))
	{
		return;
	}

	if (UFunction* Function = Participant->FindFunction(EventName))
	{
		Participant->ProcessEvent(Function, nullptr);
	}
	else
	{
		FDlgLogger::Get().Warningf(
			TEXT("Unreal Function %s Not Found. Ignoring. Context:\n\t%s, Participant = %s"),
			*EventName.ToString(), *Context.GetContextString(), Participant ? *Participant->GetPathName() : TEXT("INVALID")
		);
	}
}
