// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreTypes.h"

#include "DlgHelper.h"

#include "DlgIOTesterTypes.generated.h"


struct FDlgIOTesterOptions
{
public:
	FDlgIOTesterOptions() {}

	// can Have TArray<Enum>, TSet<Enum>
	bool bSupportsPureEnumContainer = true;

	// Can have TSet<FStructType>
	bool bSupportsNonPrimitiveInSet = true;
};

UENUM()
enum class EDlgTestEnum : uint8
{
	ETE_First = 0,
	ETE_Second,
	ETE_Third,
	ETE_NumOf
};

USTRUCT()
struct DLGSYSTEM_API FDlgTestStructPrimitives
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestStructPrimitives(const FDlgIOTesterOptions InOptions = {}) : Options(InOptions) { GenerateRandomData(); }
	bool IsEqual(const FDlgTestStructPrimitives& Other, FString& OutError) const;
	bool operator==(const FDlgTestStructPrimitives& Other) const;
	bool operator!=(const FDlgTestStructPrimitives& Other) const { return !(*this == Other); }
	friend uint32 GetTypeHash(const FDlgTestStructPrimitives& This)
	{
		uint32 KeyHash = GetTypeHash(This.Integer);
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.String));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Name));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.bBoolean));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Enum));
		return KeyHash;
	}
	void GenerateRandomData();

	FString ToString() const
	{
		return FString::Printf(TEXT("bBoolean=%d, Integer=%d, Float=%f, Enum=%d, Name=`%s`, String=`%s`, Text=`%s`"),
			bBoolean, Integer, Float, static_cast<int32>(Enum), *Name.ToString(), *String, *Text.ToString());
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	int32 Integer = 42;

	UPROPERTY()
	bool bBoolean = false;

	UPROPERTY()
	EDlgTestEnum Enum;

	UPROPERTY()
	float Float = -23.f;

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FString String;

	UPROPERTY()
	FText Text;
};


// Arrays
USTRUCT()
struct DLGSYSTEM_API FDlgTestArrayPrimitive
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestArrayPrimitive(const FDlgIOTesterOptions InOptions = {}) : Options(InOptions) { GenerateRandomData(); }
	bool IsEqual(const FDlgTestArrayPrimitive& Other, FString& OutError) const;
	bool operator==(const FDlgTestArrayPrimitive& Other) const;
	void GenerateRandomData();

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TArray<int32> IntArray;

	// TODO float, be careful about precision
	// UPROPERTY()
	// TArray<float> FloatArray;

	UPROPERTY()
	TArray<bool> BoolArray;

	UPROPERTY()
	TArray<EDlgTestEnum> EnumArray;

	UPROPERTY()
	TArray<FName> NameArray;

	UPROPERTY()
	TArray<FString> StringArray;
};

USTRUCT()
struct DLGSYSTEM_API FDlgTestArrayStruct
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestArrayStruct(const FDlgIOTesterOptions InOptions = {}) : Options(InOptions) { GenerateRandomData(); }
	bool IsEqual(const FDlgTestArrayStruct& Other, FString& OutError) const;
	bool operator==(const FDlgTestArrayStruct& Other) const;
	void GenerateRandomData();

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TArray<FDlgTestStructPrimitives> StructArrayPrimitives;
};


// Sets
USTRUCT()
struct DLGSYSTEM_API FDlgTestSetPrimitive
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestSetPrimitive(const FDlgIOTesterOptions InOptions = {}) : Options(InOptions) { GenerateRandomData(); }
	bool IsEqual(const FDlgTestSetPrimitive& Other, FString& OutError) const;
	bool operator==(const FDlgTestSetPrimitive& Other) const;
	void GenerateRandomData();

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TSet<int32> IntSet;

	UPROPERTY()
	TSet<EDlgTestEnum> EnumSet;

	UPROPERTY()
	TSet<FName> NameSet;

	UPROPERTY()
	TSet<FString> StringSet;
};

USTRUCT()
struct DLGSYSTEM_API FDlgTestSetStruct
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestSetStruct(const FDlgIOTesterOptions InOptions = {}) : Options(InOptions) { GenerateRandomData(); };
	bool operator==(const FDlgTestSetStruct& Other) const;
	bool IsEqual(const FDlgTestSetStruct& Other, FString& OutError) const;
	void GenerateRandomData();

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TSet<FDlgTestStructPrimitives> StructSetPrimitives;
};


// Map
USTRUCT()
struct DLGSYSTEM_API FDlgTestMapPrimitive
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestMapPrimitive(const FDlgIOTesterOptions InOptions = {}) : Options(InOptions) { GenerateRandomData(); }
	bool IsEqual(const FDlgTestMapPrimitive& Other, FString& OutError) const;
	bool operator==(const FDlgTestMapPrimitive& Other) const;
	void GenerateRandomData();

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TMap<int32, int32> IntToIntMap;

	UPROPERTY()
	TMap<int32, FString> IntToStringMap;

	UPROPERTY()
	TMap<int32, FName> IntToNameMap;

	UPROPERTY()
	TMap<FString, int32> StringToIntMap;

	UPROPERTY()
	TMap<FString, FString> StringToStringMap;

	UPROPERTY()
	TMap<FName, int32> NameToIntMap;

	UPROPERTY()
	TMap<FName, FName> NameToNameMap;
};

