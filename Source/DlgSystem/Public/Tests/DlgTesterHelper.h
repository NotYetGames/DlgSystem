// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include <functional>
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

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
				FString ComparisonErrorMessage;
				if (!AreValuesEqual(ThisArray[Index], OtherArray[Index], ComparisonErrorMessage))
				{
					if (bIsPrimitive)
					{
						OutError += FString::Printf(TEXT("\tThis.%s[%d] (%d) != Other.%s[%d] (%d)\n"),
							*PropertyName, Index, *GetArrayTypeAsString(ThisArray[Index]), *PropertyName, Index, *GetArrayTypeAsString(OtherArray[Index]));
					}
					else
					{
						OutError += FString::Printf(TEXT("\tThis.%s[%d] != Other.%s[%d] |message = %s|\n"),
							*PropertyName, Index, *PropertyName, Index, *ComparisonErrorMessage);
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
		std::function<FString(const ArrayType&)> GetArrayTypeAsString)
	{
		return FDlgTestHelper_ArrayEqualImpl<ArrayType>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, true, GetArrayTypeAsString,
			[](const ArrayType& FirstValue, const ArrayType& SecondValue, FString& ComparisonErrorMessage) -> bool
		{
			return FirstValue == SecondValue;
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
		std::function<FString(const float&)> GetArrayTypeAsString)
	{
		return FDlgTestHelper_ArrayEqualImpl<float>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, true, GetArrayTypeAsString,
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
		if (FDlgHelper::IsSetEqual<SetType>(ThisSet, OtherSet) == false)
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
		const FString& PropertyName, FString& OutError, const bool bIsValuePrimitive,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const ValueType&)> GetValueTypeAsString,
		std::function<bool(const ValueType& FirstValue, const ValueType& SecondValue, FString& ComparisonErrorMessage)> AreValuesEqual)
	{
		bool bIsEqual = true;
		auto AreValuesEqualWithoutMessage = [&AreValuesEqual](const ValueType& FirstValue, const ValueType& SecondValue) -> bool
		{
			FString DiscardMessage;
			return AreValuesEqual(FirstValue, SecondValue, DiscardMessage);
		};

		if (FDlgHelper::IsMapEqual<KeyType, ValueType>(ThisMap, OtherMap, AreValuesEqualWithoutMessage) == false)
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
					FString ComparisonErrorMessage;
					if (!AreValuesEqual(ThisElem.Value, *OtherMapValue, ComparisonErrorMessage))
					{
						if (bIsValuePrimitive)
						{
							ValuesThatDifferString += FString::Printf(
								TEXT("\tThis.%s[key] (%s) != Other.%s[key] (%s). Key = (%s)\n"),
								*PropertyName, *GetValueTypeAsString(ThisElem.Value),
								*PropertyName, *GetValueTypeAsString(*OtherMapValue),
								*GetKeyTypeAsString(ThisElem.Key));
						}
						else
						{
							ValuesThatDifferString += FString::Printf(
								TEXT("\tThis.%s[key] (%s) != Other.%s[key] (%s). Key = (%s). |Message = %s|\n"),
								*PropertyName, *GetValueTypeAsString(ThisElem.Value),
								*PropertyName, *GetValueTypeAsString(*OtherMapValue),
								*GetKeyTypeAsString(ThisElem.Key), *ComparisonErrorMessage);
						}
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
class FDlgTestHelper_MapPrimitiveVariantImpl
{
public:
	static bool IsEqual(const TMap<KeyType, ValueType>& ThisMap, const TMap<KeyType, ValueType>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const ValueType&)> GetValueTypeAsString)
	{
		return FDlgTestHelper_MapEqualImpl<KeyType, ValueType>::IsEqual(ThisMap, OtherMap, PropertyName, OutError, true, GetKeyTypeAsString, GetValueTypeAsString,
			[](const ValueType& FirstMapValue, const ValueType& SecondMapValue, FString& ComparisonErrorMessage) -> bool
		{
			return FirstMapValue == SecondMapValue;
		});
	}
};

// Variant for float
template <typename KeyType>
class FDlgTestHelper_MapPrimitiveVariantImpl<KeyType, float>
{
public:
	static bool IsEqual(const TMap<KeyType, float>& ThisMap, const TMap<KeyType, float>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const float&)> GetValueTypeAsString)
	{
		return FDlgTestHelper_MapEqualImpl<KeyType, float>::IsEqual(ThisMap, OtherMap, PropertyName, OutError, true, GetKeyTypeAsString, GetValueTypeAsString,
			[](const float& FirstMapValue, const float& SecondMapValue, FString& ComparisonErrorMessage) -> bool
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


	template <typename ValueType>
	static void CheckMapStringKeyInvariants(const TMap<FString, ValueType>& ThisMap)
	{
		for (const auto& Elem : ThisMap)
		{
			// Most likely the map is corrupted
			check((void*)(*Elem.Key) != nullptr);
			Elem.Key.CheckInvariants();
		}
	}

	template <typename SetType>
	static bool IsSetEqual(const TSet<SetType>& ThisSet, const TSet<SetType>& OtherSet,
		const FString& PropertyName, FString& OutError, std::function<FString(const SetType&)> GetSetTypeAsString)
	{
		return FDlgTestHelper_SetEqualImpl<SetType>::IsEqual(ThisSet, OtherSet, PropertyName, OutError, GetSetTypeAsString);
	}

	template <typename KeyType, typename ValueType>
	static bool IsPrimitiveMapEqual(const TMap<KeyType, ValueType>& ThisMap, const TMap<KeyType, ValueType>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const ValueType&)> GetValueTypeAsString)
	{
		return FDlgTestHelper_MapPrimitiveVariantImpl<KeyType, ValueType>::IsEqual(ThisMap, OtherMap, PropertyName, OutError, GetKeyTypeAsString, GetValueTypeAsString);
	}

	template <typename KeyType, typename ValueType>
	static bool IsComplexMapValueEqual(const TMap<KeyType, ValueType>& ThisMap, const TMap<KeyType, ValueType>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString,
		std::function<FString(const ValueType&)> GetValueTypeAsString)
	{
		return FDlgTestHelper_MapEqualImpl<KeyType, ValueType>::IsEqual(ThisMap, OtherMap, PropertyName, OutError, false, GetKeyTypeAsString, GetValueTypeAsString,
			[](const ValueType& FirstMapValue, const ValueType& SecondMapValue, FString& ComparisonErrorMessage) -> bool
			{
				return FirstMapValue.IsEqual(SecondMapValue, ComparisonErrorMessage);
			});
	}

	template <typename KeyType, typename ValueType>
	static bool IsComplexPointersMapEqual(const TMap<KeyType, ValueType*>& ThisMap, const TMap<KeyType, ValueType*>& OtherMap,
		const FString& PropertyName, FString& OutError,
		std::function<FString(const KeyType&)> GetKeyTypeAsString)
	{
		return FDlgTestHelper_MapEqualImpl<KeyType, ValueType*>::IsEqual(ThisMap, OtherMap, PropertyName, OutError, false, GetKeyTypeAsString,
			[](const auto* Object) -> FString
			{
				return FDlgHelper::GetFullNameFromObject(Object);
			},
			[](const auto* FirstMapValue, const auto* SecondMapValue, FString& ComparisonErrorMessage) -> bool
			{
				if (FirstMapValue == nullptr)
				{
					ComparisonErrorMessage = TEXT("FirstMapValue is nullptr");
					return SecondMapValue == nullptr;
				}
				return FirstMapValue->IsEqual(SecondMapValue, ComparisonErrorMessage);
			});
	}

	template <typename ArrayType>
	static bool IsPrimitiveArrayEqual(const TArray<ArrayType>& ThisArray, const TArray<ArrayType>& OtherArray,
		const FString& PropertyName, FString& OutError, std::function<FString(const ArrayType&)> GetArrayTypeAsString)
	{
		return FDlgTestHelper_ArrayPrimitiveVariantImpl<ArrayType>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, GetArrayTypeAsString);
	}

	template <typename ArrayType>
	static bool IsComplexArrayEqual(const TArray<ArrayType>& ThisArray, const TArray<ArrayType>& OtherArray,
		const FString& PropertyName, FString& OutError)
	{
		return FDlgTestHelper_ArrayEqualImpl<ArrayType>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, false,
			[](const ArrayType& Value) -> FString
			{
				return TEXT("IsComplexArrayEqual SHOULD NOT SEE THIS");
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
		return FDlgTestHelper_ArrayEqualImpl<ArrayType*>::IsEqual(ThisArray, OtherArray, PropertyName, OutError, false,
			[](const auto* Value) -> FString
			{
				return Value ? Value->ToString() : "";
			},
			[](const auto* FirstValue, const auto* SecondValue, FString& ComparisonErrorMessage) -> bool
			{
				if (FirstValue == nullptr)
				{
					ComparisonErrorMessage = TEXT("FirstValue is nullptr");
					return SecondValue == nullptr;
				}
				return FirstValue->IsEqual(SecondValue, ComparisonErrorMessage);
			});
	}

	static std::function<FString(const int32&)> Int32ToString;
	static std::function<FString(const int64&)> Int64ToString;
	static std::function<FString(const FName&)> NameToString;
	static std::function<FString(const FString&)> StringToString;
	static std::function<FString(const float&)> FloatToString;
	static std::function<FString(const FVector&)> VectorToString;
	static std::function<FString(const FColor&)> ColorToString;
};
