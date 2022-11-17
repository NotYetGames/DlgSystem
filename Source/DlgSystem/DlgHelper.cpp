// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgHelper.h"

#include "DlgDialogueParticipant.h"
#include "HAL/FileManager.h"
#include "Engine/Blueprint.h"
#include "Logging/DlgLogger.h"
#include "DlgSystemSettings.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Misc/Paths.h"
#include "UObject/UObjectIterator.h"
#include "Framework/Docking/TabManager.h"

bool FDlgHelper::DeleteFile(const FString& PathName, bool bVerbose)
{
	IFileManager& FileManager = IFileManager::Get();

	// Text file does not exist, ignore
	if (!FileManager.FileExists(*PathName))
	{
		if (bVerbose)
		{
			FDlgLogger::Get().Debugf(TEXT("File does not exist at path = `%s`. Can't delete."), *PathName);
		}
		return false;
	}

	// Delete the text file
	if (!FileManager.Delete(*PathName))
	{
		if (bVerbose)
		{
			FDlgLogger::Get().Errorf(TEXT("Can't delete file at path = `%s`"), *PathName);
		}
		return false;
	}

	if (bVerbose)
	{
		FDlgLogger::Get().Infof(TEXT("Deleted file %s"), *PathName);
	}
	return true;
}

bool FDlgHelper::RenameFile(const FString& OldPathName, const FString& NewPathName, bool bOverWrite, bool bVerbose)
{
	IFileManager& FileManager = IFileManager::Get();

	//  File we want to rename does not exist anymore
	if (!FileManager.FileExists(*OldPathName))
	{
		if (bVerbose)
		{
			FDlgLogger::Get().Debugf(TEXT("File before rename at path = `%s` does not exist. Can't Rename."), *OldPathName);
		}
		return false;
	}

	// File at destination already exists, conflict :/
	if (!bOverWrite && FileManager.FileExists(*NewPathName))
	{
		if (bVerbose)
		{
			FDlgLogger::Get().Errorf(
				TEXT("File at destination (after rename) at path = `%s` already exists. Current text file at path = `%s` won't be moved/renamed."),
				*NewPathName, *OldPathName
			);
		}
		return false;
	}

	// Finally Move/Rename
	if (!FileManager.Move(/*Dest=*/ *NewPathName, /*Src=*/ *OldPathName, /*bReplace=*/ bOverWrite))
	{
		if (bVerbose)
		{
			FDlgLogger::Get().Errorf(TEXT("Failure to move/rename file from `%s` to `%s`"), *OldPathName, *NewPathName);
		}
		return false;
	}

	if (bVerbose)
	{
		FDlgLogger::Get().Infof(TEXT("Text file moved/renamed from `%s` to `%s`"), *OldPathName, *NewPathName);
	}
	return true;
}


TSharedPtr<SDockTab> FDlgHelper::InvokeTab(TSharedPtr<FTabManager> TabManager, const FTabId& TabID)
{
	if (!TabManager.IsValid())
	{
		return nullptr;
	}

#if NY_ENGINE_VERSION >= 426
	return TabManager->TryInvokeTab(TabID);
#else
	return TabManager->InvokeTab(TabID);
#endif
}

FString FDlgHelper::CleanObjectName(FString Name)
{
	Name.RemoveFromEnd(TEXT("_C"));

	// Get rid of the extension from `filename.extension` from the end of the path
	static constexpr bool bRemovePath = false;
	Name = FPaths::GetBaseFilename(Name, bRemovePath);

	return Name;
}

bool FDlgHelper::IsClassIgnored(const UClass* Class)
{
	if (!Class)
	{
		return true;
	}

	// Ignore generated types that cannot be spawned
	const FString Name = Class->GetName();
	return Name.StartsWith(TEXT("SKEL_")) || Name.StartsWith(TEXT("REINST_"));
}

bool FDlgHelper::IsABlueprintClass(const UClass* Class)
{
	return Cast<UBlueprintGeneratedClass>(Class) != nullptr;
}

bool FDlgHelper::IsABlueprintObject(const UObject* Object)
{
	return Cast<UBlueprint>(Object) != nullptr;
}

