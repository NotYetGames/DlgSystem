// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgSystemSettings.h"

#include "GameFramework/Character.h"

#include "DlgManager.h"

#define LOCTEXT_NAMESPACE "DlgSystemSettings"

//////////////////////////////////////////////////////////////////////////
// UDlgSystemSettings
const FText UDlgSystemSettings::EdgeTextFinish = LOCTEXT("edge_finish", "Finish");
const FText UDlgSystemSettings::EdgeTextNext = LOCTEXT("edge_next", "Next");

UDlgSystemSettings::UDlgSystemSettings()
{
	BlacklistedReflectionClasses = {AActor::StaticClass(), APawn::StaticClass(),  ACharacter::StaticClass()};
	// AdditionalTextFormatFileExtensionsToLookFor = {""};
}

#if WITH_EDITOR
FText UDlgSystemSettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "Dialogue");
}

FText UDlgSystemSettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDescription", "Configure the look and feel of the Dialogue Editor.");
}

bool UDlgSystemSettings::CanEditChange(const UProperty* InProperty) const
{
	const bool bIsEditable = Super::CanEditChange(InProperty);
	if (bIsEditable && InProperty)
	{
		const FName PropertyName = InProperty->GetFName();

		// Do now allow to change the bDrawPrimaryEdges, bDrawSecondaryEdges if we aren't even showing them
		if (!bShowPrimarySecondaryEdges &&
			(PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bDrawPrimaryEdges) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, bDrawSecondaryEdges)))
		{
			return false;
		}
	}

	return bIsEditable;
}

void UDlgSystemSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
}
#endif // WITH_EDITOR

FString UDlgSystemSettings::GetTextFileExtension(EDlgDialogueTextFormat TextFormat)
{
	switch (TextFormat)
	{
		// JSON has the .json added at the end
		case EDlgDialogueTextFormat::JSON:
			return TEXT(".dlg.json");

		case EDlgDialogueTextFormat::DialogueDEPRECATED:
			return TEXT(".dlg");

		// Empty
		case EDlgDialogueTextFormat::None:
		default:
			return FString();
	}
}


#undef LOCTEXT_NAMESPACE
