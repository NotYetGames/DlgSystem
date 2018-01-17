// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "ModuleInterface.h"
#include "CoreMinimal.h"

// The name of the Dialogue System plugin as defined in the .uplugin file
const FString DIALOGUE_SYSTEM_PLUGIN_NAME(TEXT("DlgSystem"));

class UDlgDialogue;
struct FAssetData;

class DLGSYSTEM_API FDlgSystemModule : public IModuleInterface
{
private:
	typedef FDlgSystemModule Self;

public:
	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;

	/** Gets all the dialogues that have a duplicate GUID, should not happen, like ever. */
	static TArray<UDlgDialogue*> GetDialoguesWithDuplicateGuid();

private:
	/** Handle the event from the asset registry when an asset was deleted. */
	void HandleOnInMemoryAssetDeleted(UObject* DeletedObject);

	/** Handle the event for when assets are removed from the asset registry. */
	void HandleAssetRemoved(const FAssetData& RemovedAsset);

	/** Handle the event for when assets are renamed in the registry */
	void HandleAssetRenamed(const FAssetData& AssetRenamed, const FString& OldObjectPath);

	/** Handle the event after the Dialogue was deleted. Deletes the text file(s). */
	void HandleDialogueDeleted(UDlgDialogue* DeletedDialogue);

	/** Handle the event after the Dialogue was renamed. Rename the text file(s). */
	void HandleDialogueRenamed(UDlgDialogue* RenamedDialogue, const FString& OldObjectPath);
};
