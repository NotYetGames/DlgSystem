// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "UnrealType.h"

/**
 * The writer will ignore properties by default that are marked DEPRECATED or TRANSIENT, see SkipFlags variable.
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
 *			DlgConfigParser/DlgConfigWriter also supports TArray-s
 */
class DLGSYSTEM_API IDlgWriter
{
public:
	virtual ~IDlgWriter() {}

	/** Has to be called before ExportToFile in order to have something to write */
	virtual void Write(const UStruct* StructDefinition, const void* Object) = 0;
	virtual bool ExportToFile(const FString& FileName) = 0;

	/** Can we skip this property from exporting? */
	static bool CanSkipProperty(const UProperty* Property)
	{
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
	static bool CanWriteOneLinePerItem(const UProperty* Property)
	{
#if WITH_EDITOR
		return Property->HasMetaData(TEXT("DlgLinePerItem"));
#else
		return false;
#endif
	}

	/** Should write the index number for Property? */
	static bool CanWriteIndex(const UProperty* Property)
	{
#if WITH_EDITOR
		return Property->HasMetaData(TEXT("DlgWriteIndex"));
#else
		return false;
#endif
	}

	/** Decides if the path to the object should be serialized, or the object itself */
	virtual bool CanSaveAsReference(const UProperty* Property)
	{
#if WITH_EDITOR
		return Property->HasMetaData(TEXT("DlgSaveOnlyReference"));
#else
		return false;
#endif
	}


protected:
	/** The properties with these flags set will be ignored from writing. */
	static constexpr int64 SkipFlags = CPF_Deprecated | CPF_Transient;
};
