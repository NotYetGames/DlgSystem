// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Misc/DateTime.h"
#include "Engine/Texture2D.h"

#include "DlgSystem/DlgHelper.h"

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

	// Can we Have TMap<Key, UObject*> ?
	UPROPERTY()
	bool bSupportsUObjectValueInMap = true;

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
	First = 0,
	Second,
	Third,
	NumOf
};

UCLASS()
class UDlgTestObjectPrimitivesBase : public UObject
{
	GENERATED_BODY()
	typedef UDlgTestObjectPrimitivesBase Self;
public:
	UDlgTestObjectPrimitivesBase() { SetToDefaults(); }
	virtual void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	virtual void SetToDefaults();
	virtual bool IsEqual(const Self* Other, FString& OutError) const;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(&Other, DiscardError);
	}

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

UCLASS(DefaultToInstanced)
class UDlgTestObjectPrimitives_DefaultToInstanced : public UDlgTestObjectPrimitivesBase
{
	GENERATED_BODY()
	typedef UDlgTestObjectPrimitives_DefaultToInstanced Self;
public:
	UDlgTestObjectPrimitives_DefaultToInstanced() { SetToDefaults(); }
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions) override;
	void SetToDefaults() override;
	bool IsEqual(const Super* Other, FString& OutError) const override;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(&Other, DiscardError);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	int32 InstancedChild;
};

UCLASS()
class UDlgTestObjectPrimitives_ChildA : public UDlgTestObjectPrimitivesBase
{
	GENERATED_BODY()
	typedef UDlgTestObjectPrimitives_ChildA Self;
public:
	UDlgTestObjectPrimitives_ChildA() { SetToDefaults(); }
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions) override;
	void SetToDefaults() override;
	bool IsEqual(const Super* Other, FString& OutError) const override;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(&Other, DiscardError);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	int32 IntegerChildA;
};

UCLASS()
class UDlgTestObjectPrimitives_ChildB : public UDlgTestObjectPrimitivesBase
{
	GENERATED_BODY()
	typedef UDlgTestObjectPrimitives_ChildB Self;
public:
	UDlgTestObjectPrimitives_ChildB() { SetToDefaults(); }
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions) override;
	void SetToDefaults() override;
	bool IsEqual(const Super* Other, FString& OutError) const override;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(&Other, DiscardError);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	FString StringChildB;
};

UCLASS()
class UDlgTestObjectPrimitives_GrandChildA_Of_ChildA : public UDlgTestObjectPrimitives_ChildA
{
	GENERATED_BODY()
	typedef UDlgTestObjectPrimitives_GrandChildA_Of_ChildA Self;
	typedef UDlgTestObjectPrimitivesBase SuperBase;
public:
	UDlgTestObjectPrimitives_GrandChildA_Of_ChildA() { SetToDefaults(); }
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions) override;
	void SetToDefaults() override;
	bool IsEqual(const SuperBase* Other, FString& OutError) const override;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(&Other, DiscardError);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	int32 IntegerGrandChildA_Of_ChildA;
};

