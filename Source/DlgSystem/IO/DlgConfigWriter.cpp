// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgConfigWriter.h"

#include "Misc/FileHelper.h"
#include "UObject/EnumProperty.h"

#include "DlgSystem/DlgHelper.h"
#include "DlgSystem/NYReflectionHelper.h"

DEFINE_LOG_CATEGORY(LogDlgConfigWriter);

const TCHAR* FDlgConfigWriter::EOL_LF = TEXT("\n");
const TCHAR* FDlgConfigWriter::EOL_CRLF = TEXT("\r\n");
const TCHAR* FDlgConfigWriter::EOL = EOL_LF;
const FString FDlgConfigWriter::EOL_String{EOL};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FDlgConfigWriter::FDlgConfigWriter(const FString InComplexNamePrefix,
								   bool bInDontWriteEmptyContainer) :
	ComplexNamePrefix(InComplexNamePrefix),
	bDontWriteEmptyContainer(bInDontWriteEmptyContainer)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgConfigWriter::Write(const UStruct* const StructDefinition, const void* const Object)
{
	TopLevelObjectPtr = Object;
	WriteComplexMembersToString(StructDefinition, Object, "", EOL, ConfigText);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgConfigWriter::WriteComplexMembersToString(const UStruct* StructDefinition,
												   const void* Object,
												   const FString& PreString,
												   const FString& PostString,
												   FString& Target)
{
	if (StructDefinition == nullptr)
	{
		return;
	}

	// order
	TArray<const FProperty*> Primitives;
	TArray<const FProperty*> PrimitiveContainers;
	TArray<const FProperty*> ComplexElements;
	TArray<const FProperty*> ComplexContainers;

	constexpr int32 PropCategoryNum = 4;
	TArray<const FProperty*>* PropCategory[PropCategoryNum] = { &Primitives, &PrimitiveContainers, &ComplexElements, &ComplexContainers };

	// Handle UObject inheritance (children of class)
	if (StructDefinition->IsA<UClass>())
	{
		const UObject* UnrealObject = static_cast<const UObject*>(Object);
		if (!UnrealObject->IsValidLowLevelFast())
		{
			return;
		}

		StructDefinition = UnrealObject->GetClass();
	}
	if (!StructDefinition->IsValidLowLevelFast())
	{
		return;
	}

	// Populate categories
	for (TFieldIterator<FProperty> It(StructDefinition); It; ++It)
	{
		const auto* Property = *It;
		if (IsPrimitive(Property))
		{
			Primitives.Add(Property);
		}
		else if (IsContainer(Property))
		{
			if (IsPrimitiveContainer(Property))
			{
				PrimitiveContainers.Add(Property);
			}
			else
			{
				ComplexContainers.Add(Property);
			}
		}
		else
		{
			ComplexElements.Add(Property);
		}
	}

	constexpr bool bContainerElement = false;
	for (int32 i = 0; i < PropCategoryNum; ++i)
	{
		for (const auto* Prop : *PropCategory[i])
		{
			WritePropertyToString(Prop, Object, bContainerElement, PreString, PostString, false, Target);
		}
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::WritePropertyToString(const FProperty* Property,
											 const void* Object,
											 bool bContainerElement,
											 const FString& PreString,
											 const FString& PostString,
											 bool bPointerAsRef,
											 FString& Target)
{
	if (CanSkipProperty(Property))
	{
		return true;
	}

	// Primitive: bool, int, float, FString, FName
	if (WritePrimitiveElementToString(Property, Object, bContainerElement, PreString, PostString, Target))
	{
		return true;
	}

	// Primitive Array: Array[int], Array[bool]
	if (WritePrimitiveArrayToString(Property, Object, PreString, PostString, Target))
	{
		return true;
	}

	// Complex element: UStruct, UObject
	if (WriteComplexElementToString(Property, Object, bContainerElement, PreString, PostString, bPointerAsRef, Target))
	{
		return true;
	}

	// Complex Array: Array[UStruct], Array[UObject]
	if (WriteComplexArrayToString(Property, Object, PreString, PostString, Target))
	{
		return true;
	}

	// Map
	if (WriteMapToString(Property, Object, PreString, PostString, Target))
	{
		return true;
	}

	// Set
	if (WriteSetToString(Property, Object, PreString, PostString, Target))
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::WritePrimitiveElementToString(const FProperty* Property,
													 const void* Object,
													 bool bInContainer,
													 const FString& PreS,
													 const FString& PostS,
													 FString& Target)
{
	// Try every possible primitive type
	if (WritePrimitiveElementToStringTemplated<FBoolProperty, bool>(Property, Object, bInContainer, BoolToString, PreS, PostS, Target))
	{
		return true;
	}
	if (WritePrimitiveElementToStringTemplated<FIntProperty, int32>(Property, Object, bInContainer, IntToString, PreS, PostS, Target))
	{
		return true;
	}
	if (WritePrimitiveElementToStringTemplated<FInt64Property, int64>(Property, Object, bInContainer, IntToString, PreS, PostS, Target))
	{
		return true;
	}
	if (WritePrimitiveElementToStringTemplated<FFloatProperty, float>(Property, Object, bInContainer, FloatToString, PreS, PostS, Target))
	{
		return true;
	}
	if (WritePrimitiveElementToStringTemplated<FStrProperty, FString>(Property, Object, bInContainer, StringToString, PreS, PostS, Target))
	{
		return true;
	}
	if (WritePrimitiveElementToStringTemplated<FNameProperty, FName>(Property, Object, bInContainer, NameToString, PreS, PostS, Target))
	{
		return true;
	}
	if (WritePrimitiveElementToStringTemplated<FTextProperty, FText>(Property, Object, bInContainer, TextToString, PreS, PostS, Target))
	{
		return true;
	}

	// TODO: enum in container - why isn't it implemented in the reader?
	if (!bInContainer)
	{
		const auto* EnumProp = FNYReflectionHelper::CastProperty<FEnumProperty>(Property);
		if (EnumProp != nullptr)
		{
			const void* Value = EnumProp->ContainerPtrToValuePtr<uint8>(Object);
			const FName EnumName = EnumProp->GetEnum()->GetNameByIndex(EnumProp->GetUnderlyingProperty()->GetSignedIntPropertyValue(Value));
			Target += PreS + Property->GetName() + " " + NameToString(EnumName) + PostS;
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::WritePrimitiveArrayToString(const FProperty* Property,
												   const void* Object,
												   const FString& PreString,
												   const FString& PostString,
												   FString& Target)
{
	const auto* ArrayProp = FNYReflectionHelper::CastProperty<FArrayProperty>(Property);
	if (ArrayProp == nullptr)
	{
		return false;
	}

	// Try every possible primitive array type
	if (WritePrimitiveArrayToStringTemplated<FBoolProperty, bool>(ArrayProp, Object, BoolToString, PreString, PostString, Target))
	{
		return true;
	}
	if (WritePrimitiveArrayToStringTemplated<FIntProperty, int32>(ArrayProp, Object, IntToString, PreString, PostString, Target))
	{
		return true;
	}
	if (WritePrimitiveArrayToStringTemplated<FInt64Property, int64>(ArrayProp, Object, IntToString, PreString, PostString, Target))
	{
		return true;
	}
	if (WritePrimitiveArrayToStringTemplated<FFloatProperty, float>(ArrayProp, Object, FloatToString, PreString, PostString, Target))
	{
		return true;
	}
	if (WritePrimitiveArrayToStringTemplated<FStrProperty, FString>(ArrayProp, Object, StringToString, PreString, PostString, Target))
	{
		return true;
	}
	if (WritePrimitiveArrayToStringTemplated<FNameProperty, FName>(ArrayProp, Object, NameToString, PreString, PostString, Target))
	{
		return true;
	}
	if (WritePrimitiveArrayToStringTemplated<FTextProperty, FText>(ArrayProp, Object, TextToString, PreString, PostString, Target))
	{
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::WriteComplexElementToString(const FProperty* Property,
												   const void* Object,
												   bool bContainerElement,
												   const FString& PreString,
												   const FString& PostString,
												   bool bPointerAsRef,
												   FString& Target)
{
	if (Property == nullptr)
	{
		return false;
	}

	// UStruct
	if (const auto* StructProperty = FNYReflectionHelper::CastProperty<FStructProperty>(Property))
	{
		const void* StructObject = StructProperty->ContainerPtrToValuePtr<void>(Object, 0);
		if (StructObject == nullptr)
		{
			return true;
		}

		WriteComplexToString(StructProperty->Struct,
							 Property,
							 StructObject,
							 PreString,
							 PostString,
							 bContainerElement,
							 false,
							 Target);
		return true;
	}

	// UObject
	if (const auto* ObjectProperty = FNYReflectionHelper::CastProperty<FObjectProperty>(Property))
	{
		UObject** ObjPtrPtr = ((UObject**)ObjectProperty->ContainerPtrToValuePtr<void>(Object, 0));
		const FString Path = *ObjPtrPtr != nullptr ? (*ObjPtrPtr)->GetPathName() : "";
		auto WritePathName = [&]()
		{
			if (bContainerElement)
			{
				Target += PreString + "\"" + Path + "\"" + PostString;
			}
			else
			{
				Target += PreString + Property->GetName() + " \"" + Path + "\"" + PostString;
			}
		};

		if (CanSaveAsReference(ObjectProperty, *ObjPtrPtr) || bPointerAsRef)
		{
			WritePathName();
		}
		else
		{
			// Write nullptr as empty string
			if (*ObjPtrPtr == nullptr || ObjectProperty->PropertyClass == nullptr)
			{
				WritePathName();
				return true;
			}

			WriteComplexToString(ObjectProperty->PropertyClass,
								 Property,
								 *ObjPtrPtr,
								 PreString,
								 PostString,
								 bContainerElement,
								 true,
								 Target);
		}
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FDlgConfigWriter::WriteComplexToString(const UStruct* StructDefinition,
											const FProperty* Property,
											const void* Object,
											const FString& PreString,
											const FString& PostString,
											bool bContainerElement,
											bool bWriteType,
											FString& Target)
{
	if (CanSkipProperty(Property) || StructDefinition == nullptr)
	{
		return;
	}

	const UObject* UnrealObject = static_cast<const UObject*>(Object);
	if (bWriteType && !UnrealObject->IsValidLowLevelFast())
	{
		return;
	}

	const bool bLinePerMember = WouldWriteNonPrimitive(StructDefinition, Object);

	// WARNING: bWriteType implicates objectproperty, if that changes this code (cause of the object cast) should be updated accordingly
	const FString TypeString = bWriteType ? GetNameWithoutPrefix(Property, UnrealObject) + " ": "";
	if (bContainerElement)
	{
		if (TypeString.Len() > 0)
		{
			Target += PreString + TypeString + (bLinePerMember ? EOL + PreString + "{" + EOL : "{ ");
		}
		else
		{
			Target += PreString + (bLinePerMember ? "{" + EOL_String : "{ ");
		}
	}
	else
	{
		Target += PreString + TypeString + Property->GetName() + (bLinePerMember ? EOL + PreString + "{" + EOL : " { ");
	}

	// Write the properties of the Struct/Object
	WriteComplexMembersToString(StructDefinition, Object, (bLinePerMember ? PreString + "\t" : " "), (bLinePerMember ? EOL_String : ""), Target);

	if (bLinePerMember)
	{
		Target += PreString + "}" + PostString;
	}
	else
	{
		Target += " }" + PostString;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::WriteComplexArrayToString(const FProperty* Property,
												 const void* Object,
												 const FString& PreString,
												 const FString& PostString,
												 FString& Target)
{
	const auto* ArrayProp = FNYReflectionHelper::CastProperty<FArrayProperty>(Property);
	if (ArrayProp == nullptr)
	{
		return false;
	}
	if (FNYReflectionHelper::CastProperty<FStructProperty>(ArrayProp->Inner) == nullptr
		&& FNYReflectionHelper::CastProperty<FObjectProperty>(ArrayProp->Inner) == nullptr)
	{
		return false;
	}

	// Empty Array
	const FDlgConstScriptArrayHelper Helper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<uint8>(Object));
	if (Helper.Num() == 0 && bDontWriteEmptyContainer)
	{
		return true;
	}

	const bool bWriteIndex = CanWriteIndex(Property);
	FString TypeText = "";
	auto* ObjProp = FNYReflectionHelper::CastProperty<FObjectProperty>(ArrayProp->Inner);
	if (ObjProp != nullptr && ObjProp->PropertyClass != nullptr)
	{
		TypeText = GetStringWithoutPrefix(ObjProp->PropertyClass->GetName()) + " ";
	}

	if (Helper.Num() == 1 && !WouldWriteNonPrimitive(GetComplexType(ArrayProp->Inner), Helper.GetConstRawPtr(0)))
	{
		Target += PreString + TypeText + ArrayProp->Inner->GetName() + " {";
		for (int32 i = 0; i < Helper.Num(); ++i)
		{
			WriteComplexElementToString(ArrayProp->Inner, Helper.GetConstRawPtr(i), true, " ", "", CanSaveAsReference(ArrayProp, nullptr), Target);
		}
		Target += " }" + PostString;
	}
	else
	{
		Target += PreString + TypeText + ArrayProp->Inner->GetName() + EOL;
		Target += PreString + "{" + EOL;
		for (int32 i = 0; i < Helper.Num(); ++i)
		{
			if (bWriteIndex)
			{
				Target += PreString + "\t// " + FString::FromInt(i) + EOL;
			}
			WriteComplexElementToString(ArrayProp->Inner, Helper.GetConstRawPtr(i), true, PreString + "\t", EOL, CanSaveAsReference(ArrayProp, nullptr), Target);
		}
		Target += PreString + "}" + EOL;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::WriteMapToString(const FProperty* Property,
										const void* Object,
										const FString& PreString,
										const FString& PostString,
										FString& Target)
{
	const auto* MapProp = FNYReflectionHelper::CastProperty<FMapProperty>(Property);
	if (MapProp == nullptr)
	{
		return false;
	}

	// Empty map
	const FScriptMapHelper Helper(MapProp, MapProp->ContainerPtrToValuePtr<uint8>(Object));
	if (Helper.Num() == 0 && bDontWriteEmptyContainer)
	{
		return true;
	}

	// NOTE: Because we access the value with GetPropertyValue_InContainer, we can't use GetValuePtr, instead use GetPairPtr
	if (IsPrimitive(MapProp->KeyProp) && IsPrimitive(MapProp->ValueProp))
	{
		// Both Key and Value are primitives
		Target += PreString + MapProp->GetName() + " { ";

		// GetMaxIndex() instead of Num() - the container is not contiguous
		// elements are in [0, GetMaxIndex[, some of them are invalid (Num() returns with the valid element num)
		for (int32 i = 0; i < Helper.GetMaxIndex(); ++i)
		{
			if (!Helper.IsValidIndex(i))
			{
				continue;
			}

			WritePrimitiveElementToString(MapProp->KeyProp, Helper.GetPairPtr(i), true, "", " ", Target);
			WritePrimitiveElementToString(MapProp->ValueProp, Helper.GetPairPtr(i), true, "", " ", Target);
		}
		Target += "}" + PostString;
	}
	else
	{
		// Either Key or Value is not a primitive
		Target += PreString + MapProp->GetName() + EOL;
		Target += PreString + "{" + EOL;

		// GetMaxIndex() instead of Num() - the container is not contiguous
		// elements are in [0, GetMaxIndex[, some of them are invalid (Num() returns with the valid element num)
		for (int32 i = 0; i < Helper.GetMaxIndex(); ++i)
		{
			if (!Helper.IsValidIndex(i))
			{
				continue;
			}

			WritePropertyToString(MapProp->KeyProp, Helper.GetPairPtr(i), true, PreString + "\t", EOL, CanSaveAsReference(MapProp, nullptr), Target);
			WritePropertyToString(MapProp->ValueProp, Helper.GetPairPtr(i), true, PreString + "\t", EOL, CanSaveAsReference(MapProp, nullptr), Target);
		}
		Target += PreString + "}" + EOL;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::WriteSetToString(const FProperty* Property,
										const void* Object,
										const FString& PreString,
										const FString& PostString,
										FString& Target)
{
	const auto* SetProp = FNYReflectionHelper::CastProperty<FSetProperty>(Property);
	if (SetProp == nullptr)
	{
		return false;
	}

	// Empty set
	const FScriptSetHelper Helper(SetProp, SetProp->ContainerPtrToValuePtr<uint8>(Object));
	if (Helper.Num() == 0 && bDontWriteEmptyContainer)
	{
		return true;
	}

	// Only write primitive set elements
	if (IsPrimitive(SetProp->ElementProp))
	{
		const bool bLinePerItem = CanWriteOneLinePerItem(SetProp);

		// Add space indentation
		FString SubPreString = PreString;
		for (int32 i = 0; i < SetProp->GetName().Len() + 3; ++i)
		{
			SubPreString += " ";
		}

		// SetName {
		Target += PreString + SetProp->GetName() + " {";
		if (!bLinePerItem)
		{
			// Add space because there is no new line
			Target += " ";
		}

		// Set content
		// GetMaxIndex() instead of Num() - the container is not contiguous, elements are in [0, GetMaxIndex[, some of them is invalid (Num() returns with the valid element num)
		for (int32 i = 0; i < Helper.GetMaxIndex(); ++i)
		{
			if (!Helper.IsValidIndex(i))
			{
				continue;
			}

			if (bLinePerItem)
			{
				WritePrimitiveElementToString(SetProp->ElementProp, Helper.GetElementPtr(i), true, EOL + SubPreString, "", Target);
			}
			else
			{
				WritePrimitiveElementToString(SetProp->ElementProp, Helper.GetElementPtr(i), true, "", " ", Target);
			}
		}

		// }
		Target += (bLinePerItem ? " " : "") + FString("}") + PostString;
	}
	else
	{
		UE_LOG(LogDlgConfigWriter, Warning, TEXT("Set not exported: unsuported set member (%s)"), *SetProp->ElementProp->GetName());
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::IsPrimitive(const FProperty* Property)
{
	return FNYReflectionHelper::CastProperty<FBoolProperty>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FIntProperty>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FInt64Property>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FFloatProperty>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FStrProperty>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FNameProperty>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FTextProperty>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FEnumProperty>(Property) != nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::IsContainer(const FProperty* Property)
{
	return FNYReflectionHelper::CastProperty<FArrayProperty>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FMapProperty>(Property) != nullptr ||
		   FNYReflectionHelper::CastProperty<FSetProperty>(Property) != nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::IsPrimitiveContainer(const FProperty* Property)
{
	// Array
	if (FNYReflectionHelper::CastProperty<FArrayProperty>(Property) != nullptr
		&& IsPrimitive(FNYReflectionHelper::CastProperty<FArrayProperty>(Property)->Inner))
	{
		return true;
	}

	// Map
	if (FNYReflectionHelper::CastProperty<FMapProperty>(Property) != nullptr &&
		IsPrimitive(FNYReflectionHelper::CastProperty<FMapProperty>(Property)->KeyProp) &&
		IsPrimitive(FNYReflectionHelper::CastProperty<FMapProperty>(Property)->ValueProp))
	{
		return true;
	}

	// Set
	if (FNYReflectionHelper::CastProperty<FSetProperty>(Property) != nullptr
		&& IsPrimitive(FNYReflectionHelper::CastProperty<FSetProperty>(Property)->ElementProp))
	{
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const UStruct* FDlgConfigWriter::GetComplexType(const FProperty* Property)
{
	if (const auto* StructProperty = FNYReflectionHelper::CastProperty<FStructProperty>(Property))
	{
		return StructProperty->Struct;
	}

	if (const auto* ObjectProperty = FNYReflectionHelper::CastProperty<FObjectProperty>(Property))
	{
		return ObjectProperty->PropertyClass;
	}

	return nullptr;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool FDlgConfigWriter::WouldWriteNonPrimitive(const UStruct* StructDefinition, const void* Owner)
{
	if (StructDefinition == nullptr || Owner == nullptr)
	{
		return false;
	}

	for (TFieldIterator<const FProperty> It(StructDefinition); It; ++It)
	{
		const auto* Property = *It;

		// Ignore primitives
		if (IsPrimitive(Property))
		{
			continue;
		}

		if (IsContainer(Property))
		{
			// Map
			if (const auto* MapProperty = FNYReflectionHelper::CastProperty<FMapProperty>(Property))
			{
				const FScriptMapHelper Helper(MapProperty, Property->ContainerPtrToValuePtr<uint8>(Owner));
				if (Helper.Num() > 0)
				{
					return true;
				}
			}

			// Array
			if (const auto* ArrayProperty = FNYReflectionHelper::CastProperty<FArrayProperty>(Property))
			{
				const FScriptArrayHelper Helper(ArrayProperty, Property->ContainerPtrToValuePtr<uint8>(Owner));
				if (Helper.Num() > 0)
				{
					return true;
				}
			}

			// Set
			if (const auto* SetProperty = FNYReflectionHelper::CastProperty<FSetProperty>(Property))
			{
				const FScriptSetHelper Helper(SetProperty, Property->ContainerPtrToValuePtr<uint8>(Owner));
				if (Helper.Num() > 0)
				{
					return true;
				}
			}
		}
		else
		{
			if (FNYReflectionHelper::CastProperty<FStructProperty>(Property) != nullptr)
			{
				return true;
			}

			if (const auto* ObjectProperty = FNYReflectionHelper::CastProperty<FObjectProperty>(Property))
			{
				UObject** ObjPtrPtr = ((UObject**)ObjectProperty->ContainerPtrToValuePtr<void>(Owner, 0));
				if (ObjPtrPtr != nullptr && !CanSaveAsReference(Property, *ObjPtrPtr))
					return true;
			}
		}

	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FString FDlgConfigWriter::GetNameWithoutPrefix(const FProperty* Property, const UObject* ObjectPtr)
{
	if (const auto* StructProperty = FNYReflectionHelper::CastProperty<FStructProperty>(Property))
	{
		return GetStringWithoutPrefix(StructProperty->Struct->GetName());
	}

	if (const auto* ObjectProperty = FNYReflectionHelper::CastProperty<FObjectProperty>(Property))
	{
		// Get the Class from the ObjectProperty
		if (ObjectPtr == nullptr)
		{
			return GetStringWithoutPrefix(ObjectProperty->PropertyClass->GetName());
		}

		if (ObjectPtr->GetClass())
		{
			return GetStringWithoutPrefix(ObjectPtr->GetClass()->GetName());
		}
	}

	return "";
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FString FDlgConfigWriter::GetStringWithoutPrefix(const FString& String)
{
	int32 Count = String.Len();
	for (int32 i = 0; i < ComplexNamePrefix.Len() && i < String.Len() && ComplexNamePrefix[i] == String[i]; ++i)
	{
		Count -= 1;
	}

	return String.Right(Count);
}
