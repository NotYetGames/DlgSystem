// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgSystemModule.h"

#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "HAL/IConsoleManager.h"
#include "HAL/FileManager.h"
#include "GameFramework/Actor.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#endif
#if WITH_EDITOR
#include "WorkspaceMenuStructureModule.h"
#include "WorkspaceMenuStructure.h"
#endif

#include "DlgSystemPrivatePCH.h"
#include "DlgManager.h"
#include "DlgDialogue.h"
#include "GameplayDebugger/DlgGameplayDebuggerCategory.h"
#include "GameplayDebugger/SDlgDataDisplay.h"

#define LOCTEXT_NAMESPACE "FDlgSystemModule"

//////////////////////////////////////////////////////////////////////////
DEFINE_LOG_CATEGORY(LogDlgSystem)
//////////////////////////////////////////////////////////////////////////

static const FName NAME_MODULE_AssetRegistry("AssetRegistry");

void FDlgSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogDlgSystem, Log, TEXT("Started DlgSystemModule"));
	OnPreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddRaw(this, &Self::HandlePreLoadMap);

	// Listen for deleted assets
	// Maybe even check OnAssetRemoved if not loaded into memory?
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(NAME_MODULE_AssetRegistry).Get();
	AssetRegistry.OnInMemoryAssetDeleted().AddRaw(this, &Self::HandleOnInMemoryAssetDeleted);
	// NOTE: this seems to be the same as the OnInMemoryAssetDeleted as they are called from the same method inside
	// the asset registry.
	AssetRegistry.OnAssetRemoved().AddRaw(this, &Self::HandleAssetRemoved);
	AssetRegistry.OnAssetRenamed().AddRaw(this, &Self::HandleAssetRenamed);

#if WITH_GAMEPLAY_DEBUGGER
	// If the gameplay debugger is available, register the category and notify the editor about the changes
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory(DIALOGUE_SYSTEM_PLUGIN_NAME,
		IGameplayDebugger::FOnGetCategory::CreateStatic(&FDlgGameplayDebuggerCategory::MakeInstance),
		EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif // WITH_GAMEPLAY_DEBUGGER

	// Register tab spawners
	bHasRegisteredTabSpawners = true;

	FTabSpawnerEntry& DialogueDataSpawnEntry =
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(DIALOGUE_DATA_DISPLAY_TAB_ID,
			FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
			{
				TSharedRef<SDockTab> DialogueDataDisplayTab = SNew(SDockTab)
					.TabRole(ETabRole::NomadTab)
					[
						GetDialogueDataDisplayWindow()
					];
				return DialogueDataDisplayTab;
			}))
			.SetDisplayName(LOCTEXT("DialogueDataDisplayTitle", "Dialogue Data Display"))
			.SetTooltipText(LOCTEXT("DialogueDataDisplayTooltipText", "Open the Dialogue Data Display tab."));

#if WITH_EDITOR
	// TODO move DlgStyle to Runtime + Move DialogueToolsCategory to Runtime module
	DialogueDataSpawnEntry.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
#endif
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
		AssetRegistry.OnInMemoryAssetDeleted().RemoveAll(this);
		AssetRegistry.OnAssetRemoved().RemoveAll(this);
		AssetRegistry.OnAssetRenamed().RemoveAll(this);
	}

	if (OnPreLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PreLoadMap.Remove(OnPreLoadMapHandle);
	}

	UE_LOG(LogDlgSystem, Log, TEXT("Stopped DlgSystemModule"));
}

TSharedRef<SWidget> FDlgSystemModule::GetDialogueDataDisplayWindow()
{
	TSharedPtr<SDlgDataDisplay> DialogueData = DialogueDataDisplayWidget.Pin();
	if (!DialogueData.IsValid())
	{
		DialogueData = SNew(SDlgDataDisplay, ReferenceActor);
		DialogueDataDisplayWidget = DialogueData;
	}

	return DialogueData.ToSharedRef();
}

