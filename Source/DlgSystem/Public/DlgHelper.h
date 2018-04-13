// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include <functional>

/**
 * Classes created because Function templates cannot be partially specialised. so we use a delegate class trick
 * https://stackoverflow.com/questions/16154480/getting-illegal-use-of-explicit-template-arguments-when-doing-a-pointer-partia
 */
template <typename KeyType, typename ValueType>
class FDlgHelper_MapEqualImpl
{
public:
	static bool AreMapsEqual(const TMap<KeyType, ValueType>& FirstMap, const TMap<KeyType, ValueType>& SecondMap,
		std::function<bool(const ValueType& FirstMapValue, const ValueType& SecondMapValue)> AreValuesEqual)
	{
		if (FirstMap.Num() == SecondMap.Num())
		{
			for (const auto& ElemFirstMap : FirstMap)
			{
				const ValueType* FoundValueSecondMap = SecondMap.Find(ElemFirstMap.Key);
				if (FoundValueSecondMap != nullptr)
				{
					// Key exists in second map
					if (!AreValuesEqual(*FoundValueSecondMap, ElemFirstMap.Value))
					{
						// Value differs
						return false;
					}
				}
				else
				{
					// Key does not even exist
					return false;
				}
			}

			return true;
		}

		// Length differs
		return false;
	}
};

// Variant with default comparison
template <typename KeyType, typename ValueType>
class FDlgHelper_MapVariantImpl
{
public:
	static bool AreMapsEqual(const TMap<KeyType, ValueType>& FirstMap, const TMap<KeyType, ValueType>& SecondMap)
	{
		return FDlgHelper_MapEqualImpl<KeyType, ValueType>::AreMapsEqual(FirstMap, SecondMap,
			[](const ValueType& FirstMapValue, const ValueType& SecondMapValue) -> bool
		{
			return FirstMapValue == SecondMapValue;
		});
	}
};

// Specialization for float ValueType
template <typename KeyType>
class FDlgHelper_MapVariantImpl<KeyType, float>
{
public:
	static bool AreMapsEqual(const TMap<KeyType, float>& FirstMap, const TMap<KeyType, float>& SecondMap)
	{
		return FDlgHelper_MapEqualImpl<KeyType, float>::AreMapsEqual(FirstMap, SecondMap,
			[](const float& FirstMapValue, const float& SecondMapValue) -> bool
		{
			return FMath::IsNearlyEqual(FirstMapValue, SecondMapValue, KINDA_SMALL_NUMBER);
		});
	}
};


/**
 * General helper methods
 */
class DLGSYSTEM_API FDlgHelper
{
	typedef FDlgHelper Self;
public:
	// Is FirstSet == SecondSet
	template <typename Type>
	static bool AreSetsEqual(const TSet<Type>& FirstSet, const TSet<Type>& SecondSet)
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

	// Is FirstMap == SecondMap ?
	template <typename KeyType, typename ValueType>
	static bool AreMapsEqual(const TMap<KeyType, ValueType>& FirstMap, const TMap<KeyType, ValueType>& SecondMap,
		std::function<bool(const ValueType& FirstMapValue, const ValueType& SecondMapValue)> AreValuesEqual)
	{
		return FDlgHelper_MapEqualImpl<KeyType, ValueType>::AreMapsEqual(FirstMap, SecondMap, AreValuesEqual);
	}


	// Variant with default comparison
	template <typename KeyType, typename ValueType>
	static bool AreMapsEqual(const TMap<KeyType, ValueType>& FirstMap, const TMap<KeyType, ValueType>& SecondMap)
	{
		return FDlgHelper_MapVariantImpl<KeyType, ValueType>::AreMapsEqual(FirstMap, SecondMap);
	}

	// Default comparison function
	static bool PredicateSortFNameAlphabeticallyAscending(const FName& A, const FName& B)
	{
		return A.Compare(B) < 0;
	}

	/** Default sorting function used by all the Dialogue related methods. Sorts alphabetically ascending. */
	static void SortDefault(TArray<FName>& OutArray)
    {
		OutArray.Sort(Self::PredicateSortFNameAlphabeticallyAscending);
    }
	static void SortDefault(TSet<FName>& OutSet)
	{
		OutSet.Sort(Self::PredicateSortFNameAlphabeticallyAscending);
	}

	template<typename ValueType>
	static void SortDefault(TMap<FName, ValueType>& Map)
	{
		Map.KeySort(Self::PredicateSortFNameAlphabeticallyAscending);
	}

	/** Helper method, used to append a set to an array. Also sort. */
	static void AppendSortedSetToArray(const TSet<FName>& InSet, TArray<FName>& OutArray)
	{
		TArray<FName> UniqueNamesArray = InSet.Array();
		SortDefault(UniqueNamesArray);
		OutArray.Append(UniqueNamesArray);
	}
};
