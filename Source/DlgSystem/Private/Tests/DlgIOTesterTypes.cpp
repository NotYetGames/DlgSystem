// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgIOTesterTypes.h"

// May the macro gods have mercy on me
#define TEST_ARRAY_IS_EQUAL_TO_OTHER(_PropertyArrayName, _DifferenceElements)                                  \
	if (_PropertyArrayName != Other._PropertyArrayName)                                                        \
	{                                                                                                          \
		bIsEqual = false;                                                                                      \
		if (_PropertyArrayName.Num() != Other._PropertyArrayName.Num())                                        \
		{                                                                                                      \
			OutError += FString::Printf(                                                                       \
				TEXT("\tThis." #_PropertyArrayName ".Num (%d) != Other." #_PropertyArrayName ".Num (%d)\n"),   \
				_PropertyArrayName.Num(), Other._PropertyArrayName.Num());                                     \
		}                                                                                                      \
		_DifferenceElements                                                                                    \
	}

#define TEST_SET_IS_EQUAL_TO_OTHER(_PropertySetName, _DifferenceElements)                                  \
	if (SetEqualsSet(_PropertySetName, Other._PropertySetName) == false)                                   \
	{                                                                                                      \
		bIsEqual = false;                                                                                  \
		if (_PropertySetName.Num() != Other._PropertySetName.Num())                                        \
		{                                                                                                  \
			OutError += FString::Printf(                                                                   \
				TEXT("\tThis." #_PropertySetName ".Num (%d) != Other." #_PropertySetName ".Num (%d)\n"),   \
				_PropertySetName.Num(), Other._PropertySetName.Num());                                     \
		}                                                                                                  \
		_DifferenceElements                                                                                \
	}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestStructPrimitives
void FDlgTestStructPrimitives::GenerateRandomData()
{
	bBoolean = FMath::RandBool();
	Integer = FMath::Rand();
	Float = FMath::SRand();
	String = FString::SanitizeFloat(Float);
	Name = FName(*String);
	Enum = static_cast<EDlgTestEnum>(FMath::RandHelper(static_cast<int32>(EDlgTestEnum::ETE_NumOf)));
	Text = FText::FromString(String);
}

bool FDlgTestStructPrimitives::IsEqual(const FDlgTestStructPrimitives& Other, FString& OutError) const
{
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestStructPrimitives::IsEqual:\n");
	if (bBoolean != Other.bBoolean)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.bBoolean (%d) != Other.bBoolean (%d)\n"), bBoolean, Other.bBoolean);
	}

	if (Integer != Other.Integer)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Integer (%d) != Other.Integer (%d)\n"), Integer, Other.Integer);
	}

	if (!FMath::IsNearlyEqual(Float, Other.Float, KINDA_SMALL_NUMBER))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Float (%f) != Other.Float (%f)\n"), Float, Other.Float);
	}

	if (String != Other.String)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.String (%s) != Other.String (%s)\n"), *String, *Other.String);
	}

	if (Name != Other.Name)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Name (%s) != Other.Name (%s)\n"), *Name.ToString(), *Other.Name.ToString());
	}

	if (!Text.EqualTo(Other.Text))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Text (%s) != Other.Text (%s)\n"), *Text.ToString(), *Other.Text.ToString());
	}

	if (Enum != Other.Enum)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Enum (%d) != Other.Enum (%d)\n"), static_cast<int32>(Enum), static_cast<int32>(Other.Enum));
	}

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestStructPrimitives::operator==(const FDlgTestStructPrimitives& Other) const
{
	return bBoolean == Other.bBoolean &&
		   Integer == Other.Integer &&
		   FMath::IsNearlyEqual(Float, Other.Float, KINDA_SMALL_NUMBER) &&
		   String == Other.String &&
		   Name == Other.Name &&
		   Text.EqualTo(Other.Text) &&
		   Enum == Other.Enum;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestArrayPrimitive
void FDlgTestArrayPrimitive::GenerateRandomData()
{
	const int32 Num = FMath::RandHelper(10) + 2;
	IntArray.SetNum(Num);
	BoolArray.SetNum(Num);
	if (Options.bSupportsPureEnumContainer)
	{
		EnumArray.SetNum(Num);
	}
	StringArray.SetNum(Num);
	NameArray.SetNum(Num);

	for (int32 i = 0; i < Num; ++i)
	{
		IntArray[i] = FMath::Rand();
		BoolArray[i] = FMath::RandBool();
		if (Options.bSupportsPureEnumContainer)
		{
			EnumArray[i] = static_cast<EDlgTestEnum>(FMath::RandHelper(static_cast<int32>(EDlgTestEnum::ETE_NumOf)));
		}
		StringArray[i] = FString::SanitizeFloat(FMath::SRand());
		NameArray[i] = FName(*StringArray[i]);
	}
}

bool FDlgTestArrayPrimitive::IsEqual(const FDlgTestArrayPrimitive& Other, FString& OutError) const
{
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestArrayPrimitive::IsEqual:\n");

	TEST_ARRAY_IS_EQUAL_TO_OTHER(IntArray,
	{
		// Some element is not equal
		for (int32 i = 0; i < IntArray.Num(); i++)
		{
			if (IntArray[i] != Other.IntArray[i])
			{
				OutError += FString::Printf(TEXT("\tThis.IntArray[%d] (%d) != Other.IntArray[%d] (%d)\n"),
					i, IntArray[i], i, Other.IntArray[i]);
			}
		}
	});

	TEST_ARRAY_IS_EQUAL_TO_OTHER(BoolArray,
	{
		// Some element is not equal
		for (int32 i = 0; i < BoolArray.Num(); i++)
		{
			if (BoolArray[i] != Other.BoolArray[i])
			{
				OutError += FString::Printf(TEXT("\tThis.BoolArray[%d] (%d) != Other.BoolArray[%d] (%d)\n"),
					i, BoolArray[i], i, Other.BoolArray[i]);
			}
		}
	});

	TEST_ARRAY_IS_EQUAL_TO_OTHER(EnumArray,
	{
		// Some element is not equal
		for (int32 i = 0; i < EnumArray.Num(); i++)
		{
			if (EnumArray[i] != Other.EnumArray[i])
			{
				OutError += FString::Printf(TEXT("\tThis.EnumArray[%d] (%d) != Other.EnumArray[%d] (%d)\n"),
					i, static_cast<int32>(EnumArray[i]), i, static_cast<int32>(Other.EnumArray[i]));
			}
		}
	});

	TEST_ARRAY_IS_EQUAL_TO_OTHER(NameArray,
	{
		// Some element is not equal
		for (int32 i = 0; i < NameArray.Num(); i++)
		{
			if (NameArray[i] != Other.NameArray[i])
			{
				OutError += FString::Printf(TEXT("\tThis.NameArray[%d] (%s) != Other.NameArray[%d] (%s)\n"),
					i, *NameArray[i].ToString(), i, *Other.NameArray[i].ToString());
			}
		}
	});


	TEST_ARRAY_IS_EQUAL_TO_OTHER(StringArray,
	{
		// Some element is not equal
		for (int32 i = 0; i < StringArray.Num(); i++)
		{
			if (StringArray[i] != Other.StringArray[i])
			{
				OutError += FString::Printf(TEXT("\tThis.StringArray[%d] (%s) != Other.StringArray[%d] (%s)\n"),
					i, *StringArray[i], i, *Other.StringArray[i]);
			}
		}
	});

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestArrayPrimitive::operator==(const FDlgTestArrayPrimitive& Other) const
{
	return IntArray == Other.IntArray &&
		   BoolArray == Other.BoolArray &&
		   EnumArray == Other.EnumArray &&
		   NameArray == Other.NameArray &&
		   StringArray == Other.StringArray;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestArrayStruct
void FDlgTestArrayStruct::GenerateRandomData()
{
	StructArrayPrimitives.SetNum(FMath::RandHelper(10) + 2);
}

bool FDlgTestArrayStruct::IsEqual(const FDlgTestArrayStruct& Other, FString& OutError) const
{
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestArrayStruct::IsEqual:\n");

	TEST_ARRAY_IS_EQUAL_TO_OTHER(StructArrayPrimitives,
	{
		// Some element is not equal
		for (int32 i = 0; i < StructArrayPrimitives.Num(); i++)
		{
			FString ErrorMessage;
			if (!StructArrayPrimitives[i].IsEqual(Other.StructArrayPrimitives[i], ErrorMessage))
			{
				OutError += FString::Printf(TEXT("\tThis.StructArrayPrimitives[%d] != Other.StructArrayPrimitives[%d] |message = %s|\n"),
					i, i, *ErrorMessage);
			}
		}
	});

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestArrayStruct::operator==(const FDlgTestArrayStruct& Other) const
{
	return StructArrayPrimitives == Other.StructArrayPrimitives;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestSetPrimitive
void FDlgTestSetPrimitive::GenerateRandomData()
{
	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		IntSet.Add(FMath::Rand());
		BoolSet.Add(FMath::RandBool());
		if (Options.bSupportsPureEnumContainer)
		{
			EnumSet.Add(static_cast<EDlgTestEnum>(FMath::RandHelper(static_cast<int32>(EDlgTestEnum::ETE_NumOf))));
		}
		const FString String = FString::SanitizeFloat(FMath::SRand());
		StringSet.Add(String);
		NameSet.Add(FName(*String));
	}
}

bool FDlgTestSetPrimitive::IsEqual(const FDlgTestSetPrimitive& Other, FString& OutError) const
{
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestSetPrimitive::IsEqual:\n");

	TEST_SET_IS_EQUAL_TO_OTHER(IntSet,
	{
		const TSet<int32> NotInOther = IntSet.Difference(Other.IntSet);
		OutError += FString::Printf(TEXT("\tNotInOther = This.IntSet - Other.IntSet is of length = %d\n"), NotInOther.Num());
		FString NotInOtherString;
		for (const int32 Number : NotInOther)
		{
			NotInOtherString += FString::Printf(TEXT("%d,"), Number);
		}
		OutError += FString::Printf(TEXT("\tNotInOther = {%s}\n"), *NotInOtherString);
	});

	TEST_SET_IS_EQUAL_TO_OTHER(BoolSet,
	{
		const TSet<bool> NotInOther = BoolSet.Difference(Other.BoolSet);
		OutError += FString::Printf(TEXT("\tNotInOther = This.BoolSet - Other.BoolSet is of length = %d\n"), NotInOther.Num());
		FString NotInOtherString;
		for (const bool Boolean : NotInOther)
		{
			NotInOtherString += FString::Printf(TEXT("%d,"), Boolean);
		}
		OutError += FString::Printf(TEXT("\tNotInOther = {%s}\n"), *NotInOtherString);
	});

	TEST_SET_IS_EQUAL_TO_OTHER(EnumSet,
	{
		const TSet<EDlgTestEnum> NotInOther = EnumSet.Difference(Other.EnumSet);
		OutError += FString::Printf(TEXT("\tNotInOther = This.EnumSet - Other.EnumSet is of length = %d\n"), NotInOther.Num());
		FString NotInOtherString;
		for (const EDlgTestEnum Enum : NotInOther)
		{
			NotInOtherString += FString::Printf(TEXT("%d,"), static_cast<int32>(Enum));
		}
		OutError += FString::Printf(TEXT("\tNotInOther = {%s}\n"), *NotInOtherString);
	});

	TEST_SET_IS_EQUAL_TO_OTHER(NameSet,
	{
		const TSet<FName> NotInOther = NameSet.Difference(Other.NameSet);
		OutError += FString::Printf(TEXT("\tNotInOther = This.NameSet - Other.NameSet is of length = %d\n"), NotInOther.Num());
		FString NotInOtherString;
		for (const FName Name : NotInOther)
		{
			NotInOtherString += FString::Printf(TEXT("`%s`,"), *Name.ToString());
		}
		OutError += FString::Printf(TEXT("\tNotInOther = {%s}\n"), *NotInOtherString);
	});

	TEST_SET_IS_EQUAL_TO_OTHER(StringSet,
	{
		const TSet<FString> NotInOther = StringSet.Difference(Other.StringSet);
		OutError += FString::Printf(TEXT("\tNotInOther = This.StringSet - Other.StringSet is of length = %d\n"), NotInOther.Num());
		FString NotInOtherString;
		for (const FString& String : NotInOther)
		{
			NotInOtherString += FString::Printf(TEXT("`%s`,"), *String);
		}
		OutError += FString::Printf(TEXT("\tNotInOther = {%s}\n"), *NotInOtherString);
	});

	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestSetPrimitive::operator==(const FDlgTestSetPrimitive& Other) const
{
	return SetEqualsSet(IntSet, Other.IntSet) &&
		   SetEqualsSet(BoolSet, Other.BoolSet) &&
		   SetEqualsSet(EnumSet, Other.EnumSet) &&
		   SetEqualsSet(NameSet, Other.NameSet) &&
		   SetEqualsSet(StringSet, Other.StringSet);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestSetStruct
void FDlgTestSetStruct::GenerateRandomData()
{
	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		StructSetPrimitives.Add({});
	}
}

bool FDlgTestSetStruct::IsEqual(const FDlgTestSetStruct& Other, FString& OutError) const
{
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestSetStruct::IsEqual:\n");

	TEST_SET_IS_EQUAL_TO_OTHER(StructSetPrimitives,
	{
		const TSet<FDlgTestStructPrimitives> NotInOther = StructSetPrimitives.Difference(Other.StructSetPrimitives);
		OutError += FString::Printf(TEXT("\tNotInOther = This.StructSetPrimitives - Other.StructSetPrimitives is of length = %d\n"), NotInOther.Num());
		FString NotInOtherString;
		for (const FDlgTestStructPrimitives& Struct : NotInOther)
		{
			NotInOtherString += FString::Printf(TEXT("Struct(%s),\n"), *Struct.ToString());
		}
		OutError += FString::Printf(TEXT("\tNotInOther = {%s}\n"), *NotInOtherString);
	});

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestSetStruct::operator==(const FDlgTestSetStruct& Other) const
{
	return SetEqualsSet(StructSetPrimitives, Other.StructSetPrimitives);
}


#undef TEST_ARRAY_IS_EQUAL
#undef TEST_SET_IS_EQUAL_TO_OTHER
