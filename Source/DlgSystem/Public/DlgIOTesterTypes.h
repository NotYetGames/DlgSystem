// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreTypes.h"

#include "DlgIOTesterTypes.generated.h"


UENUM()
enum class EDlgTestEnum : uint8
{
	ETE_First,
	ETE_Second,
	ETE_Third,
	ETE_NumOf
};

USTRUCT(BlueprintType, Blueprintable)
struct DLGSYSTEM_API FDlgTestStructPrimitives
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY()
	bool bBoolean;

	UPROPERTY()
	int32 Integer;

	UPROPERTY()
	float Float;

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FString String;

	UPROPERTY()
	FText Text;

	UPROPERTY()
	EDlgTestEnum Enum;

	/** generates random values */
	FDlgTestStructPrimitives();
	bool operator==(const FDlgTestStructPrimitives& Other) const;
	friend uint32 GetTypeHash(const FDlgTestStructPrimitives& This)
	{
		return GetTypeHash(This.Integer) + GetTypeHash(This.String);
	}
};

// Arrays

USTRUCT(BlueprintType, Blueprintable)
struct DLGSYSTEM_API FDlgTestArrayPrimitive
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	TArray<FName> NameArray;
	UPROPERTY()
	TArray<FString> StringArray;

	FDlgTestArrayPrimitive();
	bool operator==(const FDlgTestArrayPrimitive& Other) const;
};

USTRUCT(BlueprintType, Blueprintable)
struct DLGSYSTEM_API FDlgTestArrayStruct
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	TArray<FDlgTestStructPrimitives> StructArray;

	FDlgTestArrayStruct();
	bool operator==(const FDlgTestArrayStruct& Other) const;
};

USTRUCT(BlueprintType, Blueprintable)
struct DLGSYSTEM_API FDlgTestArrayEnum
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	TArray<EDlgTestEnum> EnumArray;

	FDlgTestArrayEnum();
	bool operator==(const FDlgTestArrayEnum& Other) const;
};

// Sets

USTRUCT(BlueprintType, Blueprintable)
struct DLGSYSTEM_API FDlgTestSetPrimitive
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	TSet<int32> IntSet;
	UPROPERTY()
	TSet<FString> StringSet;

	FDlgTestSetPrimitive();
	bool operator==(const FDlgTestSetPrimitive& Other) const;
};

USTRUCT(BlueprintType, Blueprintable)
struct DLGSYSTEM_API FDlgTestSetStruct
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	TSet<FDlgTestStructPrimitives> Set;

	FDlgTestSetStruct();
	bool operator==(const FDlgTestSetStruct& Other) const;
};


USTRUCT(BlueprintType, Blueprintable)
struct DLGSYSTEM_API FDlgTestSetEnum
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	TSet<EDlgTestEnum> Set;

	FDlgTestSetEnum();
	bool operator==(const FDlgTestSetEnum& Other) const;
};

template <typename Type>
bool SetEqualsSet(const TSet<Type>& FirstSet, const TSet<Type>& SecondSet)
{
	if (FirstSet.Num() == SecondSet.Num())
	{
		TSet<Type> Set = FirstSet;
		Set.Append(SecondSet);

		return Set.Num() == FirstSet.Num();
	}

	return false;
}
