// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgTextArgument.h"
#include "DlgSystemPrivatePCH.h"
#include "DlgContextInternal.h"
#include "DlgDialogueParticipant.h"
#include "DlgReflectionHelper.h"


FArchive& operator<<(FArchive &Ar, FDlgTextArgument& DlgCondition)
{
	Ar << DlgCondition.DisplayString;
	Ar << DlgCondition.Type;
	Ar << DlgCondition.ParticipantName;
	Ar << DlgCondition.VariableName;
	return Ar;
}

FFormatArgumentValue FDlgTextArgument::ConstructFormatArgumentValue(class UDlgContextInternal* DlgContext, FName NodeOwner) const
{
	UObject* Participant = DlgContext->GetParticipant(ParticipantName);
	if (Participant == nullptr)
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Failed to construct text argument %s: invalid owner name %s"), *DisplayString, *ParticipantName.ToString());
		return FFormatArgumentValue(0);
	}

	switch (Type)
	{
		case EDlgTextArgumentType::DlgTextArgumentDialogueInt:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetIntValue(Participant, VariableName));

		case EDlgTextArgumentType::DlgTextArgumentClassInt:
			return FFormatArgumentValue(UDlgReflectionHelper::GetVariable<UIntProperty, int32>(Participant, VariableName));

		case EDlgTextArgumentType::DlgTextArgumentDialogueFloat:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetFloatValue(Participant, VariableName));

		case EDlgTextArgumentType::DlgTextArgumentClassFloat:
			return FFormatArgumentValue(UDlgReflectionHelper::GetVariable<UFloatProperty, float>(Participant, VariableName));

		case EDlgTextArgumentType::DlgTextArgumentClassText:
			return FFormatArgumentValue(UDlgReflectionHelper::GetVariable<UTextProperty, FText>(Participant, VariableName));

		case EDlgTextArgumentType::DlgTextArgumentDisplayName:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetParticipantDisplayName(Participant, NodeOwner));

		case EDlgTextArgumentType::DlgTextArgumentGender:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetParticipantGender(Participant));

		default:
			checkNoEntry();
			return FFormatArgumentValue(0);
	}
}