// Struct of primitives
USTRUCT()
struct DLGSYSTEM_API FDlgTestStructPrimitives
{
	GENERATED_USTRUCT_BODY()
	typedef FDlgTestStructPrimitives Self;
public:
	FDlgTestStructPrimitives() { SetToDefaults(); }
	bool IsEqual(const Self& Other, FString& OutError) const;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}
	bool operator!=(const Self& Other) const { return !(*this == Other); }
	friend uint32 GetTypeHash(const Self& This)
	{
		// NOTE not floats in the hash, these should be enough
		uint32 KeyHash = GetTypeHash(This.Integer32);
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Integer64));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.String));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Name));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.bBoolean));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Enum));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Color));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.DateTime));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.IntPoint));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.GUID));
		KeyHash = HashCombine(KeyHash, GetTypeHash(This.Texture2DReference));
		return KeyHash;
	}
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	void SetToDefaults();

	FString ToString() const
	{
		return FString::Printf(TEXT("bBoolean=%d, Integer32=%d, Integer64=%lld, Float=%f, Enum=%d, Name=%s, String=%s, Text=%s, Color=%s, LinearColor=%s, DateTime=%s"),
			bBoolean, Integer32, Integer64, Float, static_cast<int32>(Enum), *Name.ToString(), *String, *Text.ToString(), *Color.ToString(), *LinearColor.ToString(), *DateTime.ToString());
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	int32 Integer32;

	UPROPERTY()
	int64 Integer64;

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
	FGuid GUID;

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
	UDlgTestObjectPrimitivesBase* ObjectPrimitivesBase;

	UPROPERTY()
	UDlgTestObjectPrimitives_DefaultToInstanced* ObjectDefaultToInstanced;

	UPROPERTY()
	UDlgTestObjectPrimitives_ChildA* ObjectPrimitivesChildA;

	// Can be nullptr or not
	UPROPERTY()
	UDlgTestObjectPrimitivesBase* ObjectSwitch;

	// Object is defined as base but actually assigned to Child A
	UPROPERTY()
	UDlgTestObjectPrimitivesBase* ObjectPrimitivesPolymorphismChildA;

	UPROPERTY()
	UDlgTestObjectPrimitivesBase* ObjectPrimitivesPolymorphismChildB;

	UPROPERTY()
	UDlgTestObjectPrimitives_GrandChildA_Of_ChildA* ObjectPrimitivesGrandChildA;

	UPROPERTY()
	UDlgTestObjectPrimitivesBase* ObjectPrimitivesPolymorphismBaseGrandChildA;

	UPROPERTY()
	UDlgTestObjectPrimitives_ChildA* ObjectPrimitivesPolymorphismChildGrandChildA;
};


// Struct of Complex types
USTRUCT()
struct DLGSYSTEM_API FDlgTestStructComplex
{
	GENERATED_USTRUCT_BODY()
	typedef FDlgTestStructComplex Self;
public:
	FDlgTestStructComplex() { SetToDefaults(); }
	bool IsEqual(const Self& Other, FString& OutError) const;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}
	bool operator!=(const Self& Other) const { return !(*this == Other); }
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	void SetToDefaults();

	FString ToString() const
	{
		return FString();
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TArray<FDlgTestStructPrimitives> StructArrayPrimitives;

	UPROPERTY()
	TArray<UDlgTestObjectPrimitivesBase*> ArrayOfObjects;

	UPROPERTY(meta = (DlgSaveOnlyReference))
	TArray<UDlgTestObjectPrimitivesBase*> ArrayOfObjectsAsReference;
};


// Arrays simple
USTRUCT()
struct DLGSYSTEM_API FDlgTestArrayPrimitive
{
	GENERATED_USTRUCT_BODY()
	typedef FDlgTestArrayPrimitive Self;
public:
	FDlgTestArrayPrimitive() {}
	bool IsEqual(const Self& Other, FString& OutError) const;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TArray<int32> EmptyArray;

	UPROPERTY()
	TArray<int32> Num1_Array;

	UPROPERTY()
	TArray<int32> Int32Array;

	UPROPERTY()
	TArray<int64> Int64Array;

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

	// Filled with only nulls, check if the writers support it
	UPROPERTY()
	TArray<UDlgTestObjectPrimitivesBase*> ObjectArrayConstantNulls;
};

// Array complex
USTRUCT()
struct DLGSYSTEM_API FDlgTestArrayComplex
{
	GENERATED_USTRUCT_BODY()
	typedef FDlgTestArrayComplex Self;
public:
	FDlgTestArrayComplex() {}
	bool IsEqual(const Self& Other, FString& OutError) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TArray<FDlgTestStructPrimitives> StructArrayPrimitives;

	UPROPERTY()
	TArray<FDlgTestArrayPrimitive> StructArrayOfArrayPrimitives;

	UPROPERTY()
	TArray<UDlgTestObjectPrimitivesBase*> ObjectArrayFrequentsNulls;

	UPROPERTY()
	TArray<UDlgTestObjectPrimitivesBase*> ObjectArrayPrimitivesBase;

	UPROPERTY()
	TArray<UDlgTestObjectPrimitivesBase*> ObjectArrayPrimitivesAll;
};


// Set primitive
USTRUCT()
struct DLGSYSTEM_API FDlgTestSetPrimitive
{
	GENERATED_USTRUCT_BODY()
	typedef FDlgTestSetPrimitive Self;
public:
	FDlgTestSetPrimitive() {}
	bool IsEqual(const Self& Other, FString& OutError) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TSet<int32> EmptySet;

