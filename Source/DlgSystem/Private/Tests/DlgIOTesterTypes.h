// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreTypes.h"

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
	friend uint32 GetTypeHash(const FDlgTestStructPrimitives& This)
	{
		return GetTypeHash(This.Integer) + GetTypeHash(This.String) + GetTypeHash(This.bBoolean);
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
	TSet<bool> BoolSet;

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

template <typename Type>
bool SetEqualsSet(const TSet<Type>& FirstSet, const TSet<Type>& SecondSet)
{
	if (FirstSet.Num() == SecondSet.Num())
	{
		// No duplicates should be found
		TSet<Type> Set = FirstSet;
		Set.Append(SecondSet);

		return Set.Num() == FirstSet.Num();
	}

	return false;
}
