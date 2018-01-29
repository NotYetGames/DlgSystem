// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgIOTesterTypes.h"

FDlgTestStructPrimitives::FDlgTestStructPrimitives()
{
	bBoolean = FMath::RandBool();
	Integer = FMath::Rand();
	Float = FMath::SRand();
	String = FString::SanitizeFloat(Float);
	Name = FName(*String);
	Enum = static_cast<EDlgTestEnum>(FMath::RandHelper(static_cast<int32>(EDlgTestEnum::ETE_NumOf)));
	Text = FText::FromString(String);
}

bool FDlgTestStructPrimitives::operator==(const FDlgTestStructPrimitives& Other) const
{
	return bBoolean == Other.bBoolean &&
		   Integer == Other.Integer &&
		   FMath::IsNearlyEqual(Float, Other.Float) &&
		   String == Other.String &&
		   Name == Other.Name &&
		   Text.ToString() == Other.Text.ToString() &&
		   Enum == Other.Enum;
}

FDlgTestArrayPrimitive::FDlgTestArrayPrimitive()
{
	const int32 Num = FMath::RandHelper(10) + 2;

	StringArray.SetNum(Num);
	NameArray.SetNum(Num);

	for (int32 i = 0; i < Num; ++i)
	{
		StringArray[i] = FString::SanitizeFloat(FMath::SRand());
		NameArray[i] = FName(*StringArray[i]);
	}
}

bool FDlgTestArrayPrimitive::operator==(const FDlgTestArrayPrimitive& Other) const
{
	return StringArray == Other.StringArray && NameArray == Other.NameArray;
}

FDlgTestArrayStruct::FDlgTestArrayStruct()
{
	StructArray.SetNum(FMath::RandHelper(10) + 2);
}

bool FDlgTestArrayStruct::operator==(const FDlgTestArrayStruct& Other) const
{
	return StructArray == Other.StructArray;
}

FDlgTestArrayEnum::FDlgTestArrayEnum()
{
	EnumArray.SetNum(FMath::RandHelper(10) + 2);
	for (int32 i = 0; i < EnumArray.Num(); ++i)
	{
		EnumArray[i] = static_cast<EDlgTestEnum>(FMath::RandHelper(static_cast<int32>(EDlgTestEnum::ETE_NumOf)));
	}
}

bool FDlgTestArrayEnum::operator==(const FDlgTestArrayEnum& Other) const
{
	return EnumArray == Other.EnumArray;
}


FDlgTestSetPrimitive::FDlgTestSetPrimitive()
{
	const int32 Num = FMath::RandHelper(10) + 2;

	for (int32 i = 0; i < Num; ++i)
	{
		StringSet.Add(FString::SanitizeFloat(FMath::SRand()));
		IntSet.Add(FMath::Rand());
	}
}

bool FDlgTestSetPrimitive::operator==(const FDlgTestSetPrimitive& Other) const
{
	return SetEqualsSet(StringSet, Other.StringSet) && SetEqualsSet(IntSet, Other.IntSet);
}

FDlgTestSetStruct::FDlgTestSetStruct()
{
	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		Set.Add({});
	}
}

bool FDlgTestSetStruct::operator==(const FDlgTestSetStruct& Other) const
{
	return SetEqualsSet(Set, Other.Set);
}

FDlgTestSetEnum::FDlgTestSetEnum()
{
	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		Set.Add(static_cast<EDlgTestEnum>(FMath::RandHelper(static_cast<int32>(EDlgTestEnum::ETE_NumOf))));
	}
}

bool FDlgTestSetEnum::operator==(const FDlgTestSetEnum& Other) const
{
	return SetEqualsSet(Set, Other.Set);
}