void FDlgSystemModule::RegisterConsoleCommands(AActor* InReferenceActor)
{
	// Unregister first to prevent double register of commands
	UnregisterConsoleCommands();

	ReferenceActor = InReferenceActor;
	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(TEXT("Dlg.DataDisplay"),
		TEXT("Displays the Dialogue Data Window"),
		FConsoleCommandDelegate::CreateRaw(this, &Self::DisplayDialogueDataWindow), ECVF_Default));

	ConsoleCommands.Add(ConsoleManager.RegisterConsoleCommand(TEXT("Dlg.LoadAllDialogues"),
		TEXT("Load All Dialogues into memory"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			UDlgManager::LoadAllDialoguesIntoMemory();
		}), ECVF_Default));

	// In case the DlgDataDisplay is already opened, simply refresh the actor reference
	RefreshDisplayDialogueDataWindow(false);
}

void FDlgSystemModule::UnregisterConsoleCommands()
{
	ReferenceActor = nullptr;
	for (IConsoleCommand* Comand : ConsoleCommands)
	{
		IConsoleManager::Get().UnregisterConsoleObject(Comand);
	}
	ConsoleCommands.Empty();
}

void FDlgSystemModule::DisplayDialogueDataWindow()
{
	if (!bHasRegisteredTabSpawners)
	{
		UE_LOG(LogDlgSystem, Error, TEXT("Did not Initialize the tab spawner for the DisplayDialogueDataWindow"));
		return;
	}

	if (!RefreshDisplayDialogueDataWindow())
	{
		// Create, because it does not exist yet
		FGlobalTabmanager::Get()->InvokeTab(FTabId(DIALOGUE_DATA_DISPLAY_TAB_ID));
	}
}

