// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgSystemModule.h"

#include "ModuleManager.h"
#include "AssetRegistryModule.h"

#include "DlgSystemPrivatePCH.h"
#include "DlgManager.h"
#include "DlgDialogue.h"

#define LOCTEXT_NAMESPACE "FDlgSystemModule"

//////////////////////////////////////////////////////////////////////////
DEFINE_LOG_CATEGORY(LogDlgSystem)
//////////////////////////////////////////////////////////////////////////

static const FName NAME_MODULE_AssetRegistry("AssetRegistry");

void FDlgSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogDlgSystem, Verbose, TEXT("Started DlgSystemModule"));

	// Listen for deleted assets
	// Maybe even check OnAssetRemoved if not loaded into memory?
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(NAME_MODULE_AssetRegistry).Get();
	AssetRegistry.OnInMemoryAssetDeleted().AddRaw(this, &Self::HandleOnInMemoryAssetDeleted);
	// NOTE: this seems to be the same as the OnInMemoryAssetDeleted as they are called from the same method inside
	// the asset registry.
	AssetRegistry.OnAssetRemoved().AddRaw(this, &Self::HandleAssetRemoved);
	AssetRegistry.OnAssetRenamed().AddRaw(this, &Self::HandleAssetRenamed);
}

void FDlgSystemModule::ShutdownModule()
{
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

	UE_LOG(LogDlgSystem, Verbose, TEXT("Stopped DlgSystemModule"));
}

TArray<UDlgDialogue*> FDlgSystemModule::GetDialoguesWithDuplicateGuid()
{
	TArray<UDlgDialogue*> Dialogues = UDlgManager::GetAllDialoguesFromMemory();
	TArray<UDlgDialogue*> DuplicateDialogues;

	TSet<FGuid> DialogueGuids;
	for (UDlgDialogue* Dialogue : Dialogues)
	{
		const FGuid id = Dialogue->GetDlgGuid();
		if (DialogueGuids.Find(id) == nullptr)
		{
			// does not exist, good
			DialogueGuids.Add(id);
		}
		else
		{
			// how?
			DuplicateDialogues.Add(Dialogue);
		}
	}

	return DuplicateDialogues;
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
	if (DeletedDialogue == nullptr)
	{
		return;
	}

	const FString& TextFilePathName = DeletedDialogue->GetTextFilePathName(false);
	if (TextFilePathName.IsEmpty())
	{
		// Memory corruption? tread carefully here
		UE_LOG(LogDlgSystem, Error, TEXT("Can't delete text file for Dialogue = `%s` because the file path name is empty :O"),
				*DeletedDialogue->GetPathName());
		return;
	}

	IFileManager& FileManager = IFileManager::Get();
	auto DeleteTextFileIfItExists = [&FileManager](const TCHAR* Filename)
	{
		// Text file does not exist, ignore
		if (!FileManager.FileExists(Filename))
		{
			UE_LOG(LogDlgSystem, Verbose, TEXT("Text file does not exist at path = `%s`. Can't delete."), Filename);
			return;
		}

		// Delete the text file
		if (!FileManager.Delete(Filename))
		{
			UE_LOG(LogDlgSystem, Error, TEXT("Can't delete text file at path = `%s`"), Filename);
			return;
		}

		UE_LOG(LogDlgSystem, Verbose, TEXT("Deleted file %s"), Filename);
	};

	// Iterate over all possible text formats
	const int32 TextFormatsNum = static_cast<int32>(EDlgDialogueTextFormat::DlgDialogueTextFormat_Num);
	for (int32 TextFormatIndex = 0; TextFormatIndex < TextFormatsNum; TextFormatIndex++)
	{
		const EDlgDialogueTextFormat CurrentTextFormat = static_cast<EDlgDialogueTextFormat>(TextFormatIndex);
		const FString FullPathName = TextFilePathName + UDlgDialogue::GetTextFileExtension(CurrentTextFormat);
		DeleteTextFileIfItExists(*FullPathName);
	}
}

void FDlgSystemModule::HandleDialogueRenamed(UDlgDialogue* RenamedDialogue, const FString& OldObjectPath)
{
	if (RenamedDialogue == nullptr)
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
		UE_LOG(LogDlgSystem, Error, TEXT("Dialogue was renamed but the paths before and after are equal :O | `%s` == `%s`"), *OldTextFilePathName, *CurrentTextFilePathName);
		return;
	}

	IFileManager& FileManager = IFileManager::Get();
	auto RenameFileIfItExists = [&FileManager](const TCHAR* OldFileName, const TCHAR* NewFileName)
	{
		// Text file we want to rename does not exist anymore
		if (!FileManager.FileExists(OldFileName))
		{
			UE_LOG(LogDlgSystem, Verbose, TEXT("Text file before rename at path = `%s` does not exist. Can't Rename."), OldFileName);
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

		UE_LOG(LogDlgSystem, Verbose, TEXT("Text file moved/renamed from `%s` to `%s`"), OldFileName, NewFileName);
	};

	// Iterate over all possible text formats
	const int32 TextFormatsNum = static_cast<int32>(EDlgDialogueTextFormat::DlgDialogueTextFormat_Num);
	for (int32 TextFormatIndex = 0; TextFormatIndex < TextFormatsNum; TextFormatIndex++)
	{
		const EDlgDialogueTextFormat CurrentTextFormat = static_cast<EDlgDialogueTextFormat>(TextFormatIndex);
		const FString FileExtension = UDlgDialogue::GetTextFileExtension(CurrentTextFormat);
		RenameFileIfItExists(*(OldTextFilePathName + FileExtension), *(CurrentTextFilePathName + FileExtension));
	}
}

#undef LOCTEXT_NAMESPACE

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_MODULE(FDlgSystemModule, DlgSystem)
//////////////////////////////////////////////////////////////////////////
