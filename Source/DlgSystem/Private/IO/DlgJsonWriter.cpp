// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgJsonWriter.h"
#include "JsonUtilities.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "TextProperty.h"
#include "PropertyPortFlags.h"
#include "DlgHelper.h"

DEFINE_LOG_CATEGORY(LogDlgJsonWriter);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgJsonWriter::Write(const UStruct* StructDefinition, const void* Object)
{
	DlgJsonWriterOptions WriterOptions;
	WriterOptions.bPrettyPrint = true;
	WriterOptions.InitialIndent = 0;
	UStructToJsonString(StructDefinition, Object, WriterOptions, JsonString);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSharedPtr<FJsonValue> FDlgJsonWriter::ConvertScalarUPropertyToJsonValue(const UProperty* Property, const void* Value)
{
	check(Property);
	if (bLogVerbose)
	{
		UE_LOG(LogDlgJsonWriter, Verbose, TEXT("ConvertScalarUPropertyToJsonValue, Property = `%s`"), *Property->GetPathName());
	}
	if (Value == nullptr)
	{
		// Invalid
		return MakeShareable(new FJsonValueNull());
	}

	//
	// NOTE: UProperty::ExportTextItem has the following arguments
	// FString& ValueStr, const void* PropertyValue, const void* DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope
	// We set the PropertyValue to be the same as DefaultValue, if DefaultValue is nullptr then this won't export the values different from default that the UProperty has.
	// This is only a problem if the target class is different from default (like it happens in tests).
	//

	// Get Json String for Enum definition
	auto GetJsonStringForEnum = [&Value](const UEnum* EnumDefinition, const UNumericProperty* NumericProperty) -> TSharedPtr<FJsonValue>
	{
		const FString StringValue = EnumDefinition->GetNameByIndex(NumericProperty->GetSignedIntPropertyValue(Value)).ToString();
		return MakeShareable(new FJsonValueString(StringValue));
	};

	// Add Index Metadata to JsonObject
	auto AddIndexMetadata = [this, Property](TSharedRef<FJsonObject> JsonObject)
	{
		if (IndexInArray != INDEX_NONE && CanWriteIndex(Property))
		{
			JsonObject->SetField(TEXT("__index__"), MakeShareable(new FJsonValueNumber(IndexInArray)));
		}
	};

	// Enum, export enums as strings
	if (const UEnumProperty* EnumProperty = Cast<UEnumProperty>(Property))
	{
		return GetJsonStringForEnum(EnumProperty->GetEnum(), EnumProperty->GetUnderlyingProperty());
	}

	// Numeric, int, float, possible enum
	if (const UNumericProperty* NumericProperty = Cast<UNumericProperty>(Property))
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
				return MakeShareable(new FJsonValueString(FString::FromInt(NumericProperty->GetSignedIntPropertyValue(Value))));
			}
			else
			{
				return MakeShareable(new FJsonValueNumber(NumericProperty->GetSignedIntPropertyValue(Value)));
			}
		}
		if (NumericProperty->IsFloatingPoint())
		{
			return MakeShareable(new FJsonValueNumber(NumericProperty->GetFloatingPointPropertyValue(Value)));
		}

		// Invalid
		return MakeShareable(new FJsonValueNull());
	}

	// Bool, Export bools as JSON bools
	if (const UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
	{
		return MakeShareable(new FJsonValueBoolean(BoolProperty->GetPropertyValue(Value)));
	}

	// FString
	if (const UStrProperty* StringProperty = Cast<UStrProperty>(Property))
	{
		return MakeShareable(new FJsonValueString(StringProperty->GetPropertyValue(Value)));
	}

	// FName
	if (const UNameProperty* NameProperty = Cast<UNameProperty>(Property))
	{
		auto* NamePtr = static_cast<const FName*>(Value);
		if (NamePtr == nullptr)
		{
			UE_LOG(LogDlgJsonWriter,
				   Error,
				   TEXT("Got Property = `%s` of type UNameProperty but the Value it not an FName"),
				   *NameProperty->GetName())
			return MakeShareable(new FJsonValueNull());
		}
		if (!NamePtr->IsValidIndexFast() || !NamePtr->IsValid())
		{
			UE_LOG(LogDlgJsonWriter, Error, TEXT("Got Property = `%s` of type FName but it is not valid :("), *NameProperty->GetNameCPP())
			return MakeShareable(new FJsonValueNull());
		}
		//check(NamePtr == NameProperty->GetPropertyValuePtr(Value));
		return MakeShareable(new FJsonValueString(NamePtr->ToString()));
	}

	// FText
	if (const UTextProperty* TextProperty = Cast<UTextProperty>(Property))
	{
		return MakeShareable(new FJsonValueString(TextProperty->GetPropertyValue(Value).ToString()));
	}

	// TArray
	if (const UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> Array;
		FScriptArrayHelper Helper(ArrayProperty, Value);
		for (int32 Index = 0, Num = Helper.Num(); Index < Num; Index++)
		{
			IndexInArray = Index;
			TSharedPtr<FJsonValue> Elem = UPropertyToJsonValue(ArrayProperty->Inner, Helper.GetRawPtr(Index));
			if (Elem.IsValid())
			{
				// add to the array
				Array.Push(Elem);
			}
		}

		ResetState();
		return MakeShareable(new FJsonValueArray(Array));
	}

	// TSet
	if (const USetProperty* SetProperty = Cast<USetProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> Array;
		FScriptSetHelper Helper(SetProperty, Value);
		for (int32 Index = 0, Num = Helper.Num(); Index < Num; Index++)
		{
			IndexInArray = Index;
			TSharedPtr<FJsonValue> Elem = UPropertyToJsonValue(SetProperty->ElementProp, Helper.GetElementPtr(Index));
			if (Elem.IsValid())
			{
				// add to the array
				Array.Push(Elem);
			}
		}

		ResetState();
		return MakeShareable(new FJsonValueArray(Array));
	}

	// TMap
	if (const UMapProperty* MapProperty = Cast<UMapProperty>(Property))
	{
		const TSharedRef<FJsonObject> OutObject = MakeShareable(new FJsonObject());

		FScriptMapHelper Helper(MapProperty, Value);
		for (int32 Index = 0, Num = Helper.Num(); Index < Num; Index++)
		{
			if (!Helper.IsValidIndex(Index))
			{
				continue;
			}

			IndexInArray = Index;

			bIsPropertyMapKey = true;
			const uint8* KeyPtr = Helper.GetKeyPtr(Index);
			const TSharedPtr<FJsonValue> KeyElement = UPropertyToJsonValue(MapProperty->KeyProp, KeyPtr);

			bIsPropertyMapKey = false;
			const uint8* ValuePtr = Helper.GetValuePtr(Index);
			//if (const UObjectProperty* ValueObjectProperty = Cast<UObjectProperty>(MapProperty->ValueProp))
			const TSharedPtr<FJsonValue> ValueElement = UPropertyToJsonValue(MapProperty->ValueProp, ValuePtr);

			if (KeyElement.IsValid() && ValueElement.IsValid())
			{
				check(KeyPtr);

				FString KeyString;
				if (UStructProperty* KeyStructProperty = Cast<UStructProperty>(MapProperty->KeyProp))
				{
					// Key is a struct
					MapProperty->KeyProp->ExportTextItem(KeyString, KeyPtr, KeyPtr, nullptr, PPF_None);
				}
				else
				{
					// Default to key string
					KeyString = KeyElement->AsString();
				}

				// Fallback for anything else, what could this be :O
				if (KeyString.IsEmpty())
				{
					MapProperty->KeyProp->ExportTextItem(KeyString, KeyPtr, KeyPtr, nullptr, PPF_None);

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
		return MakeShareable(new FJsonValueObject(OutObject));
	}

	// UStruct
	if (const UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		// Intentionally exclude the JSON Object wrapper, which specifically needs to export JSON in an object representation instead of a string
		UScriptStruct::ICppStructOps* TheCppStructOps = StructProperty->Struct->GetCppStructOps();
		if (StructProperty->Struct != FJsonObjectWrapper::StaticStruct() && TheCppStructOps && TheCppStructOps->HasExportTextItem())
		{
			// Export to native text
			FString OutValueStr;
			TheCppStructOps->ExportTextItem(OutValueStr, Value, Value, nullptr, PPF_None, nullptr);
			return MakeShareable(new FJsonValueString(OutValueStr));
		}

		// Handle Struct
		const TSharedRef<FJsonObject> OutObject = MakeShareable(new FJsonObject());
		AddIndexMetadata(OutObject);
		if (UStructToJsonObject(StructProperty->Struct, Value, OutObject))
		{
			return MakeShareable(new FJsonValueObject(OutObject));
		}

		// Invalid
		return MakeShareable(new FJsonValueNull());
	}

	// UObject
	if (const UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property))
	{
		// NOTE: The Value here should be a pointer to a pointer
		const UObject* UnrealObject = static_cast<const UObject*>(Value);
		if (UnrealObject == nullptr)
		{
			if (bLogVerbose)
			{
				UE_LOG(LogDlgJsonWriter,
					Verbose,
					TEXT("Property = `%s` Is a UObjectProperty but can't convert Value to an UObject (NOTE: UObjects can be nullptrs)"),
					*Property->GetPathName());
			}
			// Save reference as empty string
			if (CanSaveAsReference(ObjectProperty))
			{
				return MakeShareable(new FJsonValueString(TEXT("")));
			}

			return MakeShareable(new FJsonValueNull());
		}

		// Because the UObjects are pointers, we must deference it. So instead of it being a void** we want it to be a void*
		auto* ObjectPtrPtr = static_cast<const UObject* const*>(ObjectProperty->ContainerPtrToValuePtr<void>(Value, 0));
		if (ObjectPtrPtr == nullptr)
		{
			if (bLogVerbose)
			{
				UE_LOG(LogDlgJsonWriter,
					Verbose,
					TEXT("Property = `%s` Is a UObjectProperty but got null from ContainerPtrToValuePtr from it's StructObject (NOTE: UObjects can be nullptrs)"),
					*Property->GetPathName());
			}
			// Save reference as empty string
			if (CanSaveAsReference(ObjectProperty))
			{
				return MakeShareable(new FJsonValueString(TEXT("")));
			}

			return MakeShareable(new FJsonValueNull());
		}
		const UObject* ObjectPtr = *ObjectPtrPtr;
		const UClass* ObjectClass = ObjectProperty->PropertyClass;

		if (ObjectPtr)
		{
			// Special case were we want just to save a reference to the object location
			if (CanSaveAsReference(ObjectProperty))
			{
				return MakeShareable(new FJsonValueString(ObjectPtr->GetPathName()));
			}
			else
			{
				// Save as normal JSON Object
				TSharedRef<FJsonObject> OutObject = MakeShareable(new FJsonObject());
				AddIndexMetadata(OutObject);

				// Set the uproperties of the object
				if (UStructToJsonObject(ObjectClass, ObjectPtr, OutObject))
				{
					return MakeShareable(new FJsonValueObject(OutObject));
				}
			}
		}

		// Invalid
		return MakeShareable(new FJsonValueNull());
	}

	// Default, convert to string
	FString ValueString;
	Property->ExportTextItem(ValueString, Value, Value, nullptr, PPF_None);
	return MakeShareable(new FJsonValueString(ValueString));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSharedPtr<FJsonValue> FDlgJsonWriter::UPropertyToJsonValue(const UProperty* Property, const void* Value)
{
	check(Property);
	if (bLogVerbose)
	{
		UE_LOG(LogDlgJsonWriter, Verbose, TEXT("UPropertyToJsonValue, Property = `%s`"), *Property->GetPathName());
	}

	if (Value == nullptr)
	{
		const UClass* PropertyClass = Property->GetClass();
		if (Property->IsA<UObjectProperty>())
		{
			// Object property, can be nullptr
			if (bLogVerbose)
			{
				UE_LOG(LogDlgJsonWriter,
					Verbose,
					TEXT("UStructToJsonObject - Unhandled property type Class = '%s', Name = `%s`. (NOTE: UObjects can be nullptrs)"),
					*PropertyClass->GetName(), *Property->GetPathName());
			}
		}
		else
		{
			UE_LOG(LogDlgJsonWriter,
				Error,
				TEXT("UStructToJsonObject - Unhandled property type Class = '%s', Name = `%s`"),
				*PropertyClass->GetName(), *Property->GetNameCPP());
		}

		return MakeShareable(new FJsonValueNull());
	}

	// Scalar Only one property
	if (Property->ArrayDim == 1)
	{
		return ConvertScalarUPropertyToJsonValue(Property, Value);
	}

	// Array
	// NOTE: we can't use here ArrayHelper, because then we might also need to use SetHelper, more code, meh
	TArray<TSharedPtr<FJsonValue>> Array;
	auto ValuePtr = static_cast<const uint8*>(Value);
	check(ValuePtr);
	for (int Index = 0; Index < Property->ArrayDim; Index++)
	{
		IndexInArray = Index;

		// ValuePtr + Index * Property->ElementSize is literally FScriptArrayHelper::GetRawPtr
		const TSharedPtr<FJsonValue> JsonValue = ConvertScalarUPropertyToJsonValue(Property, ValuePtr + Index * Property->ElementSize);
		if (JsonValue.IsValid())
		{
			Array.Add(JsonValue);
		}
	}

	ResetState();
	return MakeShareable(new FJsonValueArray(Array));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgJsonWriter::UStructToJsonAttributes(const UStruct* StructDefinition, const void* Object,
	TMap<FString, TSharedPtr<FJsonValue>>& OutJsonAttributes)
{
	if (StructDefinition == nullptr || Object == nullptr)
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
		const FJsonObjectWrapper* ProxyObject = static_cast<const FJsonObjectWrapper*>(Object);
		if (ProxyObject->JsonObject.IsValid())
		{
			OutJsonAttributes = ProxyObject->JsonObject->Values;
		}

		return true;
	}

	// Handle UObject inheritance (children of class)
	if (StructDefinition->IsA<UClass>())
	{
		const UObject* UnrealObject = static_cast<const UObject*>(Object);
		if (UnrealObject == nullptr || UnrealObject->GetClass() == nullptr)
		{
			return false;
		}

		// Write type, Objects because they can have inheritance
		OutJsonAttributes.Add(TEXT("__type__"), MakeShareable(new FJsonValueString(UnrealObject->GetClass()->GetName())));

		// Structure points to the child
		StructDefinition = UnrealObject->GetClass();
		check(StructDefinition);
	}

	// Iterate over all the properties of the struct
	for (TFieldIterator<const UProperty> It(StructDefinition); It; ++It)
	{
		const UProperty* Property = *It;
		check(Property);

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
			// Markes as skiped.
			if (bLogVerbose)
			{
				UE_LOG(LogDlgJsonWriter, Verbose, TEXT("Property = `%s` Marked as skiped"), *Property->GetPathName());
			}
			continue;
		}

		// Get the Pointer to the Value
		const void* ValuePtr = nullptr;
		if (Property->IsA<UObjectProperty>())
		{
			// Handle pointers, only allowed to be UObjects (are already pointers to the Value)
			check(static_cast<const UObject*>(Object) != nullptr);
			ValuePtr = Object;
		}
		else
		{
			// Normal non pointer property
			ValuePtr = Property->ContainerPtrToValuePtr<void>(Object, 0);
		}

		// convert the property to a FJsonValue
		const TSharedPtr<FJsonValue> JsonValue = UPropertyToJsonValue(Property, ValuePtr);
		if (!JsonValue.IsValid())
		{
			const UClass* PropertyClass = Property->GetClass();
			if (Property->IsA<UObjectProperty>())
			{
				// Object property, can be nullptr
				if (bLogVerbose)
				{
					UE_LOG(LogDlgJsonWriter,
						Verbose,
						TEXT("UStructToJsonObject - Unhandled property, Class = `%s`, Name =`%s`, inside Struct = `%s`. (NOTE: UObjects can be nullptrs)"),
						*PropertyClass->GetName(), *Property->GetPathName(), *StructDefinition->GetPathName());
				}
			}
			else
			{
				UE_LOG(LogDlgJsonWriter,
					Warning,
					TEXT("UStructToJsonObject - Unhandled property, Class = `%s`, Name =`%s`, inside Struct = `%s`"),
					*PropertyClass->GetName(), *Property->GetPathName(), *StructDefinition->GetPathName());
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
bool FDlgJsonWriter::UStructToJsonString(const UStruct* StructDefinition, const void* Object,
	 const DlgJsonWriterOptions& Options, FString& OutJsonString)
{
	TSharedRef<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	if (UStructToJsonObject(StructDefinition, Object, JsonObject))
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
