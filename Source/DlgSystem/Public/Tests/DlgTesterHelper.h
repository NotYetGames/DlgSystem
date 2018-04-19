
// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "NoExportTypes.h"

#include "DlgHelper.h"

template <typename ArrayType>
class FDlgTestHelper_ArrayEqualImpl
{
public:
	static bool IsEqual(const TArray<ArrayType>& ThisArray, const TArray<ArrayType>& OtherArray,
		const FString& PropertyName, FString& OutError, const bool bIsPrimitive,
		std::function<FString(const ArrayType&)> GetArrayTypeAsString,
		std::function<bool(const ArrayType& FirstValue, const ArrayType& SecondValue, FString& ComparisonErrorMessage)> AreValuesEqual)
	{
		bool bIsEqual = true;
		auto AreValuesEqualWithoutMessage = [&AreValuesEqual](const ArrayType& FirstValue, const ArrayType& SecondValue) -> bool
		{
			FString DiscardMessage;
			return AreValuesEqual(FirstValue, SecondValue, DiscardMessage);
		};

		if (FDlgHelper::IsArrayEqual<ArrayType>(ThisArray, OtherArray, AreValuesEqualWithoutMessage) == false)
		{
			bIsEqual = false;
			if (ThisArray.Num() != OtherArray.Num())
			{
				OutError += FString::Printf(
					TEXT("\tThis.%s.Num (%d) != Other.%s.Num (%d)\n"),
					*PropertyName, ThisArray.Num(), *PropertyName, OtherArray.Num());
			}

			// Find which element is different
			for (int32 Index = 0; Index < ThisArray.Num() && Index < OtherArray.Num(); Index++)
			{
				// Values differ
				FString ComparisonErorrMessage;
				if (!AreValuesEqual(ThisArray[Index], OtherArray[Index], ComparisonErorrMessage))
				{
					if (bIsPrimitive)
					{
						OutError += FString::Printf(TEXT("\tThis.%s[%d] (%d) != Other.%s[%d] (%d)\n"),
							*PropertyName, Index, *GetArrayTypeAsString(ThisArray[Index]), *PropertyName, Index, *GetArrayTypeAsString(OtherArray[Index]));
					}
					else
					{
						OutError += FString::Printf(TEXT("\tThis.%s[%d] != Other.%s[%d] |message = %s|\n"),
							*PropertyName, Index, *PropertyName, Index, *ComparisonErorrMessage);
					}

				}
			}
		}

		return bIsEqual;
	}
};

// Variant with default comparison
template <typename ArrayType>
class FDlgTestHelper_ArrayPrimitiveVariantImpl
{
public:
	static bool IsEqual(const TArray<ArrayType>& ThisArray, const TArray<ArrayType>& OtherArray,
		const FString& PropertyName, FString& OutError,
		const bool bIsPrimitive,
		std::function<FString(const ArrayType&)> GetArrayTypeAsString)
	{
		return FDlgTestHelper_ArrayEqualImpl<ArrayType>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, bIsPrimitive, GetArrayTypeAsString,
			[](const ArrayType& FirstValue, const ArrayType& SecondValue, FString& ComparisonErrorMessage) -> bool
		{
			return FirstValue == SecondValue;
		});
	}
};

// Pointers variant
template <typename ArrayType>
class FDlgTestHelper_ArrayPointersVariantImpl
{
public:
	static bool IsEqual(const TArray<ArrayType>& ThisArray, const TArray<ArrayType>& OtherArray,
		const FString& PropertyName, FString& OutError,
		const bool bIsPrimitive,
		std::function<FString(const ArrayType&)> GetArrayTypeAsString)
	{
		return FDlgTestHelper_ArrayEqualImpl<ArrayType>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, bIsPrimitive, GetArrayTypeAsString,
			[](const ArrayType& FirstValue, const ArrayType& SecondValue, FString& ComparisonErrorMessage) -> bool
		{
			return FirstValue->IsEqual(SecondValue, ComparisonErrorMessage);
		});
	}
};

// Variant for float
template <>
class FDlgTestHelper_ArrayPrimitiveVariantImpl<float>
{
public:
	static bool IsEqual(const TArray<float>& ThisArray, const TArray<float>& OtherArray,
		const FString& PropertyName, FString& OutError,
		const bool bIsPrimitive,
		std::function<FString(const float&)> GetArrayTypeAsString)
	{
		return FDlgTestHelper_ArrayEqualImpl<float>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, bIsPrimitive, GetArrayTypeAsString,
			[](const float& FirstValue, const float& SecondValue, FString& ComparisonErrorMessage) -> bool
		{
			return FMath::IsNearlyEqual(FirstValue, SecondValue, KINDA_SMALL_NUMBER);
		});
	}
};


// NOTE for SetType = float this won't work, what are you even doing?
template <typename SetType>
struct FDlgTestHelper_SetEqualImpl
{
public:
	static bool IsEqual(const TSet<SetType>& ThisSet, const TSet<SetType>& OtherSet,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const SetType&)> GetSetTypeAsString)
	{
		bool bIsEqual = true;
		if (FDlgHelper::IsSetEqual(ThisSet, OtherSet) == false)
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
};

