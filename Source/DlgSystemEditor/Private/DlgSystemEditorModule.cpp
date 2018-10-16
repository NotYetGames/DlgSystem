// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgSystemEditorModule.h"

#include "Extensions/ContentBrowserExtensions.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/BlueprintCore.h"
#include "Templates/SharedPointer.h"
#include "AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "WorkspaceMenuStructureModule.h"
#include "WorkspaceMenuStructure.h"
#include "Widgets/Docking/SDockTab.h"
#include "K2Node.h"
#include "FileHelpers.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "LevelEditor.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Editor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "DialogueGraphFactories.h"
#include "DlgSystemEditorPrivatePCH.h"
#include "TypeActions/AssetTypeActions_Dialogue.h"
#include "DialogueEditor/DialogueEditorCommands.h"
#include "DialogueEditor/Nodes/DialogueGraphNode.h"
#include "DialogueBrowser/SDialogueBrowser.h"
#include "DialogueSearch/FindInDialogueSearchManager.h"
#include "DialogueEditor/DetailsPanel/Dialogue_Details.h"
#include "DialogueEditor/DetailsPanel/DialogueGraphNode_Details.h"
#include "DialogueEditor/DetailsPanel/DialogueNode_Details.h"
#include "DialogueEditor/DetailsPanel/DialogueEdge_Details.h"
#include "DialogueEditor/DetailsPanel/DialogueCondition_Details.h"
#include "DialogueEditor/DetailsPanel/DialogueEvent_Details.h"
#include "DialogueEditor/DetailsPanel/DialogueTextArgument_Details.h"
#include "DialogueEditor/DetailsPanel/DialogueSpeechSequenceEntry_Details.h"
#include "DlgDialogueParticipant.h"
#include "DlgManager.h"
#include "DlgSystemModule.h"

#include "IO/DlgConfigWriter.h"
#include "IO/DlgConfigParser.h"

#define LOCTEXT_NAMESPACE "DlgSystemEditor"

//////////////////////////////////////////////////////////////////////////
DEFINE_LOG_CATEGORY(LogDlgSystemEditor)
//////////////////////////////////////////////////////////////////////////

// Just some constants
static const FName DIALOGUE_BROWSER_TAB_ID("DialogueBrowser");


FDlgSystemEditorModule::FDlgSystemEditorModule() : DlgSystemAssetCategoryBit(EAssetTypeCategories::UI)
{
}

