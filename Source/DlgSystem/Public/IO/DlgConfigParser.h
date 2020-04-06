// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include <functional>
#include "CoreTypes.h"
#include "Logging/LogMacros.h"

#include "IDlgParser.h"
#include "NYReflectionHelper.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDlgConfigParser, Log, All);


/**
 * Because there is always another config format
 * Parser can be used to process the config word by word
 * 'word' is defined here as the thing between whitespaces
 * '{' and '}' are exceptions:
 *		they can be in block and treated separately as *first* character recursively (e.g. "}}}}" is valid but "Name}" is not
 * Default functionality can read a whole data structure from the proper config format
 * Supported Primitives: FString, FName, int32, bool, float
 * Supported complex types: Struct, Class
 * Supported containers: Array(*), Map (Key: Primitive + Struct, Value: Primitive + Struct), Set (Primitive)
 * Enum support: limited
 *		pure enum array is not supported (enum can be a member in a struct which is in an array, but it can't be in an array directly)
 *		enum set, enum in map: NOT IMPLEMENTED
 *
 * Heterogenous Collections are also supported (e.g. TArray<UObject*>)
 */
class DLGSYSTEM_API FDlgConfigParser : public IDlgParser
{
public:
	/**
	 *  Creates empty parser
	 *  Call ReinitializeParser() on it to parse a file
	 *  It is better to use a single parser for more files from the same type if those contain dynamic arrays,
	 *  because it has to check all class names for the first time for each class
	 */
	FDlgConfigParser(const FString InPreTag = "");

	/**
	 * Creates the parser and finds the first word
	 *
	 * @param FilePath: path to the text file the parser will parse
	 * @param InPreTag: Optional pretag added at the front of structs and classes
	 */
	FDlgConfigParser(const FString& FilePath, const FString& InPreTag);

	// IDlgParser Interface
	void InitializeParser(const FString& FilePath) override;

	void InitializeParserFromString(const FString& Text) override;

	bool IsValidFile() const override
	{
		return HasValidWord();
	}

	void ReadAllProperty(const UStruct* ReferenceClass, void* TargetObject, UObject* DefaultObjectOuter = nullptr) override;

	void ResetParser();

	/**
	 * Tries to read a property from the config starting from the current word
	 * If everything goes well the active word will be the first word *after* the property value
	 * If the initial active word wasn't a property the next word will be the active one (or eof)
	 *
	 * @param ReferenceClass: UStruct (or UClass) of the TargetObject
	 * @param TargetObject: pointer to the object instance the function will modify based on the config values
	 * @param DefaultObjectOuter: Outer used for UObject construction if necessary
	 *
	 * @return Whether the word was interpreted successfully as a property or not
	 */
	bool ReadProperty(const UStruct* ReferenceClass, void* TargetObject, UObject* DefaultObjectOuter = nullptr);

private:
	/**
	 * Tries to read a block where each entry is a property
	 * a block is a '{', then properties, then '}'
	 *
	 * @param TargetObject: pointer to the object instance the function will modify based on the config values
	 * @param ReferenceClass: UStruct (or UClass) of the TargetObject
	 * @param bBlockStartAlreadyRead: if '{' is the actual word or the next
	 * @param DefaultObjectOuter: Outer used for UObject construction if necessary
	 *
	 * @return Whether there was any syntax error or not
	 */
	bool ReadPurePropertyBlock(void* TargetObject, const UStruct* ReferenceClass, bool bBlockStartAlreadyRead = false, UObject* DefaultObjectOuter = nullptr);

	/**
	 * Creates a config string from the given top level object
	 * WARNING: it exports *every* non-transient property from the supported types (check the class constructor)
	 *
	 * @param ReferenceType: UStruct (or UClass) of SourceObject
	 * @param SourceObject: pointer to the object instance the function will copy to config
	 *
	 * @return the constructed config string
	 */
	static FString ConstructConfigFile(const UStruct* ReferenceType, void* SourceObject);


	/**
	 * Jumps to the next word in the parsed config
	 *
	 * @return Whether a new word was found (false -> end of file)
	 */
	bool FindNextWord();

	/** Checks if the next parsed word begins with '"' or not */
	bool IsNextWordString() const;

	/** Checks if the actual word begins with '"' or not */
	bool IsActualWordString() const;

	/**
	 * Jumps to the next word in the parsed config
	 * warning is printed on eof based on input
	 *
	 * @return Whether a new word was found (false -> end of file)
	 */
	bool FindNextWord(const FString& ExpectedStuff);

	/**
	 * Jumps to the next word in the parsed config, and checks if it is a block start character ("{")
	 * warning is printed if the word is not "{" or if the end of file is reached
	 * @return Whether a new word "{" was found
	 */
	bool FindNextWordAndCheckIfBlockStart(const FString& BlockName);

	/**
	 * Jumps to the next word in the parsed config, and checks if it is a block end character ("}")
	 * Warning is only printed if the end of file is reached
	 * @return Whether a new word "}" was found
	 */
	bool FindNextWordAndCheckIfBlockEnd(const FString& BlockName);

