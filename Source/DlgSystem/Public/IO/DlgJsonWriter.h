// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "Logging/LogMacros.h"
#include "UObject/UnrealType.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonValue.h"
#include "Dom/JsonObject.h"

#include "IDlgWriter.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgJsonWriter, All, All);


struct DLGSYSTEM_API DlgJsonWriterOptions
{
	int32 InitialIndent = 0;
	bool bPrettyPrint = true;
};

/**
 * @brief The DlgJsonWriter class mostly adapted for Dialogues, copied from FJsonObjectConverter
 * See IDlgWriter for properties and METADATA specifiers.
 */
class DLGSYSTEM_API FDlgJsonWriter : public IDlgWriter
{
	/**
	 * Call Order and possible calls:
	 *  - DlgJsonWriter
	 *		- UStructToJsonString
	 *			- UStructToJsonObject
	 *				- UStructToJsonAttributes
	 *					- PropertyToJsonValue
	 *						- ConvertScalarPropertyToJsonValue
	 *							- PropertyToJsonValue
	 *							- UStructToJsonObject
	 */
public:

	FDlgJsonWriter() {};

	// IDlgWriter Interface
	void Write(const UStruct* StructDefinition, const void* ContainerPtr) override;

	/**
	 * Save the config string to a text file
	 * @param FullName: Full path + file name + extension
	 * @return	False on failure to write
	 */
	bool ExportToFile(const FString& FileName) override
	{
		return FFileHelper::SaveStringToFile(JsonString, *FileName, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	const FString& GetAsString() const override
	{
		return JsonString;
	}

private: // UStruct -> JSON
	/**
	 * Convert property to JSON, assuming either the property is not an array or the value is an individual array element
	 * Used by PropertyToJsonValue
	 */
	TSharedPtr<FJsonValue> ConvertScalarPropertyToJsonValue(const FNYProperty* Property, const void* const ContainerPtr, const void* const ValuePtr);

	/**
	 * Converts from a Property to a Json Value using exportText
	 *
	 * @param Property			The property to export
	 * @param ValuePtr			Pointer to the value of the property
	 *
	 * @return					The constructed JsonValue from the property
	 */
	TSharedPtr<FJsonValue> PropertyToJsonValue(const FNYProperty* Property, const void* const ContainerPtr, const void* const ValuePtr);

	/**
	 * Converts from a UStruct to a set of json attributes (possibly from within a JsonObject)
	 *
	 * @param StructDefinition UStruct definition that is looked over for properties
	 * @param ContainerPtr	   The object the UStruct represents.
	 * @param JsonAttributes   Map of attributes to copy in to
	 *
	 * @return False if any properties failed to write
	 */
	bool UStructToJsonAttributes(const UStruct* StructDefinition, const void* const ContainerPtr,
								 TMap<FString, TSharedPtr<FJsonValue>>& OutJsonAttributes);

	/**
	 * Converts from a UStruct to a JSON Object
	 *
	 * @param StructDefinition UStruct definition that is looked over for properties
	 * @param ContainerPtr	   The object the UStruct represents.
	 * @param OutJsonObject    Json Object to be filled in with data from the ustruct
	 *
	 * @return False if faile to fill properties
	 */
	bool UStructToJsonObject(const UStruct* StructDefinition, const void* const ContainerPtr, TSharedRef<FJsonObject>& OutJsonObject)
	{
		return UStructToJsonAttributes(StructDefinition, ContainerPtr, OutJsonObject->Values);
	}

	/**
	 * Converts from a UStruct to a JSON string containing an object, using exportText
	 *
	 * @param StructDefinition UStruct definition that is looked over for properties
	 * @param ContainerPtr	   The object the UStruct represents.
	 * @param OutJsonString    Json Object to be filled in with data from the ustruct
	 *
	 * @return False if failed to serialize to string
	 */
	bool UStructToJsonString(const UStruct* StructDefinition, const void* const ContainerPtr, const DlgJsonWriterOptions& Options,
							 FString& OutJsonString);

	void ResetState()
	{
		IndexInArray = INDEX_NONE;
		bIsPropertyMapKey = false;
	}

private:
	// Final output string
	FString JsonString;

	/** Only properties that have these flags will be written. */
	static constexpr int64 CheckFlags = ~CPF_ParmFlags; // all properties except those who have these flags? TODO is this ok?

	// Used when parsing

	// If it is in an array this is != INDEX_NONE
	int32 IndexInArray = INDEX_NONE;

	// Is the current property used as a map key?
	bool bIsPropertyMapKey = false;
};
