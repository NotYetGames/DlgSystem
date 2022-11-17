// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgJsonWriter.h"

#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/PropertyPortFlags.h"

#include "DlgSystem/DlgHelper.h"
#include "DlgSystem/NYReflectionHelper.h"

DEFINE_LOG_CATEGORY(LogDlgJsonWriter);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgJsonWriter::Write(const UStruct* StructDefinition, const void* ContainerPtr)
{
	DlgJsonWriterOptions WriterOptions;
	WriterOptions.bPrettyPrint = true;
	WriterOptions.InitialIndent = 0;
	UStructToJsonString(StructDefinition, ContainerPtr, WriterOptions, JsonString);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSharedPtr<FJsonValue> FDlgJsonWriter::ConvertScalarPropertyToJsonValue(const FProperty* Property, const void* const ContainerPtr, const void* const ValuePtr)
{
	check(Property);
	if (bLogVerbose)
	{
		UE_LOG(LogDlgJsonWriter, Verbose, TEXT("ConvertScalarPropertyToJsonValue, Property = `%s`"), *Property->GetPathName());
	}
	if (ValuePtr == nullptr)
	{
		// Invalid
		return MakeShared<FJsonValueNull>();
	}

	//
	// NOTE: UProperty::ExportTextItem has the following arguments
	// FString& ValueStr, const void* PropertyValue, const void* DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope
	// We set the PropertyValue to be the same as DefaultValue, if DefaultValue is nullptr then this won't export the values different from default that the UProperty has.
	// This is only a problem if the target class is different from default (like it happens in tests).
	//

	// Get Json String for Enum definition
	auto GetJsonStringForEnum = [&ValuePtr](const UEnum* EnumDefinition, const FNumericProperty* NumericProperty) -> TSharedPtr<FJsonValue>
	{
		const FString StringValue = EnumDefinition->GetNameByIndex(NumericProperty->GetSignedIntPropertyValue(ValuePtr)).ToString();
		return MakeShared<FJsonValueString>(StringValue);
	};

	// Add Index Metadata to JsonObject
	auto AddIndexMetadata = [this, Property](TSharedRef<FJsonObject> JsonObject)
	{
		if (IndexInArray != INDEX_NONE && CanWriteIndex(Property))
		{
			JsonObject->SetField(TEXT("__index__"), MakeShared<FJsonValueNumber>(IndexInArray));
		}
	};

	// Enum, export enums as strings
	if (const auto* EnumProperty = FNYReflectionHelper::CastProperty<FEnumProperty>(Property))
	{
		return GetJsonStringForEnum(EnumProperty->GetEnum(), EnumProperty->GetUnderlyingProperty());
	}

	// Numeric, int, float, possible enum
	if (const auto* NumericProperty = FNYReflectionHelper::CastProperty<FNumericProperty>(Property))
	{
		// See if it's an enum Numeric property
		if (UEnum* EnumDef = NumericProperty->GetIntPropertyEnum())
		{
			return GetJsonStringForEnum(EnumDef, NumericProperty);
		}

		// We want to export numbers as numbers
		if (NumericProperty->IsInteger())
		{
			if (bIsPropertyMapKey)
			{
				// NOTE, because JSON only supports floats we do not use the FJsonValueNumber for map keys because integers
				// are displayed as floats. For example '42' is displayed as '42.0'
				// Instead we use it as a string, this should be similar as the parser can parse an int from string
				return MakeShared<FJsonValueString>(FString::Printf(TEXT("%lld"), NumericProperty->GetSignedIntPropertyValue(ValuePtr)));
			}

			return MakeShared<FJsonValueNumber>(NumericProperty->GetSignedIntPropertyValue(ValuePtr));
		}
		if (NumericProperty->IsFloatingPoint())
		{
			return MakeShared<FJsonValueNumber>(NumericProperty->GetFloatingPointPropertyValue(ValuePtr));
		}

		// Invalid
		return MakeShared<FJsonValueNull>();
	}

	// Bool, Export bools as JSON bools
	if (const auto* BoolProperty = FNYReflectionHelper::CastProperty<FBoolProperty>(Property))
	{
		return MakeShared<FJsonValueBoolean>(BoolProperty->GetOptionalPropertyValue(ValuePtr));
	}

	// FString
	if (const auto* StringProperty = FNYReflectionHelper::CastProperty<FStrProperty>(Property))
	{
		return MakeShared<FJsonValueString>(StringProperty->GetOptionalPropertyValue(ValuePtr));
	}

	// FName
	if (const auto* NameProperty = FNYReflectionHelper::CastProperty<FNameProperty>(Property))
	{
		auto* NamePtr = static_cast<const FName*>(ValuePtr);
		if (NamePtr == nullptr)
		{
			UE_LOG(LogDlgJsonWriter,
				   Error,
				   TEXT("Got Property = `%s` of type FNameProperty but the Value it not an FName"),
				   *NameProperty->GetName())
			return MakeShared<FJsonValueNull>();
		}
		if (!NamePtr->IsValidIndexFast() || !NamePtr->IsValid())
		{
			UE_LOG(LogDlgJsonWriter, Error, TEXT("Got Property = `%s` of type FName but it is not valid :("), *NameProperty->GetNameCPP())
			return MakeShared<FJsonValueNull>();
		}
		//check(NamePtr == NameProperty->GetPropertyValuePtr(Value));
		return MakeShared<FJsonValueString>(NamePtr->ToString());
	}

	// FText
	if (const auto* TextProperty = FNYReflectionHelper::CastProperty<FTextProperty>(Property))
	{
		return MakeShared<FJsonValueString>(TextProperty->GetOptionalPropertyValue(ValuePtr).ToString());
	}

	// TArray
	if (const auto* ArrayProperty = FNYReflectionHelper::CastProperty<FArrayProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> Array;
		const FDlgConstScriptArrayHelper Helper(ArrayProperty, ValuePtr);
		for (int32 Index = 0, Num = Helper.Num(); Index < Num; Index++)
		{
			IndexInArray = Index;
			TSharedPtr<FJsonValue> Elem = PropertyToJsonValue(ArrayProperty->Inner, ContainerPtr, Helper.GetConstRawPtr(Index));
			if (Elem.IsValid())
			{
				// add to the array
				Array.Push(Elem);
			}
		}

		ResetState();
		return MakeShared<FJsonValueArray>(Array);
	}

	// TSet
	if (const auto* SetProperty = FNYReflectionHelper::CastProperty<FSetProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> Array;
		const FScriptSetHelper Helper(SetProperty, ValuePtr);

		// GetMaxIndex() instead of Num() - the container is not contiguous
		// elements are in [0, GetMaxIndex[, some of them are invalid (Num() returns with the valid element num)
		for (int32 Index = 0; Index < Helper.GetMaxIndex(); Index++)
		{
			if (!Helper.IsValidIndex(Index))
			{
				continue;
			}

			IndexInArray = Index;
			TSharedPtr<FJsonValue> Elem = PropertyToJsonValue(SetProperty->ElementProp, ContainerPtr, Helper.GetElementPtr(Index));
			if (Elem.IsValid())
			{
				// add to the array
				Array.Push(Elem);
			}
		}

		ResetState();
		return MakeShared<FJsonValueArray>(Array);
	}

	// TMap
	if (const auto* MapProperty = FNYReflectionHelper::CastProperty<FMapProperty>(Property))
	{
		const TSharedRef<FJsonObject> OutObject = MakeShared<FJsonObject>();
		const FDlgConstScriptMapHelper Helper(MapProperty, ValuePtr);

		// GetMaxIndex() instead of Num() - the container is not contiguous
		// elements are in [0, GetMaxIndex[, some of them are invalid (Num() returns with the valid element num)
		for (int32 Index = 0; Index < Helper.GetMaxIndex(); Index++)
		{
			if (!Helper.IsValidIndex(Index))
			{
				continue;
			}
			IndexInArray = Index;

			bIsPropertyMapKey = true;
			const uint8* MapKeyPtr = Helper.GetConstKeyPtr(Index);
			const TSharedPtr<FJsonValue> KeyElement = PropertyToJsonValue(Helper.GetKeyProperty(), ContainerPtr, MapKeyPtr);

			bIsPropertyMapKey = false;
			const uint8* MapValuePtr = Helper.GetConstValuePtr(Index);
			const TSharedPtr<FJsonValue> ValueElement = PropertyToJsonValue(Helper.GetValueProperty(), ContainerPtr, MapValuePtr);

			if (KeyElement.IsValid() && ValueElement.IsValid())
			{
				check(MapKeyPtr);

				FString KeyString;
				if (auto* KeyStructProperty = FNYReflectionHelper::CastProperty<FStructProperty>(MapProperty->KeyProp))
				{
					// Key is a struct
#if NY_ENGINE_VERSION >= 501
					MapProperty->KeyProp->ExportTextItem_Direct(KeyString, MapKeyPtr, MapKeyPtr, nullptr, PPF_None);
#else
					MapProperty->KeyProp->ExportTextItem(KeyString, MapKeyPtr, MapKeyPtr, nullptr, PPF_None);
#endif
				}
				else
				{
					// Default to key string
					KeyString = KeyElement->AsString();
				}

				// Fallback for anything else, what could this be :O
				if (KeyString.IsEmpty())
				{

#if NY_ENGINE_VERSION >= 501
					MapProperty->KeyProp->ExportTextItem_Direct(KeyString, MapKeyPtr, MapKeyPtr, nullptr, PPF_None);
#else
					MapProperty->KeyProp->ExportTextItem(KeyString, MapKeyPtr, MapKeyPtr, nullptr, PPF_None);
#endif

					if (KeyString.IsEmpty())
					{
						UE_LOG(LogDlgJsonWriter, Error, TEXT("Unable to convert key to string for property `%s`."), *MapProperty->GetNameCPP())
						KeyString = FString::Printf(TEXT("Unparsed Key %d"), Index);
					}
				}

				OutObject->SetField(KeyString, ValueElement);
			}
		}

		ResetState();
		return MakeShared<FJsonValueObject>(OutObject);
	}

	// UStruct
	if (const auto* StructProperty = FNYReflectionHelper::CastProperty<FStructProperty>(Property))
	{
		// Intentionally exclude the JSON Object wrapper, which specifically needs to export JSON in an object representation instead of a string
		UScriptStruct::ICppStructOps* TheCppStructOps = StructProperty->Struct->GetCppStructOps();
		if (StructProperty->Struct != FJsonObjectWrapper::StaticStruct() && TheCppStructOps && TheCppStructOps->HasExportTextItem())
		{
			// Export to native text
			FString OutValueStr;
			TheCppStructOps->ExportTextItem(OutValueStr, ValuePtr, ValuePtr, nullptr, PPF_None, nullptr);
			return MakeShared<FJsonValueString>(OutValueStr);
		}

		// Handle Struct
		TSharedRef<FJsonObject> OutObject = MakeShared<FJsonObject>();
		AddIndexMetadata(OutObject);
		if (UStructToJsonObject(StructProperty->Struct, ValuePtr, OutObject))
		{
			return MakeShared<FJsonValueObject>(OutObject);
		}

		// Invalid
		return MakeShared<FJsonValueNull>();
	}

	// UObject
	if (const auto* ObjectProperty = FNYReflectionHelper::CastProperty<FObjectProperty>(Property))
	{
		auto returnNullptr = [this, &ObjectProperty]() -> TSharedPtr<FJsonValue>
		{
			// Save reference as empty string
			if (CanSaveAsReference(ObjectProperty, nullptr))
			{
				return MakeShared<FJsonValueString>(TEXT(""));
			}

			return MakeShared<FJsonValueNull>();
		};

		// NOTE: The ValuePtr here should be a pointer to a pointer
		// Because the UObjects are pointers, we must deference it. So instead of it being a void** we want it to be a void*
		const UObject* ObjectPtr = ObjectProperty->GetObjectPropertyValue_InContainer(ValuePtr);

		// To find out if in nested containers the object is nullptr we must go a level up
		const UObject* ContainerObjectPtr = ObjectProperty->GetObjectPropertyValue_InContainer(ContainerPtr);
		if (ObjectPtr == nullptr || ContainerObjectPtr == nullptr)
		{
			// We can have nullptrs
			if (bLogVerbose)
			{
				UE_LOG(
					LogDlgJsonWriter,
					Verbose,
					TEXT("Property = `%s` Is a FObjectProperty but got null from ContainerPtrToValuePtr from it's StructObject (NOTE: UObjects can be nullptrs)"),
					*Property->GetPathName()
				);
			}
			return returnNullptr();
		}
		if (!ObjectPtr->IsValidLowLevelFast())
		{
			// Memory corruption?
			UE_LOG(
				LogDlgJsonWriter,
				Error,
				TEXT("ObjectPtr.IsValidLowLevelFast is false for Property = `%s`. Memory corruption for UObjects?"),
				*Property->GetPathName()
			);
			return returnNullptr();
		}

		// Special case were we want just to save a reference to the object location
		if (ObjectPtr != nullptr && CanSaveAsReference(ObjectProperty, ObjectPtr))
		{
			return MakeShared<FJsonValueString>(ObjectPtr->GetPathName());
		}

		// Save as normal JSON Object
		TSharedRef<FJsonObject> OutObject = MakeShared<FJsonObject>();
		AddIndexMetadata(OutObject);

		// Set the uproperties of the object
		const UClass* ObjectClass = ObjectProperty->PropertyClass;
		if (UStructToJsonObject(ObjectClass, ObjectPtr, OutObject))
		{
			return MakeShared<FJsonValueObject>(OutObject);
		}

		// Invalid
		return MakeShared<FJsonValueNull>();
	}

	// Default, convert to string
	FString ValueString;
#if NY_ENGINE_VERSION >= 501
	Property->ExportTextItem_Direct(ValueString, ValuePtr, ValuePtr, nullptr, PPF_None);
#else
	Property->ExportTextItem(ValueString, ValuePtr, ValuePtr, nullptr, PPF_None);
#endif

	return MakeShared<FJsonValueString>(ValueString);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSharedPtr<FJsonValue> FDlgJsonWriter::PropertyToJsonValue(const FProperty* Property, const void* const ContainerPtr, const void* const ValuePtr)
{
	check(Property);
	if (bLogVerbose)
	{
		UE_LOG(LogDlgJsonWriter, Verbose, TEXT("PropertyToJsonValue, Property = `%s`"), *Property->GetPathName());
	}

	if (ContainerPtr == nullptr || ValuePtr == nullptr)
	{
		const auto* PropertyClass = Property->GetClass();
		if (Property->IsA<FObjectProperty>())
		{
			// Object property, can be nullptr
			if (bLogVerbose)
			{
				UE_LOG(
					LogDlgJsonWriter,
					Verbose,
					TEXT("UStructToJsonObject - Unhandled property type Class = '%s', Name = `%s`. (NOTE: UObjects can be nullptrs)"),
					*PropertyClass->GetName(), *Property->GetPathName()
				);
			}
		}
		else
		{
			UE_LOG(
				LogDlgJsonWriter,
				Error,
				TEXT("UStructToJsonObject - Unhandled property type Class = '%s', Name = `%s`"),
				*PropertyClass->GetName(), *Property->GetNameCPP()
			);
		}

		return MakeShared<FJsonValueNull>();
	}

	// Scalar Only one property
	if (Property->ArrayDim == 1)
	{
		return ConvertScalarPropertyToJsonValue(Property, ContainerPtr, ValuePtr);
	}

	// Array
	// NOTE: we can't use here ArrayHelper, because then we might also need to use SetHelper, more code, meh
	TArray<TSharedPtr<FJsonValue>> Array;
	auto* ValueIntPtr = static_cast<const uint8*>(ValuePtr);
	for (int Index = 0; Index < Property->ArrayDim; Index++)
	{
		IndexInArray = Index;

		// ValuePtr + Index * Property->ElementSize is literally FScriptArrayHelper::GetRawPtr
		const TSharedPtr<FJsonValue> JsonValue = ConvertScalarPropertyToJsonValue(Property, ContainerPtr, ValueIntPtr + Index * Property->ElementSize);
		if (JsonValue.IsValid())
		{
			Array.Add(JsonValue);
		}
	}

	ResetState();
	return MakeShared<FJsonValueArray>(Array);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgJsonWriter::UStructToJsonAttributes(const UStruct* StructDefinition, const void* const ContainerPtr, TMap<FString, TSharedPtr<FJsonValue>>& OutJsonAttributes)
{
	if (StructDefinition == nullptr || ContainerPtr == nullptr)
	{
		return false;
	}
	if (bLogVerbose)
	{
		UE_LOG(LogDlgJsonWriter, Verbose, TEXT("UStructToJsonAttributes, StructDefinition = `%s`"), *StructDefinition->GetPathName());
	}

	// Json Wrapper, already have an Object
	if (StructDefinition == FJsonObjectWrapper::StaticStruct())
	{
		// Just copy it into the object
		const FJsonObjectWrapper* ProxyObject = static_cast<const FJsonObjectWrapper*>(ContainerPtr);
		if (ProxyObject->JsonObject.IsValid())
		{
			OutJsonAttributes = ProxyObject->JsonObject->Values;
		}

		return true;
	}

	// Handle UObject inheritance (children of class)
	if (StructDefinition->IsA<UClass>())
	{
		const UObject* UnrealObject = static_cast<const UObject*>(ContainerPtr);
		if (!UnrealObject->IsValidLowLevelFast())
		{
			UE_LOG(
				LogDlgJsonWriter,
				Error,
				TEXT("UStructToJsonObject: StructDefinition = `%s` is a UClass and expected ContainerPtr to be an UObject. Memory corruption?"),
				*StructDefinition->GetPathName()
			);
			return false;
		}

		// Write type, Objects because they can have inheritance
		OutJsonAttributes.Add(TEXT("__type__"), MakeShared<FJsonValueString>(UnrealObject->GetClass()->GetName()));

		// Structure points to the child
		StructDefinition = UnrealObject->GetClass();
	}
	if (!StructDefinition->IsValidLowLevelFast())
	{
		UE_LOG(
			LogDlgJsonWriter,
			Error,
			TEXT("UStructToJsonObject: StructDefinition = `%s` is a UClass and expected ContainerPtr.Class to be valid. Memory corruption?"),
			*StructDefinition->GetPathName()
		);
		return false;
	}

	// Iterate over all the properties of the struct
	for (TFieldIterator<const FProperty> It(StructDefinition); It; ++It)
	{
		const auto* Property = *It;
		if (!ensure(Property))
			continue;

		// Check to see if we should ignore this property
		if (CheckFlags != 0 && !Property->HasAnyPropertyFlags(CheckFlags))
		{
			// Property does not have the required Flags
			if (bLogVerbose)
			{
				UE_LOG(LogDlgJsonWriter, Verbose, TEXT("Property = `%s` Does not have the required CheckFlags"), *Property->GetPathName());
			}
			continue;
		}
		if (CanSkipProperty(Property))
		{
			// Mark as skipped.
			if (bLogVerbose)
			{
				UE_LOG(LogDlgJsonWriter, Verbose, TEXT("Property = `%s` Marked as skiped"), *Property->GetPathName());
			}
			continue;
		}

		// Get the Pointer to the Value
		const void* ValuePtr = nullptr;
		if (Property->IsA<FObjectProperty>())
		{
			// Handle pointers, only allowed to be UObjects (are already pointers to the Value)
			ValuePtr = ContainerPtr;
		}
		else
		{
			// Normal non pointer property
			ValuePtr = Property->ContainerPtrToValuePtr<void>(ContainerPtr, 0);
		}

		// convert the property to a FJsonValue
		const TSharedPtr<FJsonValue> JsonValue = PropertyToJsonValue(Property, ContainerPtr, ValuePtr);
		if (!JsonValue.IsValid())
		{
			const auto* PropertyClass = Property->GetClass();
			if (Property->IsA<FObjectProperty>())
			{
				// Object property, can be nullptr
				if (bLogVerbose)
				{
					UE_LOG(
						LogDlgJsonWriter,
						Verbose,
						TEXT("UStructToJsonObject - Unhandled property, Class = `%s`, Name =`%s`, inside Struct = `%s`. (NOTE: UObjects can be nullptrs)"),
						*PropertyClass->GetName(), *Property->GetPathName(), *StructDefinition->GetPathName()
					);
				}
			}
			else
			{
				UE_LOG(
					LogDlgJsonWriter,
					Warning,
					TEXT("UStructToJsonObject - Unhandled property, Class = `%s`, Name =`%s`, inside Struct = `%s`"),
					*PropertyClass->GetName(), *Property->GetPathName(), *StructDefinition->GetPathName()
				);
			}

			continue;
		}

		// set the value on the output object
		// NOTE default JSON writer makes the first letter to be lowercase, we do not want that ;) FJsonObjectConverter::StandardizeCase
		const FString VariableName = Property->GetName();
		OutJsonAttributes.Add(VariableName, JsonValue);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<class CharType, class PrintPolicy>
bool UStructToJsonStringInternal(const TSharedRef<FJsonObject>& JsonObject, const int32 InitialIndent, FString& OutJsonString)
{
	TSharedRef<TJsonWriter<CharType, PrintPolicy>> JsonWriter =
			TJsonWriterFactory<CharType, PrintPolicy>::Create(&OutJsonString, InitialIndent);
	const bool bSuccess = FJsonSerializer::Serialize(JsonObject, JsonWriter);
	JsonWriter->Close();
	return bSuccess;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgJsonWriter::UStructToJsonString(const UStruct* StructDefinition, const void* const ContainerPtr,
	 const DlgJsonWriterOptions& Options, FString& OutJsonString)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	if (UStructToJsonObject(StructDefinition, ContainerPtr, JsonObject))
	{
		bool bSuccess;
		if (Options.bPrettyPrint)
		{
			bSuccess = UStructToJsonStringInternal<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>(JsonObject, Options.InitialIndent, OutJsonString);
		}
		else
		{
			bSuccess = UStructToJsonStringInternal<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>(JsonObject, Options.InitialIndent, OutJsonString);
		}

		if (bSuccess)
		{
			return true;
		}
	}

	UE_LOG(LogDlgJsonWriter, Error, TEXT("UStructToJsonObjectString - Unable to write out json"));
	return false;
}
