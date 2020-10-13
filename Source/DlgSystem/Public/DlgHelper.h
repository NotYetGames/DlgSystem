// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Object.h"
#include "UObject/UnrealType.h"
#include "UObject/ObjectMacros.h"
#include <functional>



#include "DlgCondition.h"
#include "DlgEvent.h"
#include "NYReflectionTypes.h"

#include "DlgHelper.generated.h"

class UDlgSystemSettings;


USTRUCT()
struct FDlgClassAndObject
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY()
	UClass* Class = nullptr;

	UPROPERTY()
	UObject* Object = nullptr;
};

// Const version of FScriptArrayHelper
class FDlgConstScriptArrayHelper : public FScriptArrayHelper
{
	typedef FScriptArrayHelper Super;
	typedef FDlgConstScriptArrayHelper Self;
public:
	FORCEINLINE FDlgConstScriptArrayHelper(const FNYArrayProperty* InProperty, const void *InArray)
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

	FORCEINLINE FDlgConstScriptMapHelper(const FNYMapProperty* InProperty, const void* InMap)
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
	static bool IsEqual(
		const TMap<KeyType, ValueType>& FirstMap,
		const TMap<KeyType, ValueType>& SecondMap,
		std::function<bool(const ValueType& FirstMapValue,
		const ValueType& SecondMapValue)> AreValuesEqual
	)
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
	static bool IsEqual(
		const TArray<ArrayType>& FirstArray,
		const TArray<ArrayType>& SecondArray,
		std::function<bool(const ArrayType& FirstValue,
		const ArrayType& SecondValue)> AreValuesEqual
	)
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
	FORCEINLINE static int64 RandomInt64() { return static_cast<int64>(FMath::Rand()) << 32 | FMath::Rand(); }
	FORCEINLINE static bool IsFloatEqual(const float A, const float B) { return FMath::IsNearlyEqual(A, B, KINDA_SMALL_NUMBER); }
	FORCEINLINE static bool IsPathInProjectDirectory(const FString& Path) { return Path.StartsWith("/Game");  }

	static FString GetFullNameFromObject(const UObject* Object)
	{
		if (!IsValid(Object))
		{
			return TEXT("nullptr");
		}
		return Object->GetFullName();
	}

	static FString GetClassNameFromObject(const UObject* Object)
	{
		if (!IsValid(Object))
		{
			return TEXT("INVALID CLASS");
		}
		return Object->GetClass()->GetName();
	}

	// Removes _C from the end of the Name
	// And removes the .extension from the path names
	static FString CleanObjectName(FString Name);

	// Blueprint Helpers
	static bool IsClassIgnored(const UClass* Class);
	static bool IsABlueprintClass(const UClass* Class);
	static bool IsABlueprintObject(const UObject* Object);

	// This also works with Blueprints
	static bool IsObjectAChildOf(const UObject* Object, const UClass* ParentClass);

	// This also works with Blueprints
	static bool IsObjectImplementingInterface(const UObject* Object, const UClass* InterfaceClass);

	// Gets all child classes of ParentClass
	// NOTE: this is super slow, use with care
	static bool GetAllChildClassesOf(const UClass* ParentClass, TArray<UClass*>& OutNativeClasses, TArray<UClass*>& OutBlueprintClasses);

	// Gets all classes that implement InterfaceClass
	// NOTE: this is super slow, use with care
	static bool GetAllClassesImplementingInterface(const UClass* InterfaceClass, TArray<UClass*>& OutNativeClasses, TArray<UClass*>& OutBlueprintClasses);

	// Converts the Classes Array that represent the Dialogue Participants into a map where
	// Key: The participant Name
	// Value: An array of structs that contain the class and the object for that participant name
	static TMap<FName, TArray<FDlgClassAndObject>> ConvertDialogueParticipantsClassesIntoMap(const TArray<UClass*>& Classes);