void FDlgSystemEditorModule::StartupModule()
{
	UE_LOG(LogDlgSystemEditor, Verbose, TEXT("Started DlgSystemEditorModule"));
	OnPostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddRaw(this, &Self::HandleOnPostEngineInit);
	OnBeginPIEHandle = FEditorDelegates::BeginPIE.AddRaw(this, &Self::HandleOnBeginPIE);

	// Register slate style overrides
	FDialogueStyle::Initialize();

	// Register commands
	FDialogueEditorCommands::Register();

	// Register asset types, add the right click submenu
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(NAME_MODULE_AssetTools).Get();

	// make the DlgSystem be displayed in the filters menu and in the create new menu
	DlgSystemAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(DIALOGUE_SYSTEM_MENU_CATEGORY_KEY, DIALOGUE_SYSTEM_MENU_CATEGORY_KEY_TEXT);
	{
		auto Action = MakeShareable(new FAssetTypeActions_Dialogue(DlgSystemAssetCategoryBit));
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

	// Register the details panel customizations
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(NAME_MODULE_PropertyEditor);

		// For classes:
		// NOTE Order of these two arrays must match
		TArray<FOnGetDetailCustomizationInstance> CustomClassLayouts = {
			  FOnGetDetailCustomizationInstance::CreateStatic(&FDialogue_Details::MakeInstance),
			  FOnGetDetailCustomizationInstance::CreateStatic(&FDialogueGraphNode_Details::MakeInstance),
			  FOnGetDetailCustomizationInstance::CreateStatic(&FDialogueNode_Details::MakeInstance)
		};
		RegisteredCustomClassLayouts = {
			UDlgDialogue::StaticClass()->GetFName(),
			UDialogueGraphNode::StaticClass()->GetFName(),
			UDlgNode::StaticClass()->GetFName()
		};
		for (int32 i = 0; i < RegisteredCustomClassLayouts.Num(); i++)
		{
			PropertyModule.RegisterCustomClassLayout(RegisteredCustomClassLayouts[i], CustomClassLayouts[i]);
		}

		// For structs:
		// NOTE Order of these two arrays must match
		TArray<FOnGetPropertyTypeCustomizationInstance> CustomPropertyTypeLayouts = {
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDialogueEdge_Details::MakeInstance),
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDialogueCondition_Details::MakeInstance),
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDialogueEvent_Details::MakeInstance),
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDialogueSpeechSequenceEntry_Details::MakeInstance),
			FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FDialogueTextArgument_Details::MakeInstance)
		};
		RegisteredCustomPropertyTypeLayout = {
			FDlgEdge::StaticStruct()->GetFName(),
			FDlgCondition::StaticStruct()->GetFName(),
			FDlgEvent::StaticStruct()->GetFName(),
			FDlgSpeechSequenceEntry::StaticStruct()->GetFName(),
			FDlgTextArgument::StaticStruct()->GetFName()
		};
		for (int32 i = 0; i < RegisteredCustomPropertyTypeLayout.Num(); i++)
		{
			PropertyModule.RegisterCustomPropertyTypeLayout(RegisteredCustomPropertyTypeLayout[i], CustomPropertyTypeLayouts[i]);
		}

		PropertyModule.NotifyCustomizationModuleChanged();
	}

	// Register the thumbnail renderers
//	UThumbnailManager::Get().RegisterCustomRenderer(UDlgDialogue::StaticClass(), UDlgDialogueThumbnailRenderer::StaticClass());

	// Create factories
	DialogueGraphNodeFactory = MakeShareable(new FDialogueGraphNodeFactory());
	FEdGraphUtilities::RegisterVisualNodeFactory(DialogueGraphNodeFactory);

	DialogueGraphPinFactory = MakeShareable(new FDialogueGraphPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(DialogueGraphPinFactory);

	// Bind Editor commands
	GlobalEditorCommands = MakeShareable(new FUICommandList);
	GlobalEditorCommands->MapAction(FDialogueEditorCommands::Get().SaveAllDialogues,
		FExecuteAction::CreateStatic(&Self::HandleOnSaveAllDialogues));

	// Content Browser extension
	FDlgContentBrowserExtensions::InstallHooks();

	// Extend menu/toolbar
	ExtendMenu();
}

void FDlgSystemEditorModule::ShutdownModule()
{
	const FModuleManager& ModuleManger = FModuleManager::Get();
	if (DialogueGraphPinFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(DialogueGraphPinFactory);
	}

	if (DialogueGraphNodeFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(DialogueGraphNodeFactory);
	}

	if (UObjectInitialized())
	{
		// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
		// we call this function before unloading the module.
		FDlgContentBrowserExtensions::RemoveHooks();
	}

	// Unregister the custom details panel stuff
	if (ModuleManger.IsModuleLoaded(NAME_MODULE_PropertyEditor))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(NAME_MODULE_PropertyEditor);
		for (int32 i = 0; i < RegisteredCustomClassLayouts.Num(); i++)
		{
			PropertyModule.UnregisterCustomClassLayout(RegisteredCustomClassLayouts[i]);
		}

		for (int32 i = 0; i < RegisteredCustomPropertyTypeLayout.Num(); i++)
		{
			PropertyModule.UnregisterCustomPropertyTypeLayout(RegisteredCustomPropertyTypeLayout[i]);
		}
	}
	RegisteredCustomClassLayouts.Empty();
	RegisteredCustomPropertyTypeLayout.Empty();

	// Unregister all the asset types that we registered
	if (ModuleManger.IsModuleLoaded(NAME_MODULE_AssetTools))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>(NAME_MODULE_AssetTools).Get();
		for (auto TypeAction : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(TypeAction.ToSharedRef());
		}
	}
	RegisteredAssetTypeActions.Empty();

	// unregister commands
	FDialogueEditorCommands::Unregister();

	// Unregister slate style overrides
	FDialogueStyle::Shutdown();

	// Unregister the Dialogue Browser
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DIALOGUE_BROWSER_TAB_ID);

	// Unregister the Dialogue Search
	FFindInDialogueSearchManager::Get()->DisableGlobalFindResults();

	if (OnBeginPIEHandle.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(OnBeginPIEHandle);
	}

	UE_LOG(LogDlgSystemEditor, Verbose, TEXT("Stopped DlgSystemEditorModule"));
}

