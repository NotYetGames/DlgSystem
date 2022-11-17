// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgDialogueAssetTypeActions.h"

#include "DlgSystemEditor/Editor/DlgEditor.h"
#include "DlgSystemEditor/DlgSystemEditorModule.h"

void FDlgDialogueAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	// Implement the menu actions here
}

void FDlgDialogueAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects,
												TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		// Only handle dialogues
		if (UDlgDialogue* Dialogue = Cast<UDlgDialogue>(Object))
		{
			UE_LOG(LogDlgSystemEditor, Log, TEXT("Clicked a Dialogue = `%s`"), *Dialogue->GetPathName());

			TSharedRef<FDlgEditor> NewDialogueEditor(new FDlgEditor());
 			NewDialogueEditor->InitDialogueEditor(Mode, EditWithinLevelEditor, Dialogue);

			// Default Editor
//			FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, Dialogue);
		}
	}
}
