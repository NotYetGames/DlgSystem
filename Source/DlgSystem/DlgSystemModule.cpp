// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgSystemModule.h"

#include "Modules/ModuleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "HAL/IConsoleManager.h"
#include "HAL/FileManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#endif // WITH_GAMEPLAY_DEBUGGER
#if WITH_EDITOR
#include "WorkspaceMenuStructureModule.h"
#include "WorkspaceMenuStructure.h"
#endif // WITH_EDITOR

#include "DlgConstants.h"
#include "DlgManager.h"
#include "DlgDialogue.h"
#include "GameplayDebugger/DlgGameplayDebuggerCategory.h"
#include "GameplayDebugger/SDlgDataDisplay.h"
#include "Logging/DlgLogger.h"
#include "DlgHelper.h"

#define LOCTEXT_NAMESPACE "FDlgSystemModule"

//////////////////////////////////////////////////////////////////////////
DEFINE_LOG_CATEGORY(LogDlgSystem)
//////////////////////////////////////////////////////////////////////////

void FDlgSystemModule::StartupModule()
{
	FDlgLogger::OnStart();

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FDlgLogger::Get().Info(TEXT("DlgSystemModule: StartupModule"));

	OnPreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddRaw(this, &Self::HandleOnPreLoadMap);
	OnPostLoadMapWithWorldHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &Self::HandleOnPostLoadMapWithWorld);

	// Listen for deleted assets
	// Maybe even check OnAssetRemoved if not loaded into memory?
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(NAME_MODULE_AssetRegistry).Get();
	OnInMemoryAssetDeletedHandle = AssetRegistry.OnInMemoryAssetDeleted().AddRaw(this, &Self::HandleOnInMemoryAssetDeleted);
	// NOTE: this seems to be the same as the OnInMemoryAssetDeleted as they are called from the same method inside
	// the asset registry.
	OnAssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddRaw(this, &Self::HandleOnAssetRemoved);
	OnAssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddRaw(this, &Self::HandleOnAssetRenamed);

#if WITH_GAMEPLAY_DEBUGGER
	// If the gameplay debugger is available, register the category and notify the editor about the changes
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory(
		DIALOGUE_SYSTEM_PLUGIN_NAME,
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FDlgGameplayDebuggerCategory::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGameAndSimulate
	);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif // WITH_GAMEPLAY_DEBUGGER

	// Register tab spawners
	bHasRegisteredTabSpawners = true;

	DialogueDataDisplayTabSpawnEntry = &FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		DIALOGUE_DATA_DISPLAY_TAB_ID,
		FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
		{
			TSharedRef<SDockTab> DialogueDataDisplayTab = SNew(SDockTab)
				.TabRole(ETabRole::NomadTab)
				[
					GetDialogueDataDisplayWindow()
				];

#if WITH_EDITOR
			const auto* IconBrush = FNYAppStyle::GetBrush(TEXT("DebugTools.TabIcon"));
			DialogueDataDisplayTab->SetTabIcon(IconBrush);
#endif

			return DialogueDataDisplayTab;
		}))
		.SetDisplayName(LOCTEXT("DialogueDataDisplayTitle", "Dialogue Data Display"))
		.SetTooltipText(LOCTEXT("DialogueDataDisplayTooltipText", "Open the Dialogue Data Display tab.")
	);
}

