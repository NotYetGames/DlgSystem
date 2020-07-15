// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgHelper.h"
#include "HAL/FileManager.h"
#include "Engine/Blueprint.h"
#include "Logging/DlgLogger.h"
#include "DlgSystemSettings.h"

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

bool FDlgHelper::IsObjectAChildOf(const UObject* Object, const UClass* Class)
{
	check(Class);
	if (!Object)
	{
		return false;
	}

	if (const UBlueprint* Blueprint = Cast<UBlueprint>(Object))
	{
		if (const UClass* GeneratedClass = Cast<UClass>(Blueprint->GeneratedClass))
		{
			return GeneratedClass->IsChildOf(Class);
		}
	}

	// A class object, does this ever happen?
	if (const UClass* ClassObject = Cast<UClass>(Object))
	{
		return ClassObject->IsChildOf(Class);
	}

	// All other object types
	return Object->GetClass()->IsChildOf(Class);
}