	UPROPERTY()
	TSet<int32> Num1_Set;

	UPROPERTY()
	TSet<int32> Int32Set;

	UPROPERTY()
	TSet<int64> Int64Set;

	UPROPERTY()
	TSet<EDlgTestEnum> EnumSet;

	UPROPERTY()
	TSet<FName> NameSet;

	UPROPERTY()
	TSet<FString> StringSet;
};

// Set complex
USTRUCT()
struct DLGSYSTEM_API FDlgTestSetComplex
{
	GENERATED_USTRUCT_BODY()
	typedef FDlgTestSetComplex Self;
public:
	FDlgTestSetComplex() {}
	bool IsEqual(const Self& Other, FString& OutError) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TSet<FDlgTestStructPrimitives> StructSetPrimitives;
};


// Map primitive
USTRUCT()
struct DLGSYSTEM_API FDlgTestMapPrimitive
{
	GENERATED_USTRUCT_BODY()
	typedef FDlgTestMapPrimitive Self;
public:
	FDlgTestMapPrimitive() {}
	bool IsEqual(const Self& Other, FString& OutError) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	void CheckInvariants() const;
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}
	FString ToString() const
	{
		return FString::Printf(TEXT("IntToIntMap.Num()=%d, IntToStringMap.Num()=%d"),  Int32ToInt32Map.Num(), Int32ToStringMap.Num());
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TMap<int32, int32> EmptyMap;

	UPROPERTY()
	TMap<int32, int32> Int32ToInt32Map;

	UPROPERTY()
	TMap<int64, int64> Int64ToInt64Map;

	UPROPERTY()
	TMap<int32, FString> Int32ToStringMap;

	UPROPERTY()
	TMap<int32, FName> Int32ToNameMap;

	UPROPERTY()
	TMap<FString, int32> StringToInt32Map;

	UPROPERTY()
	TMap<FString, FString> StringToStringMap;

	UPROPERTY()
	TMap<FName, int32> NameToInt32Map;

	UPROPERTY()
	TMap<FName, FName> NameToNameMap;

	UPROPERTY()
	TMap<FString, float> StringToFloatMap;

	UPROPERTY()
	TMap<int32, float> Int32ToFloatMap;

	UPROPERTY()
	TMap<FName, FColor> NameToColorMap;

	UPROPERTY()
	TMap<FString, UDlgTestObjectPrimitivesBase*> ObjectFrequentsNullsMap;

	// Filled with only nulls, check if the writers support it
	UPROPERTY()
	TMap<FString, UDlgTestObjectPrimitivesBase*> ObjectConstantNullMap;

	UPROPERTY()
	TMap<FString, UDlgTestObjectPrimitivesBase*> ObjectPrimitivesAllMap;
};

// Map complex
USTRUCT()
struct DLGSYSTEM_API FDlgTestMapComplex
{
	GENERATED_USTRUCT_BODY()
	typedef FDlgTestMapComplex Self;
public:
	FDlgTestMapComplex() {}
	bool IsEqual(const Self& Other, FString& OutError) const;
	void GenerateRandomData(const FDlgIOTesterOptions& InOptions);
	bool operator==(const Self& Other) const
	{
		FString DiscardError;
		return IsEqual(Other, DiscardError);
	}

public:
	// Tester Options
	FDlgIOTesterOptions Options;

	UPROPERTY()
	TMap<int32, FDlgTestStructPrimitives> Int32ToStructPrimitiveMap;

	UPROPERTY()
	TMap<FName, FDlgTestStructPrimitives> NameToStructPrimitiveMap;

	UPROPERTY()
	TMap<FDlgTestStructPrimitives, int32> StructPrimitiveToIntMap;

	UPROPERTY()
	TMap<FName, FDlgTestMapPrimitive> NameToStructOfMapPrimitives;

	UPROPERTY()
	TMap<FName, FDlgTestArrayPrimitive> NameToStructOfArrayPrimitives;

	UPROPERTY()
	TMap<FName, FDlgTestSetPrimitive> NameToStructOfSetPrimitives;

	UPROPERTY()
	TMap<FName, FDlgTestArrayComplex> NameToStructOfArrayComplex;

	UPROPERTY()
	TMap<FName, FDlgTestSetComplex> NameToStructOfSetComplex;
};
