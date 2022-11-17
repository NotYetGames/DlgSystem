// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "UObject/Package.h"

/**
 * The writer will ignore properties by default that are marked DEPRECATED or TRANSIENT, see SkipFlags variable.
 *
 * Limitations:
 * - TSet or TMap with the KeyType as float or structures that have floats, this is very bad you should not do this anyways
 * - limitation for each type you can see inside DlgIoTester.cpp in the Options.
 * - Having an uninitialized UObject property inside a USTRUCT (https://answers.unrealengine.com/questions/566684/editor-crashes-on-startup-if-uninitialized-uproper.html)
 *   THIS CRASHES THE WRITERS so initialize them with nullptr
 *
 * MetaData specifiers:
 *		Unfortunately they only work in editor build
 *		The class can be used in exported game too, but the MetaData specifiers are ignored
 *
 *		- DlgNoExport: property is skipped - WARNING: should be only used on editor only variables,
 *			in non-editor builds you should use the properties from the SkipFlags
 *		- DlgWriteIndex: used on complex (struct/object) arrays, element index is exported as comment
 * 		- DlgLinePerItem: used to force primitive container to write each element into a new line (TODO: MAP SUPPORT)
 *		- DlgSaveOnlyReference: UObject path is serialized instead of UObject (can be used for DataAsset like objects stored in content browser)
 *			ATM IT ONLY WORKS IF IT IS NOT INSIDE A CONTAINER DIRECTLY (can be e.g. inside a struct inside a container tho)
 *
 */
class DLGSYSTEM_API IDlgWriter
{
public:
	virtual ~IDlgWriter() {}

	/** Has to be called before ExportToFile in order to have something to write */
	virtual void Write(const UStruct* StructDefinition, const void* Object) = 0;
	virtual bool ExportToFile(const FString& FileName) = 0;
	virtual const FString& GetAsString() const = 0;

	/** Can we skip this property from exporting? */
	static bool CanSkipProperty(const FProperty* Property)
	{
		if (!Property)
		{
			return true;
		}

#if WITH_EDITOR
		if (Property->HasMetaData(TEXT("DlgNoExport")))
		{
			return true;
		}
#endif
		if (Property->HasAnyPropertyFlags(SkipFlags))
		{
			return true;
		}

		return false;
	}

	/** Should  write one item per line for Property? */
	static bool CanWriteOneLinePerItem(const FProperty* Property)
	{
#if WITH_EDITOR
		return Property && Property->HasMetaData(TEXT("DlgLinePerItem"));
#else
		return false;
#endif
	}

	/** Should write the index number for Property? */
	static bool CanWriteIndex(const FProperty* Property)
	{
#if WITH_EDITOR
		return Property && Property->HasMetaData(TEXT("DlgWriteIndex"));
#else
		return false;
#endif
	}

	/** Decides if the path to the object should be serialized, or the object itself */
	virtual bool CanSaveAsReference(const FProperty* Property, const UObject* Object)
	{
		// UClass
		if (Property && Property->IsA<FClassProperty>())
		{
			return true;
		}

		// if outer is nullptr we assume this is something from content browser (texture, mesh, etc.)
		if (IsValid(Object) && Object->IsValidLowLevelFast())
		{
			if (!IsValid(Object->GetOuter()) || Object->GetOuter()->IsA<UPackage>())
			{
				return true;
			}
		}

#if WITH_EDITOR
		return Property && Property->HasMetaData(TEXT("DlgSaveOnlyReference"));
#else
		return false;
#endif
	}

	// bLogVerbose:
	bool IsLogVerbose() const { return bLogVerbose; }
	void SetLogVerbose(bool bValue) { bLogVerbose = bValue; }

protected:
	/** The properties with these flags set will be ignored from writing. */
	static constexpr int64 SkipFlags = CPF_Deprecated | CPF_Transient;

	// Should this class verbose log?
	bool bLogVerbose = false;
};