	// FileSystem
	static bool DeleteFile(const FString& PathName, bool bVerbose = true);
	static bool RenameFile(const FString& OldPathName, const FString& NewPathName, bool bOverWrite = false, bool bVerbose = true);

	// Get the Interface Function Name for this Event Type
	static FName GetFunctionNameForEventType(EDlgEventType EventType)
	{
		switch (EventType)
		{
		case EDlgEventType::Event:
			return TEXT("OnDialogueEvent");
		case EDlgEventType::ModifyBool:
			return TEXT("ModifyBoolValue");
		case EDlgEventType::ModifyFloat:
			return TEXT("ModifyFloatValue");
		case EDlgEventType::ModifyInt:
			return TEXT("ModifyIntValue");
		case EDlgEventType::ModifyName:
			return TEXT("ModifyIntValue");
		default:
			break;
		}

		return NAME_None;
	}

	static FName GetFunctionNameForConditionType(EDlgConditionType ConditionType)
	{
		switch (ConditionType)
		{
		case EDlgConditionType::EventCall:
			return TEXT("CheckCondition");
		case EDlgConditionType::BoolCall:
			return TEXT("GetBoolValue");
		case EDlgConditionType::FloatCall:
			return TEXT("GetFloatValue");
		case EDlgConditionType::IntCall:
			return TEXT("GetIntValue");
		case EDlgConditionType::NameCall:
			return TEXT("GetNameValue");
		default:
			break;
		}

		return NAME_None;
	}

	template<typename TEnum>
    static bool ConvertEnumToString(const FString& EnumName, TEnum EnumValue, bool bWithNameSpace, FString& OutEnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
		if (!EnumPtr)
		{
			OutEnumValue = FString::Printf(TEXT("INVALID EnumName = `%s`"), *EnumName);
			return false;
		}

		OutEnumValue = bWithNameSpace ? EnumPtr->GetNameByIndex(static_cast<int32>(EnumValue)).ToString()
                                      : EnumPtr->GetNameStringByIndex(static_cast<int32>(EnumValue));
		return true;
	}

	template<typename TEnum>
    static bool ConvertStringToEnum(const FString& String, const FString& EnumName, TEnum& OutEnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
		if (!EnumPtr)
		{
			return false;
		}

		const int32 Index = EnumPtr->GetIndexByNameString(String);
		OutEnumValue = static_cast<TEnum>(Index);
		return true;
	}

	template<typename TEnum>
    static bool ConvertFNameToEnum(FName Name, const FString& EnumName, TEnum& OutEnumValue)
	{
		const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
		if (!EnumPtr)
		{
			return false;
		}

		const int32 Index = EnumPtr->GetIndexByName(Name);
		OutEnumValue = static_cast<TEnum>(Index);
		return true;
	}

	// Gets the first element from a set. From https://answers.unrealengine.com/questions/332443/how-to-get-the-firstonly-element-in-tset.html
	template <typename SetType>
	static typename TCopyQualifiersFromTo<SetType, typename SetType::ElementType>::Type* GetFirstSetElement(SetType& Set)
	{
		for (auto& Element : Set)
		{
			return &Element;
		}

		return nullptr;
	}

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
	static bool PredicateSortFNameAlphabeticallyAscending(FName A, FName B)
	{
		return A.Compare(B) < 0;
	}

	/** Default sorting function used by all the Dialogue related methods. Sorts alphabetically ascending. */
	static void SortDefault(TArray<FName>& OutArray)
    {
		OutArray.Sort(PredicateSortFNameAlphabeticallyAscending);
    }
	static void SortDefault(TSet<FName>& OutSet)
	{
		OutSet.Sort(PredicateSortFNameAlphabeticallyAscending);
	}

	template<typename ValueType>
	static void SortDefault(TMap<FName, ValueType>& Map)
	{
		Map.KeySort(PredicateSortFNameAlphabeticallyAscending);
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

		const UPTRINT PtrVal = (UPTRINT)Ptr;
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
