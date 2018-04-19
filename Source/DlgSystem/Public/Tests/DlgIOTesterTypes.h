// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "DateTime.h"
#include "Engine/Texture2D.h"

#include "DlgHelper.h"

#include "DlgIOTesterTypes.generated.h"


USTRUCT()
struct FDlgIOTesterOptions
{
	GENERATED_USTRUCT_BODY()

public:
	FDlgIOTesterOptions() {}

	// can Have TArray<Enum>, TSet<Enum>
	UPROPERTY()
	bool bSupportsPureEnumContainer = true;

	// Can have TSet<FStructType>
	UPROPERTY()
	bool bSupportsNonPrimitiveInSet = true;

	// Can we write FLinearColor and FColor
	UPROPERTY()
	bool bSupportsColorPrimitives = true;

	// Can we write FDateTime
	UPROPERTY()
	bool bSupportsDatePrimitive = true;

public:
	bool operator==(const FDlgIOTesterOptions& Other) const
	{
		return bSupportsPureEnumContainer == Other.bSupportsPureEnumContainer &&
			bSupportsNonPrimitiveInSet == Other.bSupportsNonPrimitiveInSet &&
			bSupportsColorPrimitives == Other.bSupportsColorPrimitives;
	}
	bool operator!=(const FDlgIOTesterOptions& Other) const { return !(*this == Other); }

	FString ToString() const
	{
		return FString::Printf(TEXT("bSupportsPureEnumContainer=%d, bSupportsNonPrimitiveInSet=%d, bSupportsColorPrimitives=%d"),
			bSupportsPureEnumContainer, bSupportsNonPrimitiveInSet, bSupportsColorPrimitives);
	}

};


UENUM()
enum class EDlgTestEnum : uint8
{
	ETE_First = 0,
	ETE_Second,
	ETE_Third,
	ETE_NumOf
};

UCLASS()
class UDlgTestObjectPrimitives : public UObject
{
	GENERATED_BODY()
public:
	static UDlgTestObjectPrimitives* New() { return NewObject<UDlgTestObjectPrimitives>(); }
	UDlgTestObjectPrimitives() { SetToDefaults(); }
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	void SetToDefaults();
	bool IsEqual(const UDlgTestObjectPrimitives* Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}
	bool IsEqual(const UDlgTestObjectPrimitives* Other, FString& OutError) const;
	bool operator==(const UDlgTestObjectPrimitives& Other) const;

	FString ToString() const
	{
		return FString::Printf(TEXT("Integer=%d, String=%s"), Integer, *String);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	int32 Integer;

	UPROPERTY()
	FString String;
};

USTRUCT()
struct DLGSYSTEM_API FDlgTestStructPrimitives
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestStructPrimitives() { SetToDefaults(); }
	bool IsEqual(const FDlgTestStructPrimitives& Other, FString& OutError) const;
	bool operator==(const FDlgTestStructPrimitives& Other) const;
	bool operator!=(const FDlgTestStructPrimitives& Other) const { return !(*this == Other); }
	friend uint32 GetTypeHash(const FDlgTestStructPrimitives& This)
	{
		// NOTE not floats in the hash, these should be enough
		uint32 KeyHash = GetTypeHash(This.Integer);
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.String));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Name));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.bBoolean));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Enum));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Color));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.DateTime));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.IntPoint));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Guid));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Texture2DReference));
		return KeyHash;
	}
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	void SetToDefaults();

	FString ToString() const
	{
		return FString::Printf(TEXT("bBoolean=%d, Integer=%d, Float=%f, Enum=%d, Name=%s, String=%s, Text=%s, Color=%s, LinearColor=%s, DateTime=%s"),
			bBoolean, Integer, Float, static_cast<int32>(Enum), *Name.ToString(), *String, *Text.ToString(), *Color.ToString(), *LinearColor.ToString(), *DateTime.ToString());
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	int32 Integer;

	UPROPERTY()
	bool bBoolean;

	UPROPERTY()
	EDlgTestEnum Enum;

	UPROPERTY()
	float Float;

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FString String;

	UPROPERTY()
	FString EmptyString;

	UPROPERTY()
	FText Text;

	UPROPERTY()
	FColor Color;

	UPROPERTY()
	FLinearColor LinearColor;

	UPROPERTY()
	FDateTime DateTime;

	UPROPERTY()
	FIntPoint IntPoint;

	UPROPERTY()
	FVector Vector3;

	UPROPERTY()
	FVector2D Vector2;

	UPROPERTY()
	FVector4 Vector4;

	UPROPERTY()
	FRotator Rotator;

	UPROPERTY()
	FMatrix Matrix;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FGuid Guid;

	UPROPERTY()
	UClass* Class;

	UPROPERTY()
	UObject* EmptyObjectInitialized = nullptr;

	UPROPERTY(meta = (DlgSaveOnlyReference))
	UObject* EmptyObjectInitializedReference = nullptr;

	// Not initialized, check if any writer crashes. It does sadly. Can't know in C++ if a variable is initialized
	//UPROPERTY()
	//UObject* EmptyObject;

	// Check if anything crashes
	UPROPERTY()
	UTexture2D* ConstTexture2D;

	UPROPERTY(meta=(DlgSaveOnlyReference))
	UTexture2D* Texture2DReference;

	UPROPERTY()
	UDlgTestObjectPrimitives* ObjectPrimitives;
};