bool FDlgSystemModule::RefreshDisplayDialogueDataWindow(bool bFocus)
{
	const TSharedPtr<SDockTab> DlgDisplayDataTab =
		FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(DIALOGUE_DATA_DISPLAY_TAB_ID));
	if (DlgDisplayDataTab.IsValid())
	{
		// Set the new ReferenceActor.
		TSharedRef<SDlgDataDisplay> Window = StaticCastSharedRef<SDlgDataDisplay>(DlgDisplayDataTab->GetContent());
		if (ReferenceActor)
		{
			Window->SetReferenceActor(ReferenceActor);
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

void FDlgSystemModule::HandleAssetRemoved(const FAssetData& RemovedAsset)
{
	if (!RemovedAsset.IsAssetLoaded())
	{
		return;
	}
}

void FDlgSystemModule::HandleAssetRenamed(const FAssetData& AssetRenamed, const FString& OldObjectPath)
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

	const FString& TextFilePathName = DeletedDialogue->GetTextFilePathName(false);
	if (TextFilePathName.IsEmpty())
	{
		// Memory corruption? tread carefully here
		UE_LOG(LogDlgSystem,
			   Error,
			   TEXT("Can't delete text file for Dialogue = `%s` because the file path name is empty :O"),
			   *DeletedDialogue->GetPathName());
		return;
	}

	IFileManager& FileManager = IFileManager::Get();
	auto DeleteTextFileIfItExists = [&FileManager](const TCHAR* Filename)
	{
		// Text file does not exist, ignore
		if (!FileManager.FileExists(Filename))
		{
			UE_LOG(LogDlgSystem, Warning, TEXT("Text file does not exist at path = `%s`. Can't delete."), Filename);
			return;
		}

		// Delete the text file
		if (!FileManager.Delete(Filename))
		{
			UE_LOG(LogDlgSystem, Error, TEXT("Can't delete text file at path = `%s`"), Filename);
			return;
		}

		UE_LOG(LogDlgSystem, Log, TEXT("Deleted file %s"), Filename);
	};

	// Iterate over all possible text formats
	const int32 TextFormatsNum = static_cast<int32>(EDlgDialogueTextFormat::DlgDialogueTextFormat_Num);
	for (int32 TextFormatIndex = static_cast<int32>(EDlgDialogueTextFormat::DlgDialogueTextFormatDialogue);
			   TextFormatIndex < TextFormatsNum; TextFormatIndex++)
	{
		const EDlgDialogueTextFormat CurrentTextFormat = static_cast<EDlgDialogueTextFormat>(TextFormatIndex);
		const FString FullPathName = TextFilePathName + UDlgDialogue::GetTextFileExtension(CurrentTextFormat);
		DeleteTextFileIfItExists(*FullPathName);
	}
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
		UE_LOG(LogDlgSystem, Error, TEXT("OldTextFilePathName is empty. This should never happen"));
		return;
	}

	// Current PathName
	const FString CurrentTextFilePathName = RenamedDialogue->GetTextFilePathName(false);
	if (OldTextFilePathName == CurrentTextFilePathName)
	{
		UE_LOG(LogDlgSystem,
			   Error,
			   TEXT("Dialogue was renamed but the paths before and after are equal :O | `%s` == `%s`"),
			   *OldTextFilePathName, *CurrentTextFilePathName);
		return;
	}

	IFileManager& FileManager = IFileManager::Get();
	auto RenameFileIfItExists = [&FileManager](const TCHAR* OldFileName, const TCHAR* NewFileName)
	{
		// Text file we want to rename does not exist anymore
		if (!FileManager.FileExists(OldFileName))
		{
			UE_LOG(LogDlgSystem, Warning, TEXT("Text file before rename at path = `%s` does not exist. Can't Rename."), OldFileName);
			return;
		}

		// Text file at destination already exists, conflict :/
		if (FileManager.FileExists(NewFileName))
		{
			UE_LOG(LogDlgSystem,
				Error,
				TEXT("Text file at destination (after rename) at path = `%s` already exists. Current text file at path = `%s` won't be moved/renamed."),
				NewFileName, OldFileName);
			return;
		}

		// Finally Move/Rename
		if (!FileManager.Move(/*Dest=*/ NewFileName, /*Src=*/ OldFileName, /*bReplace=*/ false))
		{
			UE_LOG(LogDlgSystem, Error, TEXT("Failure to move/rename file from `%s` to `%s`"), OldFileName, NewFileName);
			return;
		}

		UE_LOG(LogDlgSystem, Log, TEXT("Text file moved/renamed from `%s` to `%s`"), OldFileName, NewFileName);
	};

	// Iterate over all possible text formats
	const int32 TextFormatsNum = static_cast<int32>(EDlgDialogueTextFormat::DlgDialogueTextFormat_Num);
	for (int32 TextFormatIndex = static_cast<int32>(EDlgDialogueTextFormat::DlgDialogueTextFormatDialogue);
			   TextFormatIndex < TextFormatsNum; TextFormatIndex++)
	{
		const EDlgDialogueTextFormat CurrentTextFormat = static_cast<EDlgDialogueTextFormat>(TextFormatIndex);
		const FString FileExtension = UDlgDialogue::GetTextFileExtension(CurrentTextFormat);
		RenameFileIfItExists(*(OldTextFilePathName + FileExtension), *(CurrentTextFilePathName + FileExtension));
	}
}

void FDlgSystemModule::HandlePreLoadMap(const FString& MapName)
{
	if (!OnPreLoadMapHandle.IsValid())
	{
		return;
	}
	if (const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>())
	{
		if (!Settings->bClearDialogueHistoryAutomatically)
		{
			return;
		}
	}

	UE_LOG(LogDlgSystem, Verbose, TEXT("PreLoadMap = %s. Clearing Dialogue History"), *MapName);
	UDlgManager::ClearDialogueHistory();
}

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE(FDlgSystemModule, DlgSystem)
//////////////////////////////////////////////////////////////////////////