bool FDlgSystemEditorModule::SaveAllDialogues()
{
	TArray<UDlgDialogue*> Dialogues = UDlgManager::GetAllDialoguesFromMemory();
	TArray<UPackage*> PackagesToSave;
	for (UDlgDialogue* Dialogue : Dialogues)
	{
		Dialogue->MarkPackageDirty();
		PackagesToSave.Add(Dialogue->GetOutermost());
	}

	static constexpr bool bCheckDirty = false;
	static constexpr bool bPromptToSave = false;
	return FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, bCheckDirty, bPromptToSave) == FEditorFileUtils::EPromptReturnCode::PR_Success;
}

void FDlgSystemEditorModule::HandleOnSaveAllDialogues()
{
	const EAppReturnType::Type Response = FPlatformMisc::MessageBoxExt(EAppMsgType::YesNo,
		TEXT("Save all Dialogue assets/files? This will save both the .uasset and the text files depending on the TextFormat from the Dialogue Settings."),
		TEXT("Save Dialogues?"));
	if (Response == EAppReturnType::No)
	{
		return;
	}

	if (!Self::SaveAllDialogues())
	{
		UE_LOG(LogDlgSystemEditor, Error, TEXT("Failed To save all Dialogues. An error occurred."));
	}
}

void FDlgSystemEditorModule::HandleOnPostEngineInit()
{
	if (!OnPostEngineInitHandle.IsValid())
	{
		return;
	}

	UE_LOG(LogDlgSystemEditor, Verbose, TEXT("DlgSystemEditorModule::HandleOnPostEngineInit"));
	FCoreDelegates::OnPostEngineInit.Remove(OnPostEngineInitHandle);
	OnPostEngineInitHandle.Reset();

	//const int32 NumDialoguesBefore = UDlgManager::GetAllDialoguesFromMemory().Num();
	const int32 NumLoadedDialogues = UDlgManager::LoadAllDialoguesIntoMemory();
	//const int32 NumDialoguesAfter = UDlgManager::GetAllDialoguesFromMemory().Num();
	//check(NumDialoguesBefore == NumDialoguesAfter);
	UE_LOG(LogDlgSystemEditor, Verbose, TEXT("UDlgManager::LoadAllDialoguesIntoMemory loaded %d Dialogues into Memory"), NumLoadedDialogues);

	// Try to fix duplicate GUID
	// Can happen for one of the following reasons:
	// - duplicated files outside of UE
	// - somehow loaded from text files?
	// - the universe hates us? +_+
	for (UDlgDialogue* Dialogue : UDlgManager::GetDialoguesWithDuplicateGuid())
	{
		UE_LOG(LogDlgSystemEditor, Warning, TEXT("Dialogue = `%s`, GUID = `%s` has a Duplicate GUID. Regenerating."),
			*Dialogue->GetPathName(), *Dialogue->GetDlgGuid().ToString())
			Dialogue->RegenerateGuid();
		Dialogue->MarkPackageDirty();
	}

	// Give it another try, Give up :((
	// May the math Gods have mercy on us!
	for (const UDlgDialogue* Dialogue : UDlgManager::GetDialoguesWithDuplicateGuid())
	{
		// GUID already exists (╯°□°）╯︵ ┻━┻
		// Does this break the universe?
		UE_LOG(LogDlgSystemEditor, Error, TEXT("Dialogue = `%s`, GUID = `%s`"),
			*Dialogue->GetPathName(), *Dialogue->GetDlgGuid().ToString())
			UE_LOG(LogDlgSystemEditor,
				Fatal,
				TEXT("(╯°□°）╯︵ ┻━┻ Congrats, you just broke the universe, are you even human? Now please go and proove an NP complete problem."
					"The chance of generating two equal random FGuid (picking 4, uint32 numbers) is p = 9.3132257 * 10^(-10) % (or something like this)"))
	}
}

