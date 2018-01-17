// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgEditorSettings.h"

#include "DlgManager.h"

#define LOCTEXT_NAMESPACE "DlgEditorSettings"

//////////////////////////////////////////////////////////////////////////
// UDlgEditorSettings

UDlgEditorSettings::UDlgEditorSettings()
{

}

FText UDlgEditorSettings::GetSectionText() const
{
	return LOCTEXT("SectionText", "Dialogue Editor");
}

FText UDlgEditorSettings::GetSectionDescription() const
{
	return LOCTEXT("SectionDescription", "Configure the look and feel of the Dialogue Editor.");
}

bool UDlgEditorSettings::CanEditChange(const UProperty* InProperty) const
{
	bool bIsEditable = Super::CanEditChange(InProperty);
	if (bIsEditable && InProperty)
	{
		const FName PropertyName = InProperty->GetFName();

		// Do now allow to change the bDrawPrimaryEdges,bDrawSecondaryEdges if we aren't even showing them
		if (!bShowPrimarySecondaryEdges &&
			(PropertyName == GET_MEMBER_NAME_CHECKED(Self, bDrawPrimaryEdges) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(Self, bDrawSecondaryEdges)))
		{
			return false;
		}
	}

	return bIsEditable;
}

void UDlgEditorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(Self, DialogueTextFormat))
	{
		// Set the new text format to all dialogues
		for (UDlgDialogue* Dialogue : UDlgManager::GetAllDialoguesFromMemory())
		{
			Dialogue->SetTextFormat(DialogueTextFormat);
		}
	}
}

#undef LOCTEXT_NAMESPACE
