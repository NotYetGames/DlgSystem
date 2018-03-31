// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgIOTesterTypes.h"

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
	IntArray.Empty();
	BoolArray.Empty();
	EnumArray.Empty();
	NameArray.Empty();
	StringArray.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	IntArray.SetNum(Num);
	BoolArray.SetNum(Num);
	if (Options.bSupportsPureEnumContainer)
	{
		EnumArray.SetNum(Num);
	}
	NameArray.SetNum(Num);
	StringArray.SetNum(Num);

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
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestArrayPrimitive::IsEqual:\n");

	PropertyName = TEXT("IntArray");
	bIsEqual = bIsEqual && TestArrayIsEqualToOther<int32>(IntArray, Other.IntArray, PropertyName, OutError,
		[&PropertyName](const int32 Index, const int32& ThisValue, const int32& OtherValue) -> FString
		{
			if (ThisValue != OtherValue)
			{
				return FString::Printf(TEXT("\tThis.%s[%d] (%d) != Other.%s[%d] (%d)\n"),
					*PropertyName, Index, ThisValue, *PropertyName, Index, OtherValue);
			}
			return FString();
		});


	PropertyName = TEXT("BoolArray");
	bIsEqual = bIsEqual && TestArrayIsEqualToOther<bool>(BoolArray, Other.BoolArray, PropertyName, OutError,
		[&PropertyName](const int32 Index, const bool& ThisValue, const bool& OtherValue) -> FString
		{
			if (ThisValue != OtherValue)
			{
				return FString::Printf(TEXT("\tThis.%s[%d] (%d) != Other.%s[%d] (%d)\n"),
					*PropertyName, Index, ThisValue, *PropertyName, Index, OtherValue);
			}
			return FString();
		});

	PropertyName = TEXT("EnumArray");
	bIsEqual = bIsEqual && TestArrayIsEqualToOther<EDlgTestEnum>(EnumArray, Other.EnumArray, PropertyName, OutError,
		[&PropertyName](const int32 Index, const EDlgTestEnum& ThisValue, const EDlgTestEnum& OtherValue) -> FString
		{
			if (ThisValue != OtherValue)
			{
				return FString::Printf(TEXT("\tThis.%s[%d] (%d) != Other.%s[%d] (%d)\n"),
					*PropertyName, Index, static_cast<int32>(ThisValue), *PropertyName, Index, static_cast<int32>(OtherValue));
			}
			return FString();
		});

	PropertyName = TEXT("NameArray");
	bIsEqual = bIsEqual && TestArrayIsEqualToOther<FName>(NameArray, Other.NameArray, PropertyName, OutError,
		[&PropertyName](const int32 Index, const FName& ThisValue, const FName& OtherValue) -> FString
		{
			if (ThisValue != OtherValue)
			{
				return FString::Printf(TEXT("\tThis.%s[%d] (%s) != Other.%s[%d] (%s)\n"),
					*PropertyName, Index, *ThisValue.ToString(), *PropertyName, Index, *OtherValue.ToString());
			}
			return FString();
		});

	PropertyName = TEXT("StringArray");
	bIsEqual = bIsEqual && TestArrayIsEqualToOther<FString>(StringArray, Other.StringArray, PropertyName, OutError,
		[&PropertyName](const int32 Index, const FString& ThisValue, const FString& OtherValue) -> FString
		{
			if (ThisValue != OtherValue)
			{
				return FString::Printf(TEXT("\tThis.%s[%d] (%s) != Other.%s[%d] (%s)\n"),
					*PropertyName, Index, *ThisValue, *PropertyName, Index, *OtherValue);
			}
			return FString();
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
	StructArrayPrimitives.Empty();
	StructArrayPrimitives.SetNum(FMath::RandHelper(10) + 2);
}

bool FDlgTestArrayStruct::IsEqual(const FDlgTestArrayStruct& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestArrayStruct::IsEqual:\n");

	PropertyName = TEXT("StructArrayPrimitives");
	bIsEqual = bIsEqual && TestArrayIsEqualToOther<FDlgTestStructPrimitives>(StructArrayPrimitives, Other.StructArrayPrimitives, PropertyName, OutError,
		[&PropertyName](const int32 Index, const FDlgTestStructPrimitives& ThisValue, const FDlgTestStructPrimitives& OtherValue) -> FString
		{
			FString ErrorMessage;
			if (!ThisValue.IsEqual(OtherValue, ErrorMessage))
			{
				return FString::Printf(TEXT("\tThis.%s[%d] != Other.%s[%d] |message = %s|\n"),
					*PropertyName, Index, *PropertyName, Index, *ErrorMessage);
			}
			return FString();
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
	IntSet.Empty();
	EnumSet.Empty();
	NameSet.Empty();
	StringSet.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		IntSet.Add(FMath::Rand());
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
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestSetPrimitive::IsEqual:\n");

	PropertyName = TEXT("IntSet");
	bIsEqual = bIsEqual && TestSetIsEqualToOther<int32>(IntSet, Other.IntSet, PropertyName, OutError,
		[](const int32& Value) -> FString
		{
			return FString::FromInt(Value);
		});

	PropertyName = TEXT("EnumSet");
	bIsEqual = bIsEqual && TestSetIsEqualToOther<EDlgTestEnum>(EnumSet, Other.EnumSet, PropertyName, OutError,
		[](const EDlgTestEnum& Value) -> FString
		{
			return FString::FromInt(static_cast<int32>(Value));
		});

	PropertyName = TEXT("NameSet");
	bIsEqual = bIsEqual && TestSetIsEqualToOther<FName>(NameSet, Other.NameSet, PropertyName, OutError,
		[](const FName& Value) -> FString
		{
			return Value.ToString();
		});

	PropertyName = TEXT("StringSet");
	bIsEqual = bIsEqual && TestSetIsEqualToOther<FString>(StringSet, Other.StringSet, PropertyName, OutError,
		[](const FString& Value) -> FString
		{
			return Value;
		});

	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestSetPrimitive::operator==(const FDlgTestSetPrimitive& Other) const
{
	return FDlgHelper::AreSetsEqual(IntSet, Other.IntSet) &&
		   FDlgHelper::AreSetsEqual(EnumSet, Other.EnumSet) &&
		   FDlgHelper::AreSetsEqual(NameSet, Other.NameSet) &&
		   FDlgHelper::AreSetsEqual(StringSet, Other.StringSet);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestSetStruct
void FDlgTestSetStruct::GenerateRandomData()
{
	StructSetPrimitives.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		StructSetPrimitives.Add(FDlgTestStructPrimitives(Options));
	}
}

bool FDlgTestSetStruct::IsEqual(const FDlgTestSetStruct& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestSetStruct::IsEqual:\n");

	PropertyName = TEXT("StructSetPrimitives");
	bIsEqual = bIsEqual && TestSetIsEqualToOther<FDlgTestStructPrimitives>(StructSetPrimitives, Other.StructSetPrimitives, PropertyName, OutError,
		[](const FDlgTestStructPrimitives& Value) -> FString
		{
			return FString::Printf(TEXT("Struct(%s)"), *Value.ToString());
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
	return FDlgHelper::AreSetsEqual(StructSetPrimitives, Other.StructSetPrimitives);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestMapPrimitive
void FDlgTestMapPrimitive::GenerateRandomData()
{
	IntToIntMap.Empty();
	IntToStringMap.Empty();
	IntToNameMap.Empty();
	StringToIntMap.Empty();
	StringToStringMap.Empty();
	NameToIntMap.Empty();
	NameToNameMap.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		IntToIntMap.Add(FMath::Rand(), FMath::Rand());
		IntToStringMap.Add(FMath::Rand(), FString::SanitizeFloat(FMath::SRand()));
		IntToNameMap.Add(FMath::Rand(), FName(*FString::SanitizeFloat(FMath::SRand())));

		const FString StringKey = FString::SanitizeFloat(FMath::SRand());
		const FString StringValue = FString::SanitizeFloat(FMath::SRand());

		StringToIntMap.Add(StringKey, FMath::Rand());
		StringToStringMap.Add(StringKey, StringValue);
		NameToIntMap.Add(FName(*StringKey), FMath::Rand());
		NameToNameMap.Add(FName(*StringKey), FName(*StringValue));
	}
}

bool FDlgTestMapPrimitive::IsEqual(const FDlgTestMapPrimitive& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestMapPrimitive::IsEqual:\n");

	auto IntToString = [](const int32& Value) -> FString
	{
		return FString::FromInt(Value);
	};
	auto NameToString = [](const FName& Value) -> FString
	{
		return Value.ToString();
	};
	auto StringToString = [](const FString& Value) -> FString
	{
		return Value;
	};

	PropertyName = TEXT("IntToIntMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<int32, int32>(IntToIntMap, Other.IntToIntMap, PropertyName, OutError,
		IntToString, IntToString);

	PropertyName = TEXT("IntToStringMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<int32, FString>(IntToStringMap, Other.IntToStringMap, PropertyName, OutError,
		IntToString, StringToString);

	PropertyName = TEXT("IntToNameMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<int32, FName>(IntToNameMap, Other.IntToNameMap, PropertyName, OutError,
		IntToString, NameToString);

	PropertyName = TEXT("NameToIntMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<FName, int32>(NameToIntMap, Other.NameToIntMap, PropertyName, OutError,
		NameToString, IntToString);

	PropertyName = TEXT("NameToNameMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<FName, FName>(NameToNameMap, Other.NameToNameMap, PropertyName, OutError,
		NameToString, NameToString);

	PropertyName = TEXT("StringToIntMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<FString, int32>(StringToIntMap, Other.StringToIntMap, PropertyName, OutError,
		StringToString, IntToString);

	PropertyName = TEXT("StringToStringMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<FString, FString>(StringToStringMap, Other.StringToStringMap, PropertyName, OutError,
		StringToString, StringToString);

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestMapPrimitive::operator==(const FDlgTestMapPrimitive& Other) const
{
	return FDlgHelper::AreMapsEqual(IntToIntMap, Other.IntToIntMap) &&
		   FDlgHelper::AreMapsEqual(IntToStringMap, Other.IntToStringMap) &&
		   FDlgHelper::AreMapsEqual(IntToNameMap, Other.IntToNameMap) &&
		   FDlgHelper::AreMapsEqual(StringToIntMap, Other.StringToIntMap) &&
		   FDlgHelper::AreMapsEqual(StringToStringMap, Other.StringToStringMap) &&
		   FDlgHelper::AreMapsEqual(NameToIntMap, Other.NameToIntMap) &&
		   FDlgHelper::AreMapsEqual(NameToNameMap, Other.NameToNameMap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestMapStruct
void FDlgTestMapStruct::GenerateRandomData()
{
	IntToStructPrimitiveMap.Empty();
	StructPrimitiveToIntMap.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		IntToStructPrimitiveMap.Add(FMath::Rand(), FDlgTestStructPrimitives(Options));
		StructPrimitiveToIntMap.Add(FDlgTestStructPrimitives(Options), FMath::Rand());
	}
}

bool FDlgTestMapStruct::IsEqual(const FDlgTestMapStruct& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestMapStruct::IsEqual:\n");

	auto IntToString = [](const int32& Value) -> FString
	{
		return FString::FromInt(Value);
	};
	auto StructPrimitiveToString = [](const FDlgTestStructPrimitives& Value) -> FString
	{
		return FString::Printf(TEXT("Struct(%s)"), *Value.ToString());
	};

	PropertyName = TEXT("IntToStructPrimitiveMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<int32, FDlgTestStructPrimitives>(IntToStructPrimitiveMap, Other.IntToStructPrimitiveMap, PropertyName, OutError,
		IntToString, StructPrimitiveToString);

	PropertyName = TEXT("StructPrimitiveToIntMap");
	bIsEqual = bIsEqual && TestMapIsEqualToOther<FDlgTestStructPrimitives, int32>(StructPrimitiveToIntMap, Other.StructPrimitiveToIntMap, PropertyName, OutError,
		StructPrimitiveToString, IntToString);

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}


bool FDlgTestMapStruct::operator==(const FDlgTestMapStruct& Other) const
{
	return FDlgHelper::AreMapsEqual(IntToStructPrimitiveMap, Other.IntToStructPrimitiveMap) &&
		   FDlgHelper::AreMapsEqual(StructPrimitiveToIntMap, Other.StructPrimitiveToIntMap);
}

