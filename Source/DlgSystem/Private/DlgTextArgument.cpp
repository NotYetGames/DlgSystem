// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgTextArgument.h"

#include "UObject/TextProperty.h"

#include "DlgSystemPrivatePCH.h"
#include "DlgContextInternal.h"
#include "DlgDialogueParticipant.h"
#include "DlgReflectionHelper.h"
#include "Logging/DlgLogger.h"


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
	// If participant name is not valid we use the node owner name
	const FName ValidParticipantName = ParticipantName == NAME_None ? NodeOwner : ParticipantName;

	const UObject* Participant = DlgContext->GetConstParticipant(ValidParticipantName);
	if (Participant == nullptr)
	{
		FDlgLogger::Get().Errorf(
			TEXT("Failed to construct text argument = %s, invalid owner name = %s, NodeOwner = %s"),
			*DisplayString, *ValidParticipantName.ToString(), *NodeOwner.ToString()
		);
		return FFormatArgumentValue(0);
	}

	switch (Type)
	{
		case EDlgTextArgumentType::DialogueInt:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetIntValue(Participant, VariableName));

		case EDlgTextArgumentType::ClassInt:
			return FFormatArgumentValue(UDlgReflectionHelper::GetVariable<UIntProperty, int32>(Participant, VariableName));

		case EDlgTextArgumentType::DialogueFloat:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetFloatValue(Participant, VariableName));

		case EDlgTextArgumentType::ClassFloat:
			return FFormatArgumentValue(UDlgReflectionHelper::GetVariable<UFloatProperty, float>(Participant, VariableName));

		case EDlgTextArgumentType::ClassText:
			return FFormatArgumentValue(UDlgReflectionHelper::GetVariable<UTextProperty, FText>(Participant, VariableName));

		case EDlgTextArgumentType::DisplayName:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetParticipantDisplayName(Participant, NodeOwner));

		case EDlgTextArgumentType::Gender:
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