void FDlgSystemEditorModule::HandleOnBeginPIE(bool bIsSimulating)
{
	if (!OnBeginPIEHandle.IsValid())
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

	UE_LOG(LogDlgSystemEditor, Verbose, TEXT("BeginPIE: Clearing Dialogue History"));
	UDlgManager::ClearDialogueHistory();
}

void FDlgSystemEditorModule::ExtendMenu()
{
	// Running in game mode (standalone game) exit as we can't get the LevelEditorModule.
	if (IsRunningGame() || IsRunningCommandlet())
	{
		return;
	}

	// File -> Save all Dialogues
	{
		TSharedRef<FExtender> FileMenuExtender(new FExtender);

		// Fill after the File->FileLoadAndSave
		FileMenuExtender->AddMenuExtension(
			"FileLoadAndSave",
			EExtensionHook::After,
			GlobalEditorCommands.ToSharedRef(),
			FMenuExtensionDelegate::CreateLambda([this](FMenuBuilder& MenuBuilder)
			{
				// Save Dialogues
				MenuBuilder.BeginSection("DialogueFileLoadAndSave", LOCTEXT("DialogueKeyFileAndSearch", "Dialogue"));
				{
					MenuBuilder.AddMenuEntry(FDialogueEditorCommands::Get().SaveAllDialogues);
				}
				MenuBuilder.EndSection();
			}));

		// Add to the level editor
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(NAME_MODULE_LevelEditor);
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(FileMenuExtender);
	}

	// Window -> Dialogue search and browse
	{
		ToolsDialogueCategory = WorkspaceMenu::GetMenuStructure().GetStructureRoot()
			->AddGroup(LOCTEXT("WorkspaceMenu_DialogueCategory", "Dialogue" ),
					FSlateIcon(FDialogueStyle::GetStyleSetName(), FDialogueStyle::PROPERTY_DialogueClassIcon), false);

		// Register the Dialogue Overview Browser
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(DIALOGUE_BROWSER_TAB_ID,
			FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& Args) -> TSharedRef<SDockTab>
			{
				const TSharedRef<SDockTab> DockTab = SNew(SDockTab)
					.TabRole(ETabRole::NomadTab)
					[
						SNew(SDialogueBrowser)
					];
				return DockTab;
			}))
			.SetDisplayName(LOCTEXT("DialogueBrowserTabTitle", "Dialogue Browser"))
			.SetTooltipText(LOCTEXT("DialogueBrowserTooltipText", "Open the Dialogue Overview Browser tab."))
			.SetIcon(FSlateIcon(FDialogueStyle::GetStyleSetName(), FDialogueStyle::PROPERTY_BrowseDialogueIcon))
			.SetGroup(ToolsDialogueCategory.ToSharedRef());

		// Register the Dialogue Search
		FFindInDialogueSearchManager::Get()->Initialize(ToolsDialogueCategory);
	}
}

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE(FDlgSystemEditorModule, DlgSystemEditor)
//////////////////////////////////////////////////////////////////////////