void FDlgSystemModule::ShutdownModule()
{
	// Unregister the console commands in case the user forgot to clear them
	UnregisterConsoleCommands();

	// Unregister the tab spawners
	bHasRegisteredTabSpawners = false;
	FGlobalTabmanager::Get()->UnregisterTabSpawner(DIALOGUE_DATA_DISPLAY_TAB_ID);

#if WITH_GAMEPLAY_DEBUGGER
	// If the gameplay debugger is available, unregister the category
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory(DIALOGUE_SYSTEM_PLUGIN_NAME);
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif // WITH_GAMEPLAY_DEBUGGER

	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	const FModuleManager& ModuleManger = FModuleManager::Get();

	// Unregister the the asset registry delete listeners
	if (ModuleManger.IsModuleLoaded(NAME_MODULE_AssetRegistry))
	{
		IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(NAME_MODULE_AssetRegistry).Get();
		if (OnInMemoryAssetDeletedHandle.IsValid())
		{
			AssetRegistry.OnInMemoryAssetDeleted().Remove(OnInMemoryAssetDeletedHandle);
		}
		if (OnAssetRemovedHandle.IsValid())
		{
			AssetRegistry.OnAssetRemoved().Remove(OnAssetRemovedHandle);
		}
		if (OnAssetRenamedHandle.IsValid())
		{
			AssetRegistry.OnAssetRenamed().Remove(OnAssetRenamedHandle);
		}
	}

	if (OnPreLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PreLoadMap.Remove(OnPreLoadMapHandle);
	}
	if (OnPostLoadMapWithWorldHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(OnPostLoadMapWithWorldHandle);
	}

	FDlgLogger::Get().Info(TEXT("DlgSystemModule: ShutdownModule"));
	FDlgLogger::OnShutdown();
}

TSharedRef<SWidget> FDlgSystemModule::GetDialogueDataDisplayWindow()
{
	TSharedPtr<SDlgDataDisplay> DialogueData = DialogueDataDisplayWidget.Pin();
	if (!DialogueData.IsValid())
	{
		DialogueData = SNew(SDlgDataDisplay, WorldContextObjectPtr);
		DialogueDataDisplayWidget = DialogueData;
	}

	return DialogueData.ToSharedRef();
}

FTabSpawnerEntry* FDlgSystemModule::GetDialogueDataDisplaySpawnEntry()
{
	return DialogueDataDisplayTabSpawnEntry;
}

void FDlgSystemModule::RegisterConsoleCommands(const TWeakObjectPtr<const UObject>& InWorldContextObjectPtr)
{
	// Unregister first to prevent double register of commands
	UnregisterConsoleCommands();

	if (InWorldContextObjectPtr.IsValid())
	{
		WorldContextObjectPtr = InWorldContextObjectPtr;
	}

	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	ConsoleCommands.Add(
		ConsoleManager.RegisterConsoleCommand(
			TEXT("Dlg.DataDisplay"),
			TEXT("Displays the Dialogue Data Window"),
			FConsoleCommandDelegate::CreateRaw(this, &Self::DisplayDialogueDataWindow),
			ECVF_Default
		)
	);

	ConsoleCommands.Add(
		ConsoleManager.RegisterConsoleCommand(
			TEXT("Dlg.LoadAllDialogues"),
			TEXT("Load All Dialogues into memory"),
			FConsoleCommandDelegate::CreateLambda([]()
			{
				UDlgManager::LoadAllDialoguesIntoMemory();
			}),
			ECVF_Default
		)
	);

	// In case the DlgDataDisplay is already opened, simply refresh the actor reference
	RefreshDisplayDialogueDataWindow(false);
}

void FDlgSystemModule::UnregisterConsoleCommands()
{
	WorldContextObjectPtr.Reset();
	for (IConsoleCommand* Command : ConsoleCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(Command);
	}
	ConsoleCommands.Empty();
}

void FDlgSystemModule::DisplayDialogueDataWindow()
{
	if (!bHasRegisteredTabSpawners)
	{
		FDlgLogger::Get().Error(TEXT("Did not Initialize the tab spawner for the DisplayDialogueDataWindow"));
		return;
	}

	if (!RefreshDisplayDialogueDataWindow())
	{
		// Create, because it does not exist yet
		FDlgHelper::InvokeTab(FGlobalTabmanager::Get(), FTabId(DIALOGUE_DATA_DISPLAY_TAB_ID));
	}
}

bool FDlgSystemModule::RefreshDisplayDialogueDataWindow(bool bFocus)
{
	const TSharedPtr<SDockTab> DlgDisplayDataTab =
		FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(DIALOGUE_DATA_DISPLAY_TAB_ID));
	if (DlgDisplayDataTab.IsValid())
	{
		// Set the new WorldContextObjectPtr.
		TSharedRef<SDlgDataDisplay> Window = StaticCastSharedRef<SDlgDataDisplay>(DlgDisplayDataTab->GetContent());
		if (WorldContextObjectPtr.IsValid())
		{
			Window->SetWorldContextObject(WorldContextObjectPtr);
			Window->RefreshTree(false);
		}

		// Focus
		if (bFocus)
		{
			FGlobalTabmanager::Get()->DrawAttention(DlgDisplayDataTab.ToSharedRef());
		}

		return true;
	}

	return false;
}