// Arrays
USTRUCT()
struct DLGSYSTEM_API FDlgTestArrayPrimitive
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestArrayPrimitive() {}
	bool IsEqual(const FDlgTestArrayPrimitive& Other, FString& OutError) const;
	bool operator==(const FDlgTestArrayPrimitive& Other) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TArray<int32> IntArray;

	UPROPERTY()
	TArray<bool> BoolArray;

	UPROPERTY()
	TArray<float> FloatArray;

	UPROPERTY()
	TArray<EDlgTestEnum> EnumArray;

	UPROPERTY()
	TArray<FName> NameArray;

	UPROPERTY()
	TArray<FString> StringArray;
};

USTRUCT()
struct DLGSYSTEM_API FDlgTestArrayComplex
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestArrayComplex() {}
	bool IsEqual(const FDlgTestArrayComplex& Other, FString& OutError) const;
	bool operator==(const FDlgTestArrayComplex& Other) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TArray<FDlgTestStructPrimitives> StructArrayPrimitives;

	UPROPERTY()
	TArray<UDlgTestObjectPrimitives*> ObjectArrayPrimitives;
};


// Sets
USTRUCT()
struct DLGSYSTEM_API FDlgTestSetPrimitive
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestSetPrimitive() {}
	bool IsEqual(const FDlgTestSetPrimitive& Other, FString& OutError) const;
	bool operator==(const FDlgTestSetPrimitive& Other) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);

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
struct DLGSYSTEM_API FDlgTestSetComplex
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestSetComplex() {}
	bool operator==(const FDlgTestSetComplex& Other) const;
	bool IsEqual(const FDlgTestSetComplex& Other, FString& OutError) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);

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
	FDlgTestMapPrimitive() {}
	bool IsEqual(const FDlgTestMapPrimitive& Other, FString& OutError) const;
	bool operator==(const FDlgTestMapPrimitive& Other) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);

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

	UPROPERTY()
	TMap<FString, float> StringToFloatMap;

	UPROPERTY()
	TMap<int32, float> IntToFloatMap;

	UPROPERTY()
	TMap<FName, FColor> NameToColorMap;
};

USTRUCT()
struct DLGSYSTEM_API FDlgTestMapComplex
{
	GENERATED_USTRUCT_BODY()
public:
	FDlgTestMapComplex() {}
	bool operator==(const FDlgTestMapComplex& Other) const;
	bool IsEqual(const FDlgTestMapComplex& Other, FString& OutError) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TMap<int32, FDlgTestStructPrimitives> IntToStructPrimitiveMap;

	UPROPERTY()
	TMap<FDlgTestStructPrimitives, int32> StructPrimitiveToIntMap;
};
