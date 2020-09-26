// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "AssetTypeActions_DlgDialogue.h"

#include "DialogueEditor/DialogueEditor.h"
#include "DlgSystemEditorModule.h"

void FAssetTypeActions_DlgDialogue::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	// Implement the menu actions here
}

void FAssetTypeActions_DlgDialogue::OpenAssetEditor(const TArray<UObject*>& InObjects,
												TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		// Only handle dialogues
		if (UDlgDialogue* Dialogue = Cast<UDlgDialogue>(Object))
		{
			UE_LOG(LogDlgSystemEditor, Log, TEXT("Clicked a Dialogue = `%s`"), *Dialogue->GetPathName());

			TSharedRef<FDialogueEditor> NewDialogueEditor(new FDialogueEditor());
 			NewDialogueEditor->InitDialogueEditor(Mode, EditWithinLevelEditor, Dialogue);

			// Default Editor
//			FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, Dialogue);
		}
	}
}
