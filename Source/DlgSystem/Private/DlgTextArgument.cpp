// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgTextArgument.h"

#include "UObject/TextProperty.h"

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

FFormatArgumentValue FDlgTextArgument::ConstructFormatArgumentValue(const UDlgContextInternal* DlgContext, FName NodeOwner) const
{
	const UObject* Participant = DlgContext->GetConstParticipant(ParticipantName);
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

void FDlgTextArgument::UpdateTextArgumentArray(const FText& Text, TArray<FDlgTextArgument>& InOutArgumentArray)
{
	TArray<FString> NewArgumentParams;
	FText::GetFormatPatternParameters(Text, NewArgumentParams);

	TArray<FDlgTextArgument> OldArguments = InOutArgumentArray;
	InOutArgumentArray.Empty();

	for (const FString& String : NewArgumentParams)
	{
		FDlgTextArgument Argument;
		Argument.DisplayString = String;

		// Replace with old argument values if display string matches
		for (const FDlgTextArgument& OldArgument : OldArguments)
		{
			if (String == OldArgument.DisplayString)
			{
				Argument = OldArgument;
				break;
			}
		}

		InOutArgumentArray.Add(Argument);
	}
}
