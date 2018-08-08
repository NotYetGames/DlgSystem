// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Object.h"
#include "UObject/UnrealType.h"
#include <functional>


// Const version of FScriptArrayHelper
class FDlgConstScriptArrayHelper : public FScriptArrayHelper
{
	typedef FScriptArrayHelper Super;
	typedef FDlgConstScriptArrayHelper Self;
public:
	FORCEINLINE FDlgConstScriptArrayHelper(const UArrayProperty* InProperty, const void *InArray)
		: Super(InProperty, InArray) {}

	FORCEINLINE const uint8* GetConstRawPtr(int32 Index = 0) const
	{
		return const_cast<Self*>(this)->GetRawPtr(Index);
	}
};


// Const version of FScriptMapHelper
class FDlgConstScriptMapHelper : public FScriptMapHelper
{
	typedef FScriptMapHelper Super;
	typedef FDlgConstScriptMapHelper Self;
public:

	FORCEINLINE FDlgConstScriptMapHelper(const UMapProperty* InProperty, const void* InMap)
		: Super(InProperty, InMap) {}


	FORCEINLINE const uint8* GetConstKeyPtr(int32 Index) const
	{
		return const_cast<Self*>(this)->GetKeyPtr(Index);
	}

	FORCEINLINE const uint8* GetConstValuePtr(int32 Index) const
	{
		return const_cast<Self*>(this)->GetValuePtr(Index);
	}
};


/**
 * Classes created because Function templates cannot be partially specialised. so we use a delegate class trick
 * https://stackoverflow.com/questions/16154480/getting-illegal-use-of-explicit-template-arguments-when-doing-a-pointer-partia
 */
