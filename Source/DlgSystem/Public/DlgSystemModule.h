// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "ModuleInterface.h"
#include "CoreMinimal.h"

// The name of the Dialogue System plugin as defined in the .uplugin file
const FName DIALOGUE_SYSTEM_PLUGIN_NAME(TEXT("DlgSystem"));

const FName DIALOGUE_DATA_DISPLAY_TAB_ID(TEXT("DlgDataDisplayWindow"));

class UDlgDialogue;
class SWidget;
struct FAssetData;
class SDlgDataDisplay;
class SDockTab;
struct IConsoleCommand;

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

	/** Gets the debug Dialogue Data Display Window. */
	TSharedRef<SWidget> GetDialogueDataDisplayWindow(const TSharedRef<SDockTab>& InParentTab);

	/** Display the debug Dialogue Data Window on the screen */
	void DisplayDialogueDataWindow();

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

private:
	/** True if the tab spawners have been registered for this module */
	bool bHasRegisteredTabSpawners = false;

	/** Holds the widget reflector singleton. */
	TWeakPtr<SDlgDataDisplay> DialogueDataDisplayWidget;

	/** Holds the console command for the DialogueDataDisplayWidget. */
	IConsoleCommand* ConsoleCommandDialogueDataDisplay;
};
