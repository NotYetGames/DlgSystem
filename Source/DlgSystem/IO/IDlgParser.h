// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "UObject/UObjectIterator.h"
#include "Containers/Array.h"
#include "UObject/Object.h"

class DLGSYSTEM_API IDlgParser
{
public:
	virtual ~IDlgParser() {}

	/** Initializes the parser with the specified FilePath. */
	virtual void InitializeParser(const FString& FilePath) = 0;

	/** Initializes the parser with the specified input string */
	virtual void InitializeParserFromString(const FString& Text) {};

	/** Is the parsed file valid? */
	virtual bool IsValidFile() const = 0;

	/**
	 * Reads all property from the config file.
	 *
	 * @param ReferenceClass: UStruct (or UClass) of the TargetObject
	 * @param TargetObject: pointer to the object instance the function will modify based on the config values
	 * @param DefaultObjectOuter: Outer used for UObject construction if necessary
	 */
	virtual void ReadAllProperty(const UStruct* ReferenceClass, void* TargetObject, UObject* DefaultObjectOuter = nullptr) = 0;

	// bLogVerbose:
	bool IsLogVerbose() const { return bLogVerbose; }
	void SetLogVerbose(bool bValue) { bLogVerbose = bValue; }

protected:
	/**
	 * Searches the proper not abstract class
	 *
	 * @param ParentClass: the class we are looking for has to inherit from this class
	 * @param Name: the name of the class we are looking for (without engine pretags, e.g. Actor for AActor)
	 *
	 * @return the class, or nullptr if it does not exist
	 */
	const UClass* GetChildClassFromName(const UClass* ParentClass, const FString& Name)
	{
		for (UClass* Class : StructCache)
		{
			if (Class->IsChildOf(ParentClass) && Class->GetName() == Name)
			{
				return Class;
			}
		}

		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (It->IsChildOf(ParentClass) && !It->HasAnyClassFlags(CLASS_Abstract) && (*It)->GetName() == Name)
			{
				StructCache.Add(*It);
				return *It;
			}
		}

		return nullptr;
	}

	/**
	 * Default way to create new objects
	 */
	static UObject* CreateNewUObject(const UClass* StructDefinition, UObject* ObjectOuter)
	{
		return NewObject<UObject>(!IsValid(ObjectOuter) ? (UObject*)GetTransientPackage() : ObjectOuter,
								 const_cast<UClass*>(StructDefinition), NAME_None, RF_Transactional);
	}

protected:
	/** each time a class is read it also cached here, so next time when it has to be found based on name it can be found faster */
	TArray<UClass*> StructCache;

	// Should this class verbose log?
	bool bLogVerbose = false;
};