	/**
	 * Checks the active word if it is a block end character ("}")
	 * Warning is only printed if the end of file is reached
	 * @Return Whether the active word is "}"
	 */
	bool CheckIfBlockEnd(const FString& BlockName);

	/**
	 * Compares the input string with the active word
	 * This function is CASE SENSITIVE
	 *
	 * @return Whether the word and the strings are equal
	 */
	bool CompareToActiveWord(const FString& StringToCompare) const;

	/**
	 * Calculates the line count for the current word
	 * TODO: could be counted on the fly
	 * @return Active line index, or INDEX_NONE if there is no valid word
	 */
	int32 GetActiveLineNumber() const;

	// @return false if the file could not be read or if the end of file is reached
	bool HasValidWord() const { return bHasValidWord; }

	/**
	 * Should be avoided whenever possible (use CompareToActiveWord)
	 * @return the active word, or an empty string if there isn't any
	 */
	FString GetActiveWord() const { return bHasValidWord ? String.Mid(From, Len) : ""; }

	/**
	 * @param FloatValue: out float value if the call succeeds
	 * @return the active word converted, or an empty string if there isn't any
	 */
	bool GetActiveWordAsFloat(float& FloatValue) const;

	/**
	 *
	 * @param TabCount: amount of tabs at the begining of each new line
	 * @param OutString: InOut param, new lines will be added
	 *
	 * @return the constructed config string
	 */
	static void ConstructConfigFileInternal(const UStruct* ReferenceType, int32 TabCount, void* SourceObject, FString& OutString);


	/** Tries to read the actual config value as a primitive property (all supported primitive is checked) */
	bool TryToReadPrimitiveProperty(void* Target, FNYProperty* PropertyBase);

	bool TryToReadEnum(void* TargetObject, FNYProperty* PropertyBase);

	/** Return value shows if it was read properly or not */
	bool ReadSet(void* TargetObject, FNYSetProperty& Property, UObject* DefaultObjectOuter);

	/** Return value shows if it was read properly or not */
	bool ReadMap(void* TargetObject, FNYMapProperty& Property, UObject* DefaultObjectOuter);

	/**
	 * Tries to read the actual config value as a Type
	 *
	 * @param Target The struct/object whose properties are searched and modified based on the config string
	 * @param PropertyBase The property - can be a TypeProperty or an ArrayProperty storing Type-s
	 * @param OnGetAsValue Function returning the actual word converted to Type
	 * @param TypeName The name of the type for logs (e.g. "bool")
	 *
	 * @return true if the property was a PropertyType or an array containging Type
	 */
	template <typename Type, typename PropertyType>
	bool ReadPrimitiveProperty(void* Target,
							   FNYProperty* PropertyBase,
							   std::function<Type()> OnGetAsValue,
							   const FString& TypeName,
							   bool bCanBeEmpty);


	/**
	 * Tries to read a complex object from the config (object/struct)
	 *
	 * @param Target The struct/object whose properties are searched and modified based on the config string
	 * @param Property can be a PropertyType or an ArrayProperty storing Type-s
	 * @param ReferenceType The uclass/ustruct of the object the function is supposed to read
	 * @param OnInitValue Creates the complex type if necessary (void*: the newly created object, UClass: the type of the object or nullptr, UObject*: outer)
	 * @param Outer the object used as outer for uobject construction
	 *
	 * @return true if the complex type was read properly
	 */
	template <typename PropertyType>
	bool ReadComplexProperty(void* Target,
							 FNYProperty* Property,
							 const UStruct* ReferenceType,
							 std::function<void*(void*, const UClass*, UObject*)> OnInitValue,
							 UObject* Outer);


	bool GetAsBool() const;
	float GetAsFloat() const;
	int32 GetAsInt32() const;
	int64 GetAsInt64() const;
	FName GetAsName() const;
	FString GetAsString() const;
	FText GetAsText() const;

	void OnInvalidValue(const FString& PropType) const;

	void* OnInitObject(void* ValuePtr, const UClass* ChildClass, UObject* OuterInit);

	/** gets the UClass from an UObject or from an array of UObjects */
	const UClass* SmartGetPropertyClass(FNYProperty* Property, const FString& TypeName);

private:

	/** file path stored for log messages */
	FString FileName = "";
	/** whole config stored in a string */
	FString String = "";
	/** first character of active word */
	int32 From = 0;
	/** length of active word */
	int32 Len = 0;

	/** optional pretag before struct and class names */
	const FString PreTag;

	bool bHasValidWord = false;

	// Nullptr value?
	bool bHasNullptr = false;

	/** used to skip the closing '"' */
	bool bActiveIsString = false;
};


