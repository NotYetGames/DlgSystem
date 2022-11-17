// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "IDlgSystemModule.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UDlgDialogue;
class SWidget;
struct FAssetData;
class SDlgDataDisplay;
class SDockTab;
struct IConsoleCommand;
class AActor;
struct FTabSpawnerEntry;

DECLARE_LOG_CATEGORY_EXTERN(LogDlgSystem, All, All);

// Implementation of the DlgSystem Module
class DLGSYSTEM_API FDlgSystemModule : public IDlgSystemModule
{
	typedef FDlgSystemModule Self;
public:
	// IModuleInterface implementation
	void StartupModule() override;
	void ShutdownModule() override;

	// IDlgSystemModule implementation
	void RegisterConsoleCommands(const TWeakObjectPtr<const UObject>& InWorldContextObjectPtr) override;
	void UnregisterConsoleCommands() override;
	TSharedRef<SWidget> GetDialogueDataDisplayWindow() override;
	FTabSpawnerEntry* GetDialogueDataDisplaySpawnEntry() override;
	void DisplayDialogueDataWindow() override;

private:
	// Refreshes the actor of the DlgDataDisplay if it is already opened. Return true if refresh was successful
	bool RefreshDisplayDialogueDataWindow(bool bFocus = true);

	// Handle the event from the asset registry when an asset was deleted.
	void HandleOnInMemoryAssetDeleted(UObject* DeletedObject);

	// Handle the event for when assets are removed from the asset registry.
	void HandleOnAssetRemoved(const FAssetData& RemovedAsset);

	// Handle the event for when assets are renamed in the registry
	void HandleOnAssetRenamed(const FAssetData& AssetRenamed, const FString& OldObjectPath);

	// Handle the event after the Dialogue was deleted. Deletes the text file(s).
	void HandleDialogueDeleted(UDlgDialogue* DeletedDialogue);

	// Handle the event after the Dialogue was renamed. Rename the text file(s).
	void HandleDialogueRenamed(UDlgDialogue* RenamedDialogue, const FString& OldObjectPath);

	// Handle event when a new map is loaded.
	void HandleOnPreLoadMap(const FString& MapName);

	// Handle event when a new map with world is loaded is loaded.
	void HandleOnPostLoadMapWithWorld(UWorld* LoadedWorld);

private:
	// True if the tab spawners have been registered for this module
	bool bHasRegisteredTabSpawners = false;

	// Holds the widget reflector singleton.
	TWeakPtr<SDlgDataDisplay> DialogueDataDisplayWidget;
	FTabSpawnerEntry* DialogueDataDisplayTabSpawnEntry = nullptr;

	// Holds the console commands for this Module
	TArray<IConsoleCommand*> ConsoleCommands;

	// Reference Object used to get the World
	TWeakObjectPtr<const UObject> WorldContextObjectPtr = nullptr;

	// NOTE: only in NON editor game
	TWeakObjectPtr<UWorld> LastLoadedWorld = nullptr;

	// Handlers
	FDelegateHandle OnPreLoadMapHandle;
	FDelegateHandle OnPostLoadMapWithWorldHandle;
	FDelegateHandle OnInMemoryAssetDeletedHandle;
	FDelegateHandle OnAssetRemovedHandle;
	FDelegateHandle OnAssetRenamedHandle;
};
