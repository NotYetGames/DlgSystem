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
#include "Logging/DlgLogger.h"

#define LOCTEXT_NAMESPACE "FDlgSystemModule"

//////////////////////////////////////////////////////////////////////////
DEFINE_LOG_CATEGORY(LogDlgSystem)
//////////////////////////////////////////////////////////////////////////

static const FName NAME_MODULE_AssetRegistry("AssetRegistry");

void FDlgSystemModule::StartupModule()
{
	FDlgLogger::OnStart();

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FDlgLogger::Get().Info(TEXT("DlgSystemModule: StartupModule"));

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

	FDlgLogger::Get().Info(TEXT("DlgSystemModule: ShutdownModule"));
	FDlgLogger::OnShutdown();
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
		FDlgLogger::Get().Error(TEXT("Did not Initialize the tab spawner for the DisplayDialogueDataWindow"));
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

void FDlgSystemModule::HandlePreLoadMap(const FString& MapName)
{
	if (!OnPreLoadMapHandle.IsValid())
	{
		return;
	}
	if (const UDlgSystemSettings* Settings = GetDefault<UDlgSystemSettings>())
	{
		if (Settings->bClearDialogueHistoryAutomatically)
		{
			FDlgLogger::Get().Debugf(TEXT("PreLoadMap = %s. Clearing Dialogue History"), *MapName);
			UDlgManager::ClearDialogueHistory();
		}
	}

}

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE(FDlgSystemModule, DlgSystem)
//////////////////////////////////////////////////////////////////////////