template <typename KeyType, typename ValueType>
struct FDlgTestHelper_MapEqualImpl
{
public:
	static bool IsEqual(const TMap<KeyType, ValueType>& ThisMap, const TMap<KeyType, ValueType>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const ValueType&)> GetValueTypeAsString,
		std::function<bool(const ValueType& FirstValue, const ValueType& SecondValue)> AreValuesEqual)
	{
		bool bIsEqual = true;
		if (FDlgHelper::IsMapEqual(ThisMap, OtherMap, AreValuesEqual) == false)
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
					if (!AreValuesEqual(*OtherMapValue, ThisElem.Value))
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
			else
			{
				FString OtherStringKeys;
				for (const auto& OtherElem : OtherMap)
				{
					OtherStringKeys += FString::Printf(TEXT("%s,"), *GetKeyTypeAsString(OtherElem.Key));
				}
				OutError += FString::Printf(TEXT("\tOther.%s KEYS (Num = %d) = {%s}\n"),
					*PropertyName, OtherMap.Num(), *OtherStringKeys);
			}

			OutError += FString::Printf(TEXT("\tKeys that ONLY exist in This.%s (Num = %d) = {%s}\n"),
				*PropertyName, NumKeysNotInOther, *KeysNotInOtherString);
			OutError += FString::Printf(TEXT("\tValues that differ:\n%s\n"), *ValuesThatDifferString);
		}

		return bIsEqual;
	}
};


// Variant with default comparison
template <typename KeyType, typename ValueType>
class FDlgTestHelper_MapVariantImpl
{
public:
	static bool IsEqual(const TMap<KeyType, ValueType>& ThisMap, const TMap<KeyType, ValueType>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const ValueType&)> GetValueTypeAsString)
	{
		return FDlgTestHelper_MapEqualImpl<KeyType, ValueType>::IsEqual(ThisMap, OtherMap, PropertyName, OutError, GetKeyTypeAsString, GetValueTypeAsString,
			[](const ValueType& FirstMapValue, const ValueType& SecondMapValue) -> bool
		{
			return FirstMapValue == SecondMapValue;
		});
	}
};

// Variant for float
template <typename KeyType>
class FDlgTestHelper_MapVariantImpl<KeyType, float>
{
public:
	static bool IsEqual(const TMap<KeyType, float>& ThisMap, const TMap<KeyType, float>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const float&)> GetValueTypeAsString)
	{
		return FDlgTestHelper_MapEqualImpl<KeyType, float>::IsEqual(ThisMap, OtherMap, PropertyName, OutError, GetKeyTypeAsString, GetValueTypeAsString,
			[](const float& FirstMapValue, const float& SecondMapValue) -> bool
		{
			return FMath::IsNearlyEqual(FirstMapValue, SecondMapValue, KINDA_SMALL_NUMBER);
		});
	}
};

/**
* Helpers for tests
*/
struct FDlgTestHelper
{
public:
	static bool IsFloatEqual(const float A, const float B)
	{
		return FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER);
	}

	static FString GetFullNameFromObject(const UObject* Object)
	{
		if (!IsValid(Object))
		{
			return TEXT("nullptr");
		}

		return Object->GetFullName();
	}

	template <typename SetType>
	static bool IsSetEqual(const TSet<SetType>& ThisSet, const TSet<SetType>& OtherSet,
		const FString& PropertyName, FString& OutError, std::function<FString(const SetType&)> GetSetTypeAsString)
	{
		return FDlgTestHelper_SetEqualImpl<SetType>::IsEqual(ThisSet, OtherSet, PropertyName, OutError, GetSetTypeAsString);
	}

	template <typename KeyType, typename ValueType>
	static bool IsMapEqual(const TMap<KeyType, ValueType>& ThisMap, const TMap<KeyType, ValueType>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const ValueType&)> GetValueTypeAsString)
	{
		return FDlgTestHelper_MapVariantImpl<KeyType, ValueType>::IsEqual(ThisMap, OtherMap, PropertyName, OutError, GetKeyTypeAsString, GetValueTypeAsString);
	}

	template <typename ArrayType>
	static bool IsPrimitiveArrayEqual(const TArray<ArrayType>& ThisArray, const TArray<ArrayType>& OtherArray,
		const FString& PropertyName, FString& OutError, std::function<FString(const ArrayType&)> GetArrayTypeAsString)
	{
		return FDlgTestHelper_ArrayPrimitiveVariantImpl<ArrayType>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, true, GetArrayTypeAsString);
	}

	template <typename ArrayType>
	static bool IsComplexArrayEqual(const TArray<ArrayType>& ThisArray, const TArray<ArrayType>& OtherArray,
		const FString& PropertyName, FString& OutError)
	{
		return FDlgTestHelper_ArrayEqualImpl<ArrayType>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, false,
			[](const ArrayType& Value) -> FString
			{
				return FString();
			},
			[](const ArrayType& FirstValue, const ArrayType& SecondValue, FString& ComparisonErrorMessage) -> bool
			{
				return FirstValue.IsEqual(SecondValue, ComparisonErrorMessage);
			});
	}

	template <typename ArrayType>
	static bool IsComplexPointerArrayEqual(const TArray<ArrayType*>& ThisArray, const TArray<ArrayType*>& OtherArray,
		const FString& PropertyName, FString& OutError)
	{
		return FDlgTestHelper_ArrayPointersVariantImpl<ArrayType*>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, false,
			[](const auto* Value) -> FString
			{
				return Value->ToString();
			});
	}

	static std::function<FString(const int32&)> IntToString;
	static std::function<FString(const FName&)> NameToString;
	static std::function<FString(const FString&)> StringToString;
	static std::function<FString(const float&)> FloatToString;
	static std::function<FString(const FVector&)> VectorToString;
	static std::function<FString(const FColor&)> ColorToString;
};
