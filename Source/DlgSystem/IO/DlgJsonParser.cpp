// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgJsonParser.h"


#include "Logging/LogMacros.h"
#include "UObject/Object.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"
#include "UObject/EnumProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/PropertyPortFlags.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "Internationalization/CulturePointer.h"
#include "Internationalization/Culture.h"
#include "Misc/OutputDevice.h"
#include "Misc/FeedbackContext.h"

#include "DlgSystem/NYReflectionHelper.h"


DEFINE_LOG_CATEGORY(LogDlgJsonParser);

bool GetTextFromObject(const TSharedRef<FJsonObject>& Obj, FText& TextOut)
{
	// get the prioritized culture name list
	const FCultureRef CurrentCulture = FInternationalization::Get().GetCurrentCulture();
	const TArray<FString> CultureList = CurrentCulture->GetPrioritizedParentCultureNames();

	// try to follow the fall back chain that the engine uses
	FString TextString;
	for (const FString& CultureCode : CultureList)
	{
		if (Obj->TryGetStringField(CultureCode, TextString))
		{
			TextOut = FText::FromString(TextString);
			return true;
		}
	}

	// no luck, is this possibly an unrelated json object?
	return false;
}

FString GetStringForJsonType(const EJson Type)
{
	switch (Type)
	{
		case EJson::None:
			return TEXT("EJson::None");
		case EJson::Null:
			return TEXT("EJson::Null");
		case EJson::String:
			return TEXT("EJson::String");
		case EJson::Number:
			return TEXT("EJson::Number");
		case EJson::Boolean:
			return TEXT("EJson::Boolean");
		case EJson::Array:
			return TEXT("EJson::Array");
		case EJson::Object:
			return TEXT("EJson::Object");
		default:
			return TEXT("UNKNOWN TYPE, should never happen");
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgJsonParser::InitializeParser(const FString& FilePath)
{
	if (FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		FileName = FPaths::GetBaseFilename(FilePath, true);
		bIsValidFile = true;
	}
	else
	{
		UE_LOG(LogDlgJsonParser, Error, TEXT("Failed to load config file %s"), *FilePath);
		bIsValidFile = false;
	}

	// TODO check here if the JSON file is valid.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgJsonParser::InitializeParserFromString(const FString& Text)
{
	JsonString = Text;
	bIsValidFile = true;
	FileName = "";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgJsonParser::ReadAllProperty( const UStruct* ReferenceClass, void* TargetObject, UObject* InDefaultObjectOuter)
{
	if (!IsValidFile())
	{
		return;
	}

	// TODO use DefaultObjectOuter;
	DefaultObjectOuter = InDefaultObjectOuter;
	bIsValidFile = JsonObjectStringToUStruct(ReferenceClass, TargetObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgJsonParser::ConvertScalarJsonValueToProperty(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* ContainerPtr, void* ValuePtr)
{
	check(Property);
	if (bLogVerbose)
	{
		UE_LOG(LogDlgJsonParser, Verbose, TEXT("ConvertScalarJsonValueToProperty, Property = `%s`"), *Property->GetPathName());
	}
	if (ValuePtr == nullptr)
	{
		// Nothing else to do
		return true;
	}

	// Enum
	if (auto* EnumProperty = FNYReflectionHelper::CastProperty<FEnumProperty>(Property))
	{
		if (JsonValue->Type == EJson::String)
		{
			// see if we were passed a string for the enum
			const UEnum* Enum = EnumProperty->GetEnum();
			check(Enum);
			const FString StrValue = JsonValue->AsString();
			const int64 IntValue = Enum->GetValueByName(FName(*StrValue));
			if (IntValue == INDEX_NONE)
			{
				UE_LOG(LogDlgJsonParser,
					   Error,
					   TEXT("ConvertScalarJsonValueToProperty - Unable import enum `%s` from string value `%s` for property `%s`"),
					   *Enum->CppType, *StrValue, *Property->GetNameCPP());
				return false;
			}
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, IntValue);
		}
		else
		{
			// Numeric enum
			// AsNumber will log an error for completely inappropriate types (then give us a default)
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, static_cast<int64>(JsonValue->AsNumber()));
		}

		return true;
	}

	// Numeric, int, float, possible enum
	if (auto* NumericProperty = FNYReflectionHelper::CastProperty<FNumericProperty>(Property))
	{
		if (NumericProperty->IsEnum() && JsonValue->Type == EJson::String)
		{
			// see if we were passed a string for the enum
			const UEnum* Enum = NumericProperty->GetIntPropertyEnum();
			check(Enum); // should be assured by IsEnum()
			const FString StrValue = JsonValue->AsString();
			const int64 IntValue = Enum->GetValueByName(FName(*StrValue));
			if (IntValue == INDEX_NONE)
			{
				UE_LOG(
					LogDlgJsonParser,
					Error,
					TEXT("ConvertScalarJsonValueToProperty - Unable import enum %s from string value %s for property %s"),
					*Enum->CppType, *StrValue, *Property->GetNameCPP()
				);
				return false;
			}
			NumericProperty->SetIntPropertyValue(ValuePtr, IntValue);
		}
		else if (NumericProperty->IsInteger())
		{
			if (JsonValue->Type == EJson::String)
			{
				// parse string -> int64 ourselves so we don't lose any precision going through AsNumber (aka double)
				NumericProperty->SetIntPropertyValue(ValuePtr, FCString::Atoi64(*JsonValue->AsString()));
			}
			else
			{
				// AsNumber will log an error for completely inappropriate types (then give us a default)
				NumericProperty->SetIntPropertyValue(ValuePtr, static_cast<int64>(JsonValue->AsNumber()));
			}
		}
		else if (NumericProperty->IsFloatingPoint())
		{
			// AsNumber will log an error for completely inappropriate types (then give us a default)
			NumericProperty->SetFloatingPointPropertyValue(ValuePtr, JsonValue->AsNumber());
		}
		else
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("ConvertScalarJsonValueToProperty - Unable to set numeric property type %s for property %s"),
				*Property->GetClass()->GetName(), *Property->GetNameCPP()
			);
			return false;
		}

		return true;
	}

	// Bool
	if (auto* BoolProperty = FNYReflectionHelper::CastProperty<FBoolProperty>(Property))
	{
		// AsBool will log an error for completely inappropriate types (then give us a default)
		BoolProperty->SetPropertyValue(ValuePtr, JsonValue->AsBool());
		return true;
	}

	// FString
	if (auto* StringProperty = FNYReflectionHelper::CastProperty<FStrProperty>(Property))
	{
		// Seems unsafe: AsString will log an error for completely inappropriate types (then give us a default)
		FString String = JsonValue->AsString();
		StringProperty->SetPropertyValue(ValuePtr, String);
		return true;
	}

	// FName
	if (auto* NameProperty = FNYReflectionHelper::CastProperty<FNameProperty>(Property))
	{
		FString String;
		const FName StringFName = FName(*JsonValue->AsString());
		NameProperty->SetPropertyValue(ValuePtr, StringFName);
		return true;
	}

	// FText
	if (auto* TextProperty = FNYReflectionHelper::CastProperty<FTextProperty>(Property))
	{
		if (JsonValue->Type == EJson::String)
		{
			// assume this string is already localized, so import as invariant
			const FString String = JsonValue->AsString();
			TextProperty->SetPropertyValue(ValuePtr, FText::FromString(String));
		}
		else if (JsonValue->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> Obj = JsonValue->AsObject();
			check(Obj.IsValid()); // should not fail if Type == EJson::Object

			// import the subvalue as a culture invariant string
			FText Text;
			if (!GetTextFromObject(Obj.ToSharedRef(), Text))
			{
				UE_LOG(
					LogDlgJsonParser,
					Error,
					TEXT("ConvertScalarJsonValueToProperty - Attempted to import FText from JSON object with invalid keys for property %s"),
					*Property->GetNameCPP()
				);
				return false;
			}
			TextProperty->SetPropertyValue(ValuePtr, Text);
		}
		else
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("ConvertScalarJsonValueToProperty - Attempted to import FText from JSON that was neither string nor object for property %s"),
				*Property->GetNameCPP()
			);
			return false;
		}

		return true;
	}

	// TArray
	if (auto* ArrayProperty = FNYReflectionHelper::CastProperty<FArrayProperty>(Property))
	{
		if (JsonValue->Type == EJson::Array)
		{
			const TArray<TSharedPtr<FJsonValue>> ArrayValue = JsonValue->AsArray();
			const int32 ArrayNum = ArrayValue.Num();

			// make the output array size match
			FScriptArrayHelper Helper(ArrayProperty, ValuePtr);
			Helper.EmptyValues();
			Helper.Resize(ArrayNum);

			// set the property values
			bool bReturnStatus = true;
			for (int32 Index = 0; Index < ArrayNum; Index++)
			{
				const TSharedPtr<FJsonValue>& ArrayValueItem = ArrayValue[Index];
				if (ArrayValueItem.IsValid())
				{
					if (!JsonValueToProperty(ArrayValueItem, ArrayProperty->Inner, ContainerPtr, Helper.GetRawPtr(Index)))
					{
						bReturnStatus = false;
						UE_LOG(
							LogDlgJsonParser,
							Error,
							TEXT("ConvertScalarJsonValueToProperty - Unable to deserialize array element [%d] for property %s"),
							Index, *Property->GetNameCPP()
						);
					}
				}
			}

			return bReturnStatus;
		}

		UE_LOG(LogDlgJsonParser,
			   Error,
			   TEXT("ConvertScalarJsonValueToProperty - Attempted to import TArray from non-array JSON key for property %s"),
			   *Property->GetNameCPP());
		return false;
	}

	// Set
	if (auto* SetProperty = FNYReflectionHelper::CastProperty<FSetProperty>(Property))
	{
		if (JsonValue->Type == EJson::Array)
		{
			const TArray<TSharedPtr<FJsonValue>> ArrayValue = JsonValue->AsArray();
			const int32 ArrayNum = ArrayValue.Num();

			FScriptSetHelper Helper(SetProperty, ValuePtr);
			Helper.EmptyElements();

			// set the property values
			bool bReturnStatus = true;
			for (int32 Index = 0; Index < ArrayNum; ++Index)
			{
				const TSharedPtr<FJsonValue>& ArrayValueItem = ArrayValue[Index];
				if (ArrayValueItem.IsValid())
				{
					const int32 NewIndex = Helper.AddDefaultValue_Invalid_NeedsRehash();
					if (!JsonValueToProperty(ArrayValueItem, SetProperty->ElementProp, ContainerPtr, Helper.GetElementPtr(NewIndex)))
					{
						bReturnStatus = false;
						UE_LOG(
							LogDlgJsonParser,
							Error,
							TEXT("ConvertScalarJsonValueToProperty - Unable to deserialize set element [%d] for property %s"),
							Index,
							*Property->GetNameCPP()
						);
					}
				}
			}

			Helper.Rehash();
			return bReturnStatus;
		}

		UE_LOG(
			LogDlgJsonParser,
			Error,
			TEXT("ConvertScalarJsonValueToProperty - Attempted to import TSet from non-array (JsonValue->Type = `%s`) JSON key for property %s"),
			*GetStringForJsonType(JsonValue->Type), *Property->GetNameCPP()
		);
		return false;
	}

	// TMap
	if (auto* MapProperty = FNYReflectionHelper::CastProperty<FMapProperty>(Property))
	{
		if (JsonValue->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> ObjectValue = JsonValue->AsObject();
			FScriptMapHelper Helper(MapProperty, ValuePtr);
			Helper.EmptyValues();

			// set the property values
			bool bReturnStatus = true;
			for (const auto& Entry : ObjectValue->Values)
			{
				if (Entry.Value.IsValid())
				{
					const int32 NewIndex = Helper.AddDefaultValue_Invalid_NeedsRehash();

					// NOTE if key is a FStructProperty no need to Import the text item here as it will do that below in UStruct
					// Add key
					const TSharedPtr<FJsonValueString> KeyAsString = MakeShared<FJsonValueString>(Entry.Key);
					const bool bKeySuccess = JsonValueToProperty(KeyAsString, Helper.GetKeyProperty(), ContainerPtr, Helper.GetKeyPtr(NewIndex));

					// Add value
					const bool bValueSuccess = JsonValueToProperty(Entry.Value, Helper.GetValueProperty(), ContainerPtr, Helper.GetValuePtr(NewIndex));

					if (!bKeySuccess || !bValueSuccess)
					{
						Helper.RemoveAt(NewIndex);
						bReturnStatus = false;
						UE_LOG(
							LogDlgJsonParser,
							Error,
							TEXT("ConvertScalarJsonValueToProperty - Unable to deserialize map element [key: %s] for property %s"),
							*Entry.Key, *Property->GetNameCPP()
						);
					}
				}
			}

			Helper.Rehash();
			return bReturnStatus;
		}

		UE_LOG(LogDlgJsonParser,
			   Error,
			   TEXT("ConvertScalarJsonValueToProperty - Attempted to import TMap from non-object JSON key for property %s"),
			   *Property->GetNameCPP());
		return false;
	}

	// UStruct
	if (auto* StructProperty = FNYReflectionHelper::CastProperty<FStructProperty>(Property))
	{
		static const FName NAME_JSON_DateTime(TEXT("DateTime"));
		static const FName NAME_JSON_Color(TEXT("Color"));
		static const FName NAME_JSON_LinearColor(TEXT("LinearColor"));

		// Default struct export
		if (JsonValue->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> Obj = JsonValue->AsObject();
			check(Obj.IsValid()); // should not fail if Type == EJson::Object
			if (!JsonObjectToUStruct(Obj.ToSharedRef(), StructProperty->Struct, ValuePtr))
			{
				UE_LOG(
					LogDlgJsonParser,
					Error,
					TEXT("ConvertScalarJsonValueToProperty - JsonObjectToUStruct failed for property %s"),
					*Property->GetNameCPP()
				);
				return false;
			}
		}

		// Handle some structs that are exported to string in a special way
		else if (JsonValue->Type == EJson::String && StructProperty->Struct->GetFName() == NAME_JSON_LinearColor)
		{
			const FString ColorString = JsonValue->AsString();
			const FColor IntermediateColor = FColor::FromHex(ColorString);
			FLinearColor& ColorOut = *static_cast<FLinearColor*>(ValuePtr);
			ColorOut = IntermediateColor;
		}
		else if (JsonValue->Type == EJson::String && StructProperty->Struct->GetFName() == NAME_JSON_Color)
		{
			const FString ColorString = JsonValue->AsString();
			FColor& ColorOut = *static_cast<FColor*>(ValuePtr);
			ColorOut = FColor::FromHex(ColorString);
		}
		else if (JsonValue->Type == EJson::String && StructProperty->Struct->GetFName() == NAME_JSON_DateTime)
		{
			const FString DateString = JsonValue->AsString();
			FDateTime& DateTimeOut = *static_cast<FDateTime*>(ValuePtr);
			if (DateString == TEXT("min"))
			{
				// min representable value for our date struct. Actual date may vary by platform (this is used for sorting)
				DateTimeOut = FDateTime::MinValue();
			}
			else if (DateString == TEXT("max"))
			{
				// max representable value for our date struct. Actual date may vary by platform (this is used for sorting)
				DateTimeOut = FDateTime::MaxValue();
			}
			else if (DateString == TEXT("now"))
			{
				// this value's not really meaningful from json serialization (since we don't know timezone) but handle it anyway since we're handling the other keywords
				DateTimeOut = FDateTime::UtcNow();
			}
			else if (FDateTime::ParseIso8601(*DateString, DateTimeOut))
			{
				// ok
			}
			else if (FDateTime::Parse(DateString, DateTimeOut))
			{
				// ok
			}
			else
			{
				UE_LOG(
					LogDlgJsonParser,
					Warning,
					TEXT("ConvertScalarJsonValueToProperty - Unable to import FDateTime for property %s"),
					*Property->GetNameCPP()
				);
				return false;
			}
		}
		else if (JsonValue->Type == EJson::String &&
				 StructProperty->Struct->GetCppStructOps() &&
				 StructProperty->Struct->GetCppStructOps()->HasImportTextItem())
		{
			// Import as simple native string
			UScriptStruct::ICppStructOps* TheCppStructOps = StructProperty->Struct->GetCppStructOps();

			const FString ImportTextString = JsonValue->AsString();
			const TCHAR* ImportTextPtr = *ImportTextString;
			if (!TheCppStructOps->ImportTextItem(ImportTextPtr, ValuePtr, PPF_None, nullptr, static_cast<FOutputDevice*>(GWarn)))
			{
				// Fall back to trying the tagged property approach if custom ImportTextItem couldn't get it done
#if NY_ENGINE_VERSION >= 501
				Property->ImportText_Direct(ImportTextPtr, ValuePtr, nullptr, PPF_None);
#else
				Property->ImportText(ImportTextPtr, ValuePtr, PPF_None, nullptr);
#endif

			}
		}
		else if (JsonValue->Type == EJson::String)
		{
			// Import as simple string
			// UTextBuffer* ImportErrors = NewObject<UTextBuffer>();
			const FString ImportTextString = JsonValue->AsString();
			const TCHAR* ImportTextPtr = *ImportTextString;
#if NY_ENGINE_VERSION >= 501
			Property->ImportText_Direct(ImportTextPtr, ValuePtr, nullptr, PPF_None);
#else
			Property->ImportText(ImportTextPtr, ValuePtr, PPF_None, nullptr);
#endif
		}
		else
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("ConvertScalarJsonValueToProperty - Attempted to import UStruct from non-object JSON key for property %s"),
				*Property->GetNameCPP()
			);
			return false;
		}

		return true;
	}

	// UObject
	if (auto* ObjectProperty = FNYReflectionHelper::CastProperty<FObjectProperty>(Property))
	{
		// NOTE: The Value here should be a pointer to a pointer
		// Because the UObjects are pointers, we must deference it. So instead of it being a void** we want it to be a void*
		auto* ObjectPtrPtr = static_cast<UObject**>(ObjectProperty->ContainerPtrToValuePtr<void>(ValuePtr, 0));
		if (ObjectPtrPtr == nullptr)
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("PropertyName = `%s` Is a FObjectProperty but can't get non null ContainerPtrToValuePtr from it's StructObject"),
				*Property->GetNameCPP()
			);
			return false;
		}

		// NOTE: We must check one level up to check if it is a nullptr or not
		// Reset first, if non nullptr
		const UObject* ContainerObjectPtr = ObjectProperty->GetObjectPropertyValue_InContainer(ContainerPtr);
		if (ContainerObjectPtr != nullptr)
		{
			*ObjectPtrPtr = nullptr;
		}

		// Nothing else to do
		if (JsonValue->IsNull())
		{
			return true;
		}
		const UClass* ObjectClass = ObjectProperty->PropertyClass;

		// Special case, load by reference, See CanSaveAsReference
		// Handle some objects that are exported to string in a special way. Similar to the UStruct above.
		if (JsonValue->Type == EJson::String)
		{
			const FString Path = JsonValue->AsString();
			if (!Path.TrimStartAndEnd().IsEmpty()) // null reference?
			{
				*ObjectPtrPtr = StaticLoadObject(UObject::StaticClass(), DefaultObjectOuter, *Path);
			}
			return true;
		}

		// Load the Normal JSON object
		// Must have the type inside the Json Object
		check(JsonValue->Type == EJson::Object);
		const TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
		check(JsonObject.IsValid()); // should not fail if Type == EJson::Object

		const FString SpecialKeyType = TEXT("__type__");
		if (!JsonObject->HasField(SpecialKeyType))
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("ConvertScalarJsonValueToProperty - PropertyName = `%s` JSON does not have the __type__ special property."),
				*Property->GetNameCPP()
			);
			return false;
		}

		//  Create the new Object
		FString JsonObjectType;
		const bool bExists = JsonObject->TryGetStringField(SpecialKeyType, JsonObjectType);
		if (!bExists)
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("ConvertScalarJsonValueToProperty - Trying to load by string reference. Can't find key = `%s` for FObjectProperty = `%s`. Ignored"),
				*SpecialKeyType, *Property->GetNameCPP()
			);
			return false;
		}

		const UClass* ChildClass = GetChildClassFromName(ObjectClass, JsonObjectType);
		if (ChildClass == nullptr)
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("ConvertScalarJsonValueToProperty - Trying to load by string reference. Could not find class `%s` for FObjectProperty = `%s`. Ignored."),
				*JsonObjectType, *Property->GetNameCPP()
			);
			return false;
		}
		*ObjectPtrPtr = CreateNewUObject(ChildClass, DefaultObjectOuter);

		// Something is wrong
		if (*ObjectPtrPtr == nullptr || !(*ObjectPtrPtr)->IsValidLowLevelFast())
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("JsonValueToProperty - PropertyName = `%s` Is a FObjectProperty but could not build any valid UObject"),
				*Property->GetNameCPP()
			);
			return false;
		}

		// Write the json object
		if (!JsonObjectToUStruct(JsonObject.ToSharedRef(), ObjectClass, *ObjectPtrPtr))
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("JsonValueToProperty - JsonObjectToUStruct failed for property %s"),
				*Property->GetNameCPP()
			);
			return false;
		}

		return true;
	}

	// Default to expect a string for everything else
	check(JsonValue->Type != EJson::Object);
	const FString Buffer = JsonValue->AsString();