template <typename KeyType, typename ValueType>
class FDlgHelper_MapEqualImpl
{
public:
	static bool IsEqual(const TMap<KeyType, ValueType>& FirstMap, const TMap<KeyType, ValueType>& SecondMap,
		std::function<bool(const ValueType& FirstMapValue, const ValueType& SecondMapValue)> AreValuesEqual)
	{
		if (FirstMap.Num() == SecondMap.Num())
		{
			for (const auto& ElemFirstMap : FirstMap)
			{
				const auto* FoundValueSecondMap = SecondMap.Find(ElemFirstMap.Key);
				if (FoundValueSecondMap != nullptr)
				{
					// Key exists in second map
					if (!AreValuesEqual(ElemFirstMap.Value, *FoundValueSecondMap))
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
class FDlgHelper_MapEqualVariantImpl
{
public:
	static bool IsEqual(const TMap<KeyType, ValueType>& FirstMap, const TMap<KeyType, ValueType>& SecondMap)
	{
		return FDlgHelper_MapEqualImpl<KeyType, ValueType>::IsEqual(FirstMap, SecondMap,
			[](const ValueType& FirstMapValue, const ValueType& SecondMapValue) -> bool
		{
			return FirstMapValue == SecondMapValue;
		});
	}
};

// Variant with Specialization for float ValueType
template <typename KeyType>
class FDlgHelper_MapEqualVariantImpl<KeyType, float>
{
public:
	static bool IsEqual(const TMap<KeyType, float>& FirstMap, const TMap<KeyType, float>& SecondMap)
	{
		return FDlgHelper_MapEqualImpl<KeyType, float>::IsEqual(FirstMap, SecondMap,
			[](const float& FirstMapValue, const float& SecondMapValue) -> bool
		{
			return FMath::IsNearlyEqual(FirstMapValue, SecondMapValue, KINDA_SMALL_NUMBER);
		});
	}
};


template <typename ArrayType>
class FDlgHelper_ArrayEqualImpl
{
public:
	static bool IsEqual(const TArray<ArrayType>& FirstArray, const TArray<ArrayType>& SecondArray,
		std::function<bool(const ArrayType& FirstValue, const ArrayType& SecondValue)> AreValuesEqual)
	{
		if (FirstArray.Num() == SecondArray.Num())
		{
			// Some value is not equal
			for (int32 Index = 0; Index < FirstArray.Num(); Index++)
			{
				if (!AreValuesEqual(FirstArray[Index], SecondArray[Index]))
				{
					return false;
				}
			}

			return true;
		}

		// length differs
		return false;
	}
};

// Variant with default comparison
template <typename ArrayType>
class FDlgHelper_ArrayEqualVariantImpl
{
public:
	static bool IsEqual(const TArray<ArrayType>& FirstArray, const TArray<ArrayType>& SecondArray)
	{
		return FDlgHelper_ArrayEqualImpl<ArrayType>::IsEqual(FirstArray	, SecondArray,
			[](const ArrayType& FirstValue, const ArrayType& SecondValue) -> bool
		{
			return FirstValue == SecondValue;
		});
	}
};

// Variant with Specialization for float ArrayType
template <>
class FDlgHelper_ArrayEqualVariantImpl<float>
{
public:
	static bool IsEqual(const TArray<float>& FirstArray, const TArray<float>& SecondArray)
	{
		return FDlgHelper_ArrayEqualImpl<float>::IsEqual(FirstArray, SecondArray,
			[](const float& FirstValue, const float& SecondValue) -> bool
		{
			return FMath::IsNearlyEqual(FirstValue, SecondValue, KINDA_SMALL_NUMBER);
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
	// NOTE for SetType = float this won't work, what are you even doing?
	template <typename SetType>
	static bool IsSetEqual(const TSet<SetType>& FirstSet, const TSet<SetType>& SecondSet)
	{
		if (FirstSet.Num() == SecondSet.Num())
		{
			// No duplicates should be found
			TSet<SetType> Set = FirstSet;
			Set.Append(SecondSet);

			return Set.Num() == FirstSet.Num();
		}

		return false;
	}

	// Is FirstArray == SecondArray ?
	template <typename ArrayType>
	static bool IsArrayEqual(const TArray<ArrayType>& FirstArray, const TArray<ArrayType>& SecondArray,
		std::function<bool(const ArrayType& FirstValue, const ArrayType& SecondValue)> AreValuesEqual)
	{
		return FDlgHelper_ArrayEqualImpl<ArrayType>::IsEqual(FirstArray, SecondArray, AreValuesEqual);
	}

	// Variant with default comparison
	template <typename ArrayType>
	static bool IsArrayEqual(const TArray<ArrayType>& FirstArray, const TArray<ArrayType>& SecondArray)
	{
		return FDlgHelper_ArrayEqualVariantImpl<ArrayType>::IsEqual(FirstArray, SecondArray);
	}

	// Variant with pointer value comparison
	template <typename ArrayType>
	static bool IsArrayOfPointersEqual(const TArray<ArrayType*>& FirstArray, const TArray<ArrayType*>& SecondArray)
	{
		return FDlgHelper_ArrayEqualImpl<ArrayType*>::IsEqual(FirstArray, SecondArray,
			[](const auto* FirstValue, const auto* SecondValue) -> bool
			{
				if (FirstValue == nullptr)
				{
					return SecondValue == nullptr;
				}
				return *FirstValue == *SecondValue;
			});
	}

	// Is FirstMap == SecondMap ?
	template <typename KeyType, typename ValueType>
	static bool IsMapEqual(const TMap<KeyType, ValueType>& FirstMap, const TMap<KeyType, ValueType>& SecondMap,
		std::function<bool(const ValueType& FirstMapValue, const ValueType& SecondMapValue)> AreValuesEqual)
	{
		return FDlgHelper_MapEqualImpl<KeyType, ValueType>::IsEqual(FirstMap, SecondMap, AreValuesEqual);
	}

	// Variant with default comparison
	template <typename KeyType, typename ValueType>
	static bool IsMapEqual(const TMap<KeyType, ValueType>& FirstMap, const TMap<KeyType, ValueType>& SecondMap)
	{
		return FDlgHelper_MapEqualVariantImpl<KeyType, ValueType>::IsEqual(FirstMap, SecondMap);
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

	FORCEINLINE static bool IsPossiblyAllocatedUObjectPointer(const void* Ptr)
	{
		auto CountByteValues = [](UPTRINT Val, UPTRINT ByteVal) -> int32
		{
			int32 Result = 0;

			for (int32 I = 0; I != sizeof(UPTRINT); ++I)
			{
				if ((Val & 0xFF) == ByteVal)
				{
					++Result;
				}
				Val >>= 8;
			}

			return Result;
		};

		UPTRINT PtrVal = (UPTRINT)Ptr;
		// (void*) - 1 Is technically a valid address 0xffffffff, but is it?
		return Ptr != nullptr && Ptr != ((void*) - 1) && PtrVal >= 0x1000 && CountByteValues(PtrVal, 0xCD) < sizeof(UPTRINT) / 2;
	}

	/** Very low level and safe way to check if the object is valid. Kinda slow but safe. */
	static bool IsValidLowLevel(const UObject* Object)
	{
		if (Object == nullptr)
		{
			return false;
		}

		// From IsValidLowLevelFast
		// Check pointer range, may the gods have mercy
		if (!::IsPossiblyAllocatedUObjectPointer(const_cast<UObject*>(Object)))
		{
			return false;
		}

		// As DEFAULT_ALIGNMENT is defined to 0 now, I changed that to the original numerical value here
		static const int32 AlignmentCheck = MIN_ALIGNMENT - 1;
		if ((UPTRINT)Object & AlignmentCheck)
		{
			return false;
		}
		//if (!FDlgHelper::IsPossiblyAllocatedUObjectPointer((void**)Object) || !FDlgHelper::IsPossiblyAllocatedUObjectPointer(*(void**)Object))
		//{
		//	return false;
		//}

		// Virtual functions table is invalid.
		if ((void**)Object == nullptr || *(void**)Object == nullptr)
		{
			return false;
		}

		if (!Object->IsValidLowLevelFast())
		{
			return false;
		}

		return !Object->IsPendingKillOrUnreachable();
	}
};