template <typename Type, typename PropertyType>
bool FDlgConfigParser::ReadPrimitiveProperty(void* Target,
											 FNYProperty* PropertyBase,
											 std::function<Type()> OnGetAsValue,
											 const FString& TypeName,
											 bool bCanBeEmpty)
{
	// try to find a member variable with the name
	PropertyType* Property = FNYReflectionHelper::CastProperty<PropertyType>(PropertyBase);
	if (Property == nullptr)
	{
		// Array
		// No property found, let's check if there is an array with the same name
		auto* ArrayProp = FNYReflectionHelper::CastProperty<FNYArrayProperty>(PropertyBase);

		// SmartCastProperty gets the inner type of the array and uses dynamic_cast to cast it to the proper type
		if (ArrayProp == nullptr || FNYReflectionHelper::SmartCastProperty<PropertyType>(ArrayProp) == nullptr)
		{
			return false;
		}

		TArray<Type>* Array = ArrayProp->ContainerPtrToValuePtr<TArray<Type>>(Target);
		Array->Empty();
		if (FindNextWordAndCheckIfBlockStart(TypeName + "Array"))
		{
			// read values until the block ends
			while (!FindNextWordAndCheckIfBlockEnd(TypeName + "Array"))
			{
				if (!bHasValidWord && !bCanBeEmpty)
				{
					UE_LOG(LogDlgConfigParser, Warning, TEXT("Unexpected end of file while array end was expected (config %s)"), *FileName)
				}
				else
				{
					Array->Add(OnGetAsValue());
				}
			}
		}
	}
	else
	{
		// Primitive type
		FindNextWord();
		if (bHasValidWord || bCanBeEmpty)
		{
			Property->SetPropertyValue_InContainer(Target, OnGetAsValue());
		}
		else
		{
			UE_LOG(LogDlgConfigParser, Warning, TEXT("Unexpected end of file while %s value was expected (config %s)"), *TypeName, *FileName)
		}
	}

	FindNextWord();
	return true;
}


template <typename PropertyType>
bool FDlgConfigParser::ReadComplexProperty(void* Target,
										   FNYProperty* Property,
										   const UStruct* ReferenceType,
										   std::function<void*(void*, const UClass*, UObject*)> OnInitValue,
										   UObject* Outer)
{
	PropertyType* ElementProp = FNYReflectionHelper::CastProperty<PropertyType>(Property);
	if (ElementProp == nullptr)
	{
		auto* ArrayProp = FNYReflectionHelper::CastProperty<FNYArrayProperty>(Property);
		if (ArrayProp == nullptr || FNYReflectionHelper::SmartCastProperty<PropertyType>(ArrayProp) == nullptr)
		{
			return false;
		}

		// Array
		FScriptArrayHelper Helper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<uint8>(Target));
		Helper.EmptyValues();
		if (!FindNextWordAndCheckIfBlockStart(ReferenceType->GetName() + "Array element") || !FindNextWord("{ or }"))
		{
			return false;
		}

		while (!CheckIfBlockEnd(ReferenceType->GetName() + "Array element"))
		{
			const UClass* ReferenceClass = Cast<UClass>(ReferenceType);
			if (ReferenceClass != nullptr)
			{
				if (IsActualWordString() && ArrayProp->Inner != nullptr) // UObject by reference
				{
					const FString Path = GetActiveWord();
					void* TargetPtr = Helper.GetRawPtr(Helper.AddValue());
					auto* ObjectPtrPtr = static_cast<UObject**>(TargetPtr);
					*ObjectPtrPtr = nullptr; // reset first
					if (!Path.TrimStartAndEnd().IsEmpty()) // null reference ?
					{
						*ObjectPtrPtr = StaticLoadObject(UObject::StaticClass(), Outer, *Path);
					}

					FindNextWord();
					continue;
				}

				const FString TypeName = PreTag + GetActiveWord();
				ReferenceClass = GetChildClassFromName(ReferenceClass, TypeName);
				if (ReferenceClass == nullptr)
				{
					UE_LOG(LogDlgConfigParser, Warning, TEXT("Failed to find class %s in config %s (:%d)"),
						*TypeName, *FileName, GetActiveLineNumber());
					return false;
				}
			}
			else if (!CompareToActiveWord("{"))
			{
				if (!bHasValidWord)
				{
					UE_LOG(LogDlgConfigParser, Warning, TEXT("Unexpected end of file while %s array element end was expected (config %s)"),
														*ReferenceType->GetName(), *FileName)
				}
				else
				{
					UE_LOG(LogDlgConfigParser, Warning, TEXT("'{' expected but %s found for %s array element (config %s :%d)"),
														*ReferenceType->GetName(), *GetActiveWord(), *FileName, GetActiveLineNumber());
				}
				return false;
			}

			void* Value = OnInitValue(Helper.GetRawPtr(Helper.AddValue()), ReferenceClass, Outer);
			if (!ReadPurePropertyBlock(Value, ReferenceClass == nullptr ? ReferenceType : ReferenceClass, ReferenceClass == nullptr, Outer))
			{
				return false;
			}
		}

		FindNextWord();
		return true;
	}

	// Try pure block
	void* Value = OnInitValue(ElementProp->template ContainerPtrToValuePtr<void>(Target), Cast<UClass>(ReferenceType), Outer);
	return ReadPurePropertyBlock(Value, ReferenceType, false, Outer);
}
