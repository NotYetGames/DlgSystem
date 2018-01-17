// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgJsonWriter.h"
#include "JsonUtilities.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "TextProperty.h"
#include "PropertyPortFlags.h"

DEFINE_LOG_CATEGORY(LogDlgJsonWriter);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DlgJsonWriter::DlgJsonWriter(const UStruct* StructDefinition, const void* Object)
{
	DlgJsonWriterOptions WriterOptions;
	WriterOptions.bPrettyPrint = true;
	WriterOptions.InitialIndent = 0;
	UStructToJsonString(StructDefinition, Object, WriterOptions, JsonString);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSharedPtr<FJsonValue> DlgJsonWriter::ConvertScalarUPropertyToJsonValue(UProperty* Property, const void* Value, int32 IndexInArray)
{
	check(Property);
	check(Value);

	// Get Json String for Enum definition
	auto GetJsonStringForEnum = [&Value](const UEnum* EnumDefinition, const UNumericProperty* NumericProperty) -> TSharedPtr<FJsonValue>
	{
		const FString StringValue = EnumDefinition->GetNameByIndex(NumericProperty->GetSignedIntPropertyValue(Value)).ToString();
		return MakeShareable(new FJsonValueString(StringValue));
	};

	// Add Index Metadata to JsonObject
	auto AddIndexMetadata = [Property, IndexInArray](TSharedRef<FJsonObject> JsonObject)
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
		if (NumericProperty->IsFloatingPoint())
		{
			return MakeShareable(new FJsonValueNumber(NumericProperty->GetFloatingPointPropertyValue(Value)));
		}
		else if (NumericProperty->IsInteger())
		{
			return MakeShareable(new FJsonValueNumber(NumericProperty->GetSignedIntPropertyValue(Value)));
		}

		// Invalid
		return TSharedPtr<FJsonValue>();
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
				   Warning,
				   TEXT("Got Property = `%s` of type UNameProperty but the Value it not an FName"),
				   *NameProperty->GetName())
			return TSharedPtr<FJsonValue>();
		}
		if (!NamePtr->IsValidIndexFast() || !NamePtr->IsValid())
		{
			UE_LOG(LogDlgJsonWriter, Warning, TEXT("Got Property = `%s` of type FName but it is not valid :("), *NameProperty->GetName())
			return TSharedPtr<FJsonValue>();
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
			TSharedPtr<FJsonValue> Elem = UPropertyToJsonValue(ArrayProperty->Inner, Helper.GetRawPtr(Index), Index);
			if (Elem.IsValid())
			{
				// add to the array
				Array.Push(Elem);
			}
		}
		return MakeShareable(new FJsonValueArray(Array));
	}

	// TSet
	if (const USetProperty* SetProperty = Cast<USetProperty>(Property))
	{
		TArray<TSharedPtr<FJsonValue>> Out;
		FScriptSetHelper Helper(SetProperty, Value);
		for (int32 Index = 0, Num = Helper.Num(); Index < Num; Index++)
		{
			TSharedPtr<FJsonValue> Elem = UPropertyToJsonValue(SetProperty->ElementProp, Helper.GetElementPtr(Index), Index);
			if (Elem.IsValid())
			{
				// add to the array
				Out.Push(Elem);
			}
		}

		return MakeShareable(new FJsonValueArray(Out));
	}

	// TMap
	if (const UMapProperty* MapProperty = Cast<UMapProperty>(Property))
	{
		const TSharedRef<FJsonObject> OutObject = MakeShareable(new FJsonObject());

		FScriptMapHelper Helper(MapProperty, Value);
		for (int32 Index = 0, Num = Helper.Num(); Index < Num; Index++)
		{
			const TSharedPtr<FJsonValue> KeyElement = UPropertyToJsonValue(MapProperty->KeyProp, Helper.GetKeyPtr(Index), Index);
			const TSharedPtr<FJsonValue> ValueElement = UPropertyToJsonValue(MapProperty->ValueProp, Helper.GetValuePtr(Index), Index);
			if (KeyElement.IsValid() && ValueElement.IsValid())
			{
				FString KeyString = KeyElement->AsString();

				// Try key
				if (KeyString.IsEmpty())
				{
					uint8* KeyValuePtr = Helper.GetKeyPtr(Index);
					check(KeyValuePtr);

					MapProperty->KeyProp->ExportTextItem(KeyString, KeyValuePtr, nullptr, nullptr, 0);
					if (KeyString.IsEmpty())
					{
						UE_LOG(LogDlgJsonWriter, Error, TEXT("Unable to convert key to string for property %s."), *MapProperty->GetName())
						KeyString = FString::Printf(TEXT("Unparsed Key %d"), Index);
					}
				}

				OutObject->SetField(KeyString, ValueElement);
			}
		}

		return MakeShareable(new FJsonValueObject(OutObject));
	}

	// UStruct
	if (const UStructProperty* StructProperty = Cast<UStructProperty>(Property))
	{
		UScriptStruct::ICppStructOps* TheCppStructOps = StructProperty->Struct->GetCppStructOps();
		// Intentionally exclude the JSON Object wrapper, which specifically needs to export JSON in an object representation instead of a string
		if (StructProperty->Struct != FJsonObjectWrapper::StaticStruct() && TheCppStructOps && TheCppStructOps->HasExportTextItem())
		{
			FString OutValueStr;
			TheCppStructOps->ExportTextItem(OutValueStr, Value, nullptr, nullptr, PPF_None, nullptr);
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
		return TSharedPtr<FJsonValue>();
	}

	// UObject
	if (const UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property))
	{
		// NOTE: The Value here should be a pointer to a pointer
		if (static_cast<const UObject*>(Value) == nullptr)
		{
			UE_LOG(LogDlgJsonWriter, Error, TEXT("PropertyName = `%s` Is a UObjectProperty but can't convert Value to an UObject..."),
				   *Property->GetName());
			return TSharedPtr<FJsonValue>();
		}

		// Because the UObjects are pointers, we must deference it. So instead of it being a void** we want it to be a void*
		auto* ObjectPtrPtr = static_cast<const UObject* const*>(ObjectProperty->ContainerPtrToValuePtr<void>(Value, 0));
		if (ObjectPtrPtr == nullptr || *ObjectPtrPtr == nullptr)
		{
			UE_LOG(LogDlgJsonWriter, Verbose, TEXT("PropertyName = `%s` Is a UObjectProperty but got null from ContainerPtrToValuePtr from it's StructObject"),
				   *Property->GetName());
			return TSharedPtr<FJsonValue>();
		}
		const UObject* ObjectPtr = *ObjectPtrPtr;
		const UClass* ObjectClass = ObjectProperty->PropertyClass;

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

		// Invalid
		return TSharedPtr<FJsonValue>();
	}

	// Default, convert to string
	FString ValueString;
	Property->ExportTextItem(ValueString, Value, nullptr, nullptr, PPF_None);
	return MakeShareable(new FJsonValueString(ValueString));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSharedPtr<FJsonValue> DlgJsonWriter::UPropertyToJsonValue(UProperty* Property, const void* Value, int32 IndexInArray)
{
	check(Property);
	UE_LOG(LogDlgJsonWriter, Verbose, TEXT("UPropertyToJsonValue, PropertyName = `%s`"), *Property->GetName());

	if (Value == nullptr)
	{
		const UClass* PropClass = Property->GetClass();
		UE_LOG(LogDlgJsonWriter,
			   Warning,
			   TEXT("UStructToJsonObject - Unhandled property type '%s': %s"),
			   *PropClass->GetName(), *Property->GetPathName());
		return nullptr;
	}

	// Scalar Only one property
	if (Property->ArrayDim == 1)
	{
		return ConvertScalarUPropertyToJsonValue(Property, Value, IndexInArray);
	}

	// Array
	// NOTE: we can't use here ArrayHelper, because then we might also need to use SetHelper, more code, meh
	TArray<TSharedPtr<FJsonValue>> Array;
	auto ValuePtr = static_cast<const uint8*>(Value);
	check(ValuePtr);
	for (int Index = 0; Index < Property->ArrayDim; Index++)
	{
		// ValuePtr + Index * Property->ElementSize is literally FScriptArrayHelper::GetRawPtr
		const TSharedPtr<FJsonValue> JsonValue =
				ConvertScalarUPropertyToJsonValue(Property, ValuePtr + Index * Property->ElementSize, Index);
		if (JsonValue.IsValid())
		{
			Array.Add(JsonValue);
		}
	}
	return MakeShareable(new FJsonValueArray(Array));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool DlgJsonWriter::UStructToJsonAttributes(const UStruct* StructDefinition, const void* Object,
	TMap<FString, TSharedPtr<FJsonValue>>& OutJsonAttributes)
{
	check(StructDefinition);
	check(Object);
	UE_LOG(LogDlgJsonWriter, Verbose, TEXT("UStructToJsonAttributes, StructDefinition = `%s`"), *StructDefinition->GetName());

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
		const UObject* TempObject = static_cast<const UObject*>(Object);

		// Write type, Objects because they can have inheritance
		OutJsonAttributes.Add(TEXT("__type__"), MakeShareable(new FJsonValueString(TempObject->GetClass()->GetName())));

		// Structure points to the child
		StructDefinition = TempObject->GetClass();
		check(StructDefinition);
	}

	// Iterate over all the properties of the struct
	for (TFieldIterator<UProperty> It(StructDefinition); It; ++It)
	{
		UProperty* Property = *It;
		check(Property);

		// Check to see if we should ignore this property
		if (CheckFlags != 0 && !Property->HasAnyPropertyFlags(CheckFlags))
		{
			// Property does not have the required Flags
			UE_LOG(LogDlgJsonWriter, Verbose, TEXT("PropertyName = `%s` Does not have the required CheckFlags"), *Property->GetName());
			continue;
		}
		if (CanSkipProperty(Property))
		{
			// Markes as skiped.
			UE_LOG(LogDlgJsonWriter, Verbose, TEXT("PropertyName = `%s` Marked as skiped"), *Property->GetName());
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
			UClass* PropClass = Property->GetClass();
			UE_LOG(LogDlgJsonWriter,
				   Verbose,
				   TEXT("UStructToJsonObject - Unhandled property type '%s': %s inside Struct = `%s`"),
				   *PropClass->GetName(), *Property->GetPathName(), *StructDefinition->GetName());
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
bool DlgJsonWriter::UStructToJsonString(const UStruct* StructDefinition, const void* Object,
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
