// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgTextArgument.h"

#include "UObject/TextProperty.h"

#include "DlgConstants.h"
#include "DlgContext.h"
#include "DlgHelper.h"
#include "DlgDialogueParticipant.h"
#include "NYReflectionHelper.h"
#include "Logging/DlgLogger.h"

FFormatArgumentValue FDlgTextArgument::ConstructFormatArgumentValue(const UDlgContext& Context, FName NodeOwner) const
{
	// If participant name is not valid we use the node owner name
	const FName ValidParticipantName = ParticipantName == NAME_None ? NodeOwner : ParticipantName;
	const UObject* Participant = Context.GetParticipant(ValidParticipantName);
	if (Participant == nullptr)
	{
		FDlgLogger::Get().Errorf(
			TEXT("FAILED to construct text argument because the PARTICIPANT is INVALID (Supplied Participant = %s). \nContext:\n\t%s, DisplayString = %s, ParticipantName = %s, ArgumentType = %s"),
			*ValidParticipantName.ToString(), *Context.GetContextString(), *DisplayString, *ParticipantName.ToString(), *ArgumentTypeToString(Type)
		);
		return FFormatArgumentValue(FText::FromString(TEXT("[CustomTextArgument is INVALID. Missing Participant. Check log]")));
	}

	switch (Type)
	{
		case EDlgTextArgumentType::DialogueInt:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetIntValue(Participant, VariableName));

		case EDlgTextArgumentType::ClassInt:
			return FFormatArgumentValue(FNYReflectionHelper::GetVariable<FNYIntProperty, int32>(Participant, VariableName));

		case EDlgTextArgumentType::DialogueFloat:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetFloatValue(Participant, VariableName));

		case EDlgTextArgumentType::ClassFloat:
			return FFormatArgumentValue(FNYReflectionHelper::GetVariable<FNYFloatProperty, float>(Participant, VariableName));

		case EDlgTextArgumentType::ClassText:
			return FFormatArgumentValue(FNYReflectionHelper::GetVariable<FNYTextProperty, FText>(Participant, VariableName));

		case EDlgTextArgumentType::DisplayName:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetParticipantDisplayName(Participant, NodeOwner));

		case EDlgTextArgumentType::Gender:
			return FFormatArgumentValue(IDlgDialogueParticipant::Execute_GetParticipantGender(Participant));

		case EDlgTextArgumentType::Custom:
			if (CustomTextArgument == nullptr)
			{
				FDlgLogger::Get().Errorf(
					TEXT("Custom Text Argument is INVALID. Returning Error Text. Context:\n\t%s, Participant = %s"),
					*Context.GetContextString(), Participant ? *Participant->GetPathName() : TEXT("INVALID")
				);
				return FFormatArgumentValue(FText::FromString(TEXT("[CustomTextArgument is INVALID. Missing Custom Text Argument. Check log]")));
			}

			return FFormatArgumentValue(CustomTextArgument->GetText(&Context, Participant, DisplayString));

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

FString FDlgTextArgument::ArgumentTypeToString(EDlgTextArgumentType Type)
{
	FString EnumValue;
	FDlgHelper::ConvertEnumToString<EDlgTextArgumentType>(TEXT("EDlgTextArgumentType"), Type, false, EnumValue);
	return EnumValue;
}