USTRUCT()
struct DLGSYSTEM_API FDlgTestMapStruct
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestMapStruct(const FDlgIOTesterOptions InOptions = {}) : Options(InOptions) { GenerateRandomData(); };
	bool operator==(const FDlgTestMapStruct& Other) const;
	bool IsEqual(const FDlgTestMapStruct& Other, FString& OutError) const;
	void GenerateRandomData();

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TMap<int32, FDlgTestStructPrimitives> IntToStructPrimitiveMap;

	UPROPERTY()
	TMap<FDlgTestStructPrimitives, int32> StructPrimitiveToIntMap;
};


template <typename ArrayType>
bool TestArrayIsEqualToOther(const TArray<ArrayType>& ThisArray, const TArray<ArrayType>& OtherArray,
	const FString& PropertyName, FString& OutError,
	std::function<FString(const int32 Index, const ArrayType&, const ArrayType&)> CompareEveryElement)
{
	bool bIsEqual = true;
	if (ThisArray != OtherArray)
	{
		bIsEqual = false;
		if (ThisArray.Num() != OtherArray.Num())
		{
			OutError += FString::Printf(
				TEXT("\tThis.%s.Num (%d) != Other.%s.Num (%d)\n"),
				*PropertyName, ThisArray.Num(), *PropertyName, OtherArray.Num());
		}

		// Find which element is different
		for (int32 i = 0; i < ThisArray.Num(); i++)
		{
			OutError += CompareEveryElement(i, ThisArray[i], OtherArray[i]);
		}
	}

	return bIsEqual;
}

template <typename SetType>
bool TestSetIsEqualToOther(const TSet<SetType>& ThisSet, const TSet<SetType>& OtherSet,
	const FString& PropertyName, FString& OutError,
	std::function<FString(const SetType&)> GetSetTypeAsString)
{
	bool bIsEqual = true;
	if (FDlgHelper::AreSetsEqual(ThisSet, OtherSet) == false)
	{
		bIsEqual = false;
		if (ThisSet.Num() != OtherSet.Num())
		{
			OutError += FString::Printf(
				TEXT("\tThis.%s.Num (%d) != Other.%s.Num (%d)\n"),
				*PropertyName, ThisSet.Num(), *PropertyName, OtherSet.Num());
		}

		// Find The set that is different from the other
		const TSet<SetType> NotInOther = ThisSet.Difference(OtherSet);
		OutError += FString::Printf(
			TEXT("\tNotInOther = This.%s - Other.%s is of length = %d\n"),
			*PropertyName, *PropertyName, NotInOther.Num());

		FString NotInOtherString;
		for (const SetType& ValueInOther : NotInOther)
		{
			NotInOtherString += FString::Printf(TEXT("%s,"), *GetSetTypeAsString(ValueInOther));
		}
		OutError += FString::Printf(TEXT("\tNotInOther = {%s}\n"), *NotInOtherString);
	}

	return bIsEqual;
}

template <typename KeyType, typename ValueType>
bool TestMapIsEqualToOther(const TMap<KeyType, ValueType>& ThisMap, const TMap<KeyType, ValueType>& OtherMap,
	const FString& PropertyName, FString& OutError,
	std::function<FString(const KeyType&)> GetKeyTypeAsString,
	std::function<FString(const ValueType&)> GetValueTypeAsString)
{
	bool bIsEqual = true;
	if (FDlgHelper::AreMapsEqual(ThisMap, OtherMap) == false)
	{
		bIsEqual = false;
		if (ThisMap.Num() != OtherMap.Num())
		{
			OutError += FString::Printf(
				TEXT("\tThis.%s.Num (%d) != Other.%s.Num (%d)\n"),
				*PropertyName, ThisMap.Num(), *PropertyName, OtherMap.Num());
		}

		// Find values in ThisMap that do not exist in OtherMap
		int32 NumKeysNotInOther = 0;
		FString KeysNotInOtherString;
		FString ValuesThatDifferString;
		const bool OtherMapIsEmpty = OtherMap.Num() == 0;
		for (const auto& ThisElem : ThisMap)
		{
			const ValueType* OtherMapValue = OtherMap.Find(ThisElem.Key);
			if (OtherMapValue != nullptr)
			{
				if (*OtherMapValue != ThisElem.Value)
				{
					ValuesThatDifferString += FString::Printf(
						TEXT("\tThis.%s[key] (%s) != Other.%s[key] (%s). Key = (%s)\n"),
						*PropertyName, *GetValueTypeAsString(ThisElem.Value),
						*PropertyName, *GetValueTypeAsString(*OtherMapValue),
						*GetKeyTypeAsString(ThisElem.Key));
				}
			}
			else
			{
				KeysNotInOtherString += FString::Printf(TEXT("%s,"), *GetKeyTypeAsString(ThisElem.Key));
				NumKeysNotInOther++;
			}
		}

		if (OtherMapIsEmpty)
		{
			OutError += FString::Printf(TEXT("\tOther.%s IS EMPTY\n"), *PropertyName);
		}
		OutError += FString::Printf(TEXT("\tKeys that ONLY exist in This.%s (Num = %d) = {%s}\n"),
			*PropertyName, NumKeysNotInOther, *KeysNotInOtherString);
		OutError += FString::Printf(TEXT("\tValues that differ:\n%s\n"), *ValuesThatDifferString);
	}

	return bIsEqual;
}

