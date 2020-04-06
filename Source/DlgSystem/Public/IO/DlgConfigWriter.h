// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include <functional>
#include "Logging/LogMacros.h"
#include "UObject/UnrealType.h"
#include "Misc/FileHelper.h"
#include "UObject/TextProperty.h"

#include "IDlgWriter.h"
#include "NYReflectionHelper.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgConfigWriter, Log, All);



/**
 * Because there is always another config format
 * And there is always a copy-pasted comment.
 * See IDlgWriter for properties and METADATA specifiers.
 */
class DLGSYSTEM_API FDlgConfigWriter : public IDlgWriter
{

public:

	/**
	 * Creates a config string from the input struct/object
	 * All property is saved recursively except the ones with the metadata "DlgNoExport" and the nodes with the flags
	 * defined in SkipFlags.
	 * Call ExportToFile() to save it
	 *
	 * Check FDlgConfigParser for the list of supported constructions (some container is limited in some ways)
	 * If there is a nullptr in the middle of an array it is skipped, the array num will be different after parsing it back,
	 * unless it is written by reference
	 *
	 * @param StructDefinition: UStruct of Object
	 * @param Object: pointer to a variable (Struct / UObject)
	 * @param InComplexNamePrefix: project specific pretag in struct and class names
								   can be used not to export it to have better readable config file
	 * @param bInDontWriteEmptyContainer: if it is enabled empty TSet, TMap and TArray properties won't be written
	 */
	FDlgConfigWriter(const FString InComplexNamePrefix = "",
					 bool bInDontWriteEmptyContainer = true);

	// IDlgWriter Interface

	/** Has to be called to prepare the text data */
	void Write(const UStruct* StructDefinition, const void* Object) override;