bool FDlgHelper::IsObjectAChildOf(const UObject* Object, const UClass* ParentClass)
{
	if (!Object || !ParentClass)
	{
		return false;
	}

	// Blueprint
	if (const UBlueprint* Blueprint = Cast<UBlueprint>(Object))
	{
		if (const UClass* GeneratedClass = Cast<UClass>(Blueprint->GeneratedClass))
		{
			return GeneratedClass->IsChildOf(ParentClass);
		}
	}

	// A class object, does this ever happen?
	if (const UClass* ClassObject = Cast<UClass>(Object))
	{
		return ClassObject->IsChildOf(ParentClass);
	}

	// All other object types
	return Object->GetClass()->IsChildOf(ParentClass);
}

bool FDlgHelper::IsObjectImplementingInterface(const UObject* Object, const UClass* InterfaceClass)
{
	if (!Object || !InterfaceClass)
	{
		return false;
	}

	// Blueprint
	if (const UBlueprint* Blueprint = Cast<UBlueprint>(Object))
	{
		if (const UClass* GeneratedClass = Cast<UClass>(Blueprint->GeneratedClass))
		{
			return GeneratedClass->ImplementsInterface(InterfaceClass);
		}
	}

	// A class object, does this ever happen?
	if (const UClass* Class = Cast<UClass>(Object))
	{
		return Class->ImplementsInterface(InterfaceClass);
	}

	// All other object types
	return Object->GetClass()->ImplementsInterface(InterfaceClass);
}

bool FDlgHelper::GetAllChildClassesOf(const UClass* ParentClass, TArray<UClass*>& OutNativeClasses, TArray<UClass*>& OutBlueprintClasses)
{
	// Iterate over UClass, this might be heavy on performance
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* ChildClass = *It;
		if (!ChildClass->IsChildOf(ParentClass))
		{
			continue;
		}

		// It is a child of the Parent Class
		// make sure we don't include our parent class in the array
		if (ChildClass == ParentClass)
		{
			continue;
		}

		if (IsABlueprintClass(ChildClass))
		{
			// Blueprint
			OutBlueprintClasses.Add(ChildClass);
		}
		else
		{
			// Native
			OutNativeClasses.Add(ChildClass);
		}
	}

	return OutNativeClasses.Num() > 0 || OutBlueprintClasses.Num() > 0;
}

bool FDlgHelper::GetAllClassesImplementingInterface(const UClass* InterfaceClass, TArray<UClass*>& OutNativeClasses, TArray<UClass*>& OutBlueprintClasses)
{
	// Iterate over UClass, this might be heavy on performance
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		if (!Class->ImplementsInterface(InterfaceClass))
		{
			continue;
		}
		if (IsClassIgnored(Class))
		{
			continue;
		}

		if (IsABlueprintClass(Class))
		{
			// Blueprint
			OutBlueprintClasses.Add(Class);
		}
		else
		{
			// Native
			OutNativeClasses.Add(Class);
		}
	}

	return OutNativeClasses.Num() > 0 || OutBlueprintClasses.Num() > 0;
}

TMap<FName, TArray<FDlgClassAndObject>> FDlgHelper::ConvertDialogueParticipantsClassesIntoMap(const TArray<UClass*>& Classes)
{
	TMap<FName, TArray<FDlgClassAndObject>> ObjectsMap;

	for (UClass* Class : Classes)
	{
		// How did this even get here
		if (!Class->ImplementsInterface(UDlgDialogueParticipant::StaticClass()))
		{
			continue;
		}

		// Should be the same for native and blueprint classes
		UObject* Object = Class->GetDefaultObject();

		// Something is wrong
		if (!Object)
		{
			continue;
		}

		FDlgClassAndObject Struct;
		Struct.Class = Class;
		Struct.Object = Object;

		const FName ParticipantName = IDlgDialogueParticipant::Execute_GetParticipantName(Object);
		if (ObjectsMap.Contains(ParticipantName))
		{
			// Update
			ObjectsMap[ParticipantName].Add(Struct);
		}
		else
		{
			// Create
			TArray<FDlgClassAndObject> Array;
			Array.Add(Struct);
			ObjectsMap.Add(ParticipantName, Array);
		}
	}

	return ObjectsMap;
}