#if NY_ENGINE_VERSION >= 501
	if (Property->ImportText_Direct(*Buffer, ValuePtr, nullptr, PPF_None) == nullptr)
#else
	if (Property->ImportText(*Buffer, ValuePtr, PPF_None, nullptr) == nullptr)
#endif
	{
		UE_LOG(
			LogDlgJsonParser,
			Error,
			TEXT("JsonValueToProperty - Unable import property type %s from string value for property %s"),
			*Property->GetClass()->GetName(), *Property->GetNameCPP()
		);
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgJsonParser::JsonValueToProperty(const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* ContainerPtr, void* ValuePtr)
{
	check(Property);
	if (bLogVerbose)
	{
		UE_LOG(LogDlgJsonParser, Verbose, TEXT("JsonValueToProperty, Property = `%s`"), *Property->GetPathName());
	}
	if (!JsonValue.IsValid())
	{
		UE_LOG(LogDlgJsonParser, Error, TEXT("JsonValueToProperty - Invalid value JSON key"));
		return false;
	}

	const bool bArrayProperty = Property->IsA<FArrayProperty>();
	const bool bSetProperty = Property->IsA<FSetProperty>();
	const bool bJsonArray = JsonValue->Type == EJson::Array;

	// Scalar only one property
	if (!bJsonArray)
	{
		if (bArrayProperty)
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("JsonValueToProperty - Attempted to import TArray from non-array JSON type = `%s`"),
				*GetStringForJsonType(JsonValue->Type)
			);
			return false;
		}
		if (bSetProperty)
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("JsonValueToProperty - Attempted to import TSet from non-array JSON type = `%s`"),
				*GetStringForJsonType(JsonValue->Type)
			);
			return false;
		}

		if (Property->ArrayDim != 1)
		{
			UE_LOG(LogDlgJsonParser, Warning, TEXT("[Property->ArrayDim != 1] Ignoring excess properties when deserializing %s"), *Property->GetNameCPP());
		}

		return ConvertScalarJsonValueToProperty(JsonValue, Property, ContainerPtr, ValuePtr);
	}

	// In practice, the ArrayDim == 1 check ought to be redundant, since nested arrays of UPropertys are not supported
	if ((bArrayProperty || bSetProperty) && Property->ArrayDim == 1)
	{
		// Read into TArray/TSet
		return ConvertScalarJsonValueToProperty(JsonValue, Property, ContainerPtr, ValuePtr);
	}

	// Array
	// We're deserializing a JSON array
	const auto& ArrayValue = JsonValue->AsArray();
	if (Property->ArrayDim < ArrayValue.Num())
	{
		UE_LOG(LogDlgJsonParser, Warning, TEXT("[Property->ArrayDim < ArrayValue.Num()] Ignoring excess properties when deserializing %s"), *Property->GetNameCPP());
	}

	// Read into Array/Set
	const int32 ItemsToRead = FMath::Clamp(ArrayValue.Num(), 0, Property->ArrayDim);
	auto* ValueIntPtr = static_cast<uint8*>(ValuePtr);
	bool bReturnStatus = true;
	for (int32 Index = 0; Index < ItemsToRead; ++Index)
	{
		// ValuePtr + Index * Property->ElementSize is literally FScriptArrayHelper::GetRawPtr
		bReturnStatus &= ConvertScalarJsonValueToProperty(ArrayValue[Index], Property, ContainerPtr, ValueIntPtr + Index * Property->ElementSize);
	}
	return bReturnStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgJsonParser::JsonAttributesToUStruct(const TMap<FString, TSharedPtr<FJsonValue>>& JsonAttributes,
											const UStruct* StructDefinition, void* ContainerPtr)
{
	check(StructDefinition);
	check(ContainerPtr);
	if (bLogVerbose)
	{
		UE_LOG(LogDlgJsonParser, Verbose, TEXT("JsonAttributesToUStruct, StructDefinition = `%s`"), *StructDefinition->GetPathName());
	}

	// Json Wrapper, already have an Object
	if (StructDefinition == FJsonObjectWrapper::StaticStruct())
	{
		// Just copy it into the object
		FJsonObjectWrapper* ProxyObject = (FJsonObjectWrapper *)ContainerPtr;
		ProxyObject->JsonObject = MakeShared<FJsonObject>();
		ProxyObject->JsonObject->Values = JsonAttributes;
		return true;
	}

	// Handle UObject inheritance (children of class)
	if (StructDefinition->IsA<UClass>())
	{
		// Structure points to the child
		const UObject* UnrealObject = static_cast<const UObject*>(ContainerPtr);
		if (!UnrealObject->IsValidLowLevelFast())
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("JsonAttributesToUStruct: StructDefinition = `%s` is a UClass and expected ContainerPtr to be an UObject. Memory corruption?"),
				*StructDefinition->GetPathName()
			);
			return false;
		}
		StructDefinition = UnrealObject->GetClass();
	}
	if (!StructDefinition->IsValidLowLevelFast())
	{
		UE_LOG(
			LogDlgJsonParser,
			Error,
			TEXT("JsonAttributesToUStruct: StructDefinition = `%s` is a UClass and expected ContainerPtr.Class to be valid. Memory corruption?"),
			*StructDefinition->GetPathName()
		);
		return false;
	}

	// iterate over the struct properties
	for (TFieldIterator<FProperty> PropIt(StructDefinition); PropIt; ++PropIt)
	{
		auto* Property = *PropIt;
		if (!ensure(Property))
			continue;

		const FString PropertyName = Property->GetName();

		// Check to see if we should ignore this property
		if (CheckFlags != 0 && !Property->HasAnyPropertyFlags(CheckFlags))
		{
			continue;
		}
		// TODO skip property

		// Find a JSON value matching this property name
		TSharedPtr<FJsonValue> JsonValue;
		for (auto& Elem : JsonAttributes)
		{
			// use case insensitive search since FName may change case strangely on us
			// TODO does this break on struct/classes with properties of similar name?
			if (PropertyName.Equals(Elem.Key, ESearchCase::IgnoreCase))
			{
				JsonValue = Elem.Value;
				break;
			}
		}
		if (!JsonValue.IsValid())
		{
			// we allow values to not be found since this mirrors the typical UObject mantra that all the fields are optional when deserializing
			continue;
		}

		void* ValuePtr = nullptr;
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

		// Convert the JsonValue to the Property
		if (!JsonValueToProperty(JsonValue, Property, ContainerPtr, ValuePtr))
		{
			UE_LOG(
				LogDlgJsonParser,
				Error,
				TEXT("JsonObjectToUStruct - Unable to parse %s.%s from JSON"),
				*StructDefinition->GetName(), *PropertyName
			);
			continue;
		}
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgJsonParser::JsonObjectStringToUStruct(const UStruct* StructDefinition, void* ContainerPtr)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogDlgJsonParser, Error, TEXT("JsonObjectStringToUStruct - Unable to parse json=[%s]"), *JsonString);
		return false;
	}
	if (!JsonObjectToUStruct(JsonObject.ToSharedRef(), StructDefinition, ContainerPtr))
	{
		UE_LOG(LogDlgJsonParser, Error, TEXT("JsonObjectStringToUStruct - Unable to deserialize. json=[%s]"), *JsonString);
		return false;
	}
	return true;
}