	/**
	 * Save the config string to a text file
	 * @param FullName: Full path + file name + extension
	 */
	bool ExportToFile(const FString& FileName) override
	{
		return FFileHelper::SaveStringToFile(ConfigText, *FileName, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	const FString& GetAsString() const override
	{
		return ConfigText;
	}

protected:

	void WriteComplexToString(const UStruct* StructDefinition,
							  const FNYProperty* Property,
							  const void* Object,
							  const FString& PreString,
							  const FString& PostString,
							  bool bContainerElement,
							  bool bWriteType,
							  FString& Target);

	void WriteComplexMembersToString(const UStruct* StructDefinition,
									 const void* Object,
									 const FString& PreString,
									 const FString& PostString,
									 FString& Target);

	bool WritePropertyToString(const FNYProperty* Property,
							   const void* Object,
							   bool bContainerElement,
							   const FString& PreString,
							   const FString& PostString,
							   bool bPointerAsRef,
							   FString& Target);

	// object is pointer to the owner
	bool WritePrimitiveElementToString(const FNYProperty* Property,
									   const void* Object,
									   bool bContainerElement,
									   const FString& PreString,
									   const FString& PostString,
									   FString& Target);

	bool WritePrimitiveArrayToString(const FNYProperty* Property,
									 const void* Object,
									 const FString& PreString,
									 const FString& PostString,
									 FString& Target);

	bool WriteComplexElementToString(const FNYProperty* Property,
									 const void* Object,
									 bool bContainerElement,
									 const FString& PreString,
									 const FString& PostString,
									 bool bPointerAsRef,
									 FString& Target);

	bool WriteComplexArrayToString(const FNYProperty* Property,
								   const void* Object,
								   const FString& PreString,
								   const FString& PostString,
								   FString& Target);

	bool WriteMapToString(const FNYProperty* Property,
						  const void* Object,
						  const FString& PreString,
						  const FString& PostString,
						  FString& Target);

	bool WriteSetToString(const FNYProperty* Property,
						  const void* Object,
						  const FString& PreString,
						  const FString& PostString,
						  FString& Target);

	bool IsPrimitive(const FNYProperty* Property);
	bool IsContainer(const FNYProperty* Property);
	bool IsPrimitiveContainer(const FNYProperty* Property);

	bool WouldWriteNonPrimitive(const UStruct* StructDefinition, const void* Owner);

	const UStruct* GetComplexType(const FNYProperty* Property);


	// expects object or struct property, returns empty string otherwise
	FString GetNameWithoutPrefix(const FNYProperty* StructDefinition, const UObject* ObjectPtr = nullptr);
	FString GetStringWithoutPrefix(const FString& String);


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename PropertyType, typename VariableType>
	bool WritePrimitiveElementToStringTemplated(const FNYProperty* Property,
												const void* Object,
												bool bContainerElement,
												std::function<FString(const VariableType&)> GetAsString,
												const FString& PreString,
												const FString& PostString,
												FString& Target)
	{
		const PropertyType* CastedProperty = FNYReflectionHelper::CastProperty<PropertyType>(Property);
		if (CastedProperty != nullptr)
		{
			Target += PreString +
					  (bContainerElement ? "" : (CastedProperty->GetName() + " ")) +
					  GetAsString(CastedProperty->GetPropertyValue_InContainer(Object, 0)) +
					  PostString;
			return true;
		}

		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename PropertyType, typename VariableType>
	bool WritePrimitiveArrayToStringTemplated(const FNYArrayProperty* ArrayProp,
											  const void* Object,
											  std::function<FString(const VariableType&)> ToString,
											  const FString& PreString,
											  const FString& PostString,
											  FString& Target)
	{
		if (FNYReflectionHelper::CastProperty<PropertyType>(ArrayProp->Inner) == nullptr)
		{
			return false;
		}

		const TArray<VariableType>* ArrayPtr = ArrayProp->ContainerPtrToValuePtr<TArray<VariableType>>(Object);
		if (ArrayPtr == nullptr)
		{
			return false;
		}
		const bool bLinePerItem = CanWriteOneLinePerItem(ArrayProp);

		// Empty array
		const TArray<VariableType> Array = *ArrayPtr;
		if (Array.Num() == 0 && bDontWriteEmptyContainer)
		{
			return true;
		}

		// Establish indentation to be the same as the ArrayName.len + 3 spaces
		FString SubPreString = PreString;
		for (int32 i = 0; i < ArrayProp->GetName().Len() + 3; ++i)
		{
			SubPreString += " ";
		}

		// ArrayName {
		Target += PreString + ArrayProp->GetName() + " {" + (bLinePerItem ? EOL_String : " ");

		// Array content
		for (int32 i = 0; i < Array.Num(); ++i)
		{
			if (bLinePerItem)
			{
				Target += SubPreString + ToString(Array[i]) + EOL;
			}
			else
			{
				Target += ToString(Array[i]) + " ";
			}
		}

		// }
		Target += (bLinePerItem ? PreString : "") + "}" + PostString;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename PropertyType, typename VariableType>
	bool WritePrimitiveToStringTemplated(const FNYProperty* Property,
										 const void* Object,
										 bool bContainerElement,
										 std::function<FString(const VariableType&)> GetAsString,
										 const FString& PreString,
										 const FString& PostString,
										 FString& Target)
	{
		const PropertyType* CastedProperty = FNYReflectionHelper::CastProperty<PropertyType>(Property);
		if (CastedProperty != nullptr)
		{
			Target += PreString +
					  (bContainerElement ? "" : (CastedProperty->GetName() + " ")) +
					  GetAsString(*((VariableType*)(Object))) +
					  PostString;
			return true;
		}

		return false;
	}

	/** Converts all endlines to be of one type. */
	static FString NormalizeEndlines(const FString& Original)
	{
		return Original.Replace(EOL_CRLF, EOL_LF, ESearchCase::IgnoreCase);
	}

private:
	// End of line
	static const TCHAR* EOL_LF;
	static const TCHAR* EOL_CRLF;
	static const TCHAR* EOL;

	// Helper strings
	static const FString EOL_String;

	FString ConfigText = "";

	const void* TopLevelObjectPtr = nullptr;
	const FString ComplexNamePrefix;
	const bool bDontWriteEmptyContainer;

	// Conversion to string functions
	const std::function<FString(const bool&)> BoolToString = [](const bool& bBool) -> FString
	{
		return bBool ? "True" : "False";
	};

	const std::function<FString(const int64&)> IntToString = [](const int64& IntVal) -> FString
	{
		return FString::Printf(TEXT("%lld"), IntVal);
	};

	const std::function<FString(const float&)> FloatToString = [](const float& FloatVal) -> FString
	{
		return FString::SanitizeFloat(FloatVal);
	};

	const std::function<FString(const FString&)> StringToString = [](const FString& String) -> FString
	{
		return FString("\"") + NormalizeEndlines(String) + "\"";
	};

	const std::function<FString(const FName&)> NameToString = [](const FName& Name) -> FString
	{
		return FString("\"") + NormalizeEndlines(Name.ToString()) + "\"";
	};

	const std::function<FString(const FText&)> TextToString = [](const FText& Text) -> FString
	{
		return FString("\"") + NormalizeEndlines(Text.ToString()) + "\"";
	};
};