void FDlgSystemModule::HandleOnInMemoryAssetDeleted(UObject* DeletedObject)
{
	// Should be safe to access it here
	// See UAssetRegistryImpl::AssetDeleted
	if (UDlgDialogue* Dialogue = Cast<UDlgDialogue>(DeletedObject))
	{
		HandleDialogueDeleted(Dialogue);
	}
}

void FDlgSystemModule::HandleOnAssetRemoved(const FAssetData& RemovedAsset)
{
	if (!RemovedAsset.IsAssetLoaded())
	{
		return;
	}
}

void FDlgSystemModule::HandleOnAssetRenamed(const FAssetData& AssetRenamed, const FString& OldObjectPath)
{
	UObject* ObjectRenamed = AssetRenamed.GetAsset();
	if (UDlgDialogue* Dialogue = Cast<UDlgDialogue>(ObjectRenamed))
	{
		HandleDialogueRenamed(Dialogue, OldObjectPath);
	}
}

void FDlgSystemModule::HandleDialogueDeleted(UDlgDialogue* DeletedDialogue)
{
	if (!IsValid(DeletedDialogue))
	{
		return;
	}

	DeletedDialogue->DeleteAllTextFiles();
}

void FDlgSystemModule::HandleDialogueRenamed(UDlgDialogue* RenamedDialogue, const FString& OldObjectPath)
{
	if (!IsValid(RenamedDialogue))
	{
		return;
	}

	// Rename text file file to new location
	const FString OldTextFilePathName = UDlgDialogue::GetTextFilePathNameFromAssetPathName(OldObjectPath);
	if (OldTextFilePathName.IsEmpty())
	{
		FDlgLogger::Get().Error(TEXT("OldTextFilePathName is empty. This should never happen"));
		return;
	}

	// Current PathName
	const FString CurrentTextFilePathName = RenamedDialogue->GetTextFilePathName(false);
	if (OldTextFilePathName == CurrentTextFilePathName)
	{
		FDlgLogger::Get().Errorf(
			TEXT("Dialogue was renamed but the paths before and after are equal :O | `%s` == `%s`"),
			*OldTextFilePathName, *CurrentTextFilePathName
		);
		return;
	}

	// Iterate over all possible text formats
	for (const FString& FileExtension : GetDefault<UDlgSystemSettings>()->GetAllTextFileExtensions())
	{
		const FString OldFileName = OldTextFilePathName + FileExtension;
		const FString NewFileName = CurrentTextFilePathName + FileExtension;
		FDlgHelper::RenameFile(OldFileName, NewFileName, true);
	}
}

void FDlgSystemModule::HandleOnPreLoadMap(const FString& MapName)
{
	// NOTE: only in NON editor game
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	if (!Settings)
	{
		return;
	}

	if (Settings->bClearDialogueHistoryAutomatically)
	{
		FDlgLogger::Get().Debugf(TEXT("PreLoadMap = %s. Clearing Dialogue History"), *MapName);
		UDlgManager::ClearDialogueHistory();
	}
}

void FDlgSystemModule::HandleOnPostLoadMapWithWorld(UWorld* LoadedWorld)
{
	// NOTE: only in NON editor game
	if (!LoadedWorld)
	{
		return;
	}

	LastLoadedWorld = LoadedWorld;
	const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>();
	if (!Settings)
	{
		return;
	}

	if (Settings->bRegisterDialogueConsoleCommandsAutomatically)
	{
		FDlgLogger::Get().Debugf(TEXT("PostLoadMapWithWorld = %s. Registering Console commands"), *LoadedWorld->GetMapName());
		RegisterConsoleCommands(LoadedWorld);
	}
}

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE(FDlgSystemModule, DlgSystem)
//////////////////////////////////////////////////////////////////////////
