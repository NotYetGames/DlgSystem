// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgIOTesterTypes.h"

#include "Engine/Engine.h"

#include "DlgTesterHelper.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDlgTestObjectPrimitives
void UDlgTestObjectPrimitives::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	SetToDefaults();

	Integer = FMath::Rand();
	String = FString::SanitizeFloat(FMath::SRand());
}

void UDlgTestObjectPrimitives::SetToDefaults()
{
	Integer = 0;
	String.Empty();
}

bool UDlgTestObjectPrimitives::IsEqual(const UDlgTestObjectPrimitives* Other, FString& OutError) const
{
	if (Other == nullptr)
	{
		OutError += FString::Printf(TEXT("\tOther is nullptr.\n"));
		return false;
	}

	bool bIsEqual = true;
	if (Integer != Other->Integer)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Integer (%d) != Other.Integer (%d)\n"), Integer, Other->Integer);
	}

	if (String != Other->String)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.String (%s) != Other.String (%s)\n"), *String, *Other->String);
	}

	return bIsEqual;
}

bool UDlgTestObjectPrimitives::operator==(const UDlgTestObjectPrimitives& Other) const
{
	return Integer == Other.Integer &&
		   String == Other.String;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestStructPrimitives
void FDlgTestStructPrimitives::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	SetToDefaults();

	bBoolean = FMath::RandBool();
	Integer = FMath::Rand();
	Float = FMath::SRand();
	String = FString::SanitizeFloat(Float);
	Name = FName(*String);
	Enum = static_cast<EDlgTestEnum>(FMath::RandHelper(static_cast<int32>(EDlgTestEnum::ETE_NumOf)));
	Text = FText::FromString(String);

	if (Options.bSupportsColorPrimitives)
	{
		LinearColor = FLinearColor(FMath::SRand(), FMath::SRand(), FMath::SRand());
		Color = LinearColor.ToFColor(false);
	}
	if (Options.bSupportsDatePrimitive)
	{
		DateTime = FDateTime(FMath::Rand());
	}
	IntPoint = FIntPoint(FMath::Rand(), FMath::Rand());
	Vector3 = FVector(FMath::SRand(), FMath::SRand(), FMath::SRand());
	Vector2 = FVector2D(FMath::SRand(), FMath::SRand());
	Vector4 = FVector4(FMath::SRand(), FMath::SRand(), FMath::SRand(), FMath::SRand());
	Rotator = FRotator(FMath::SRand(), FMath::SRand(), FMath::SRand());
	Matrix = FMatrix(Vector3, FVector(IntPoint), FVector(Vector4), Vector3);
	Transform = FTransform(Vector3);
	Guid = FGuid::NewGuid();

	const TArray<UClass*> ClassesPool = {
		UField::StaticClass(), UStruct::StaticClass(), nullptr, UObject::StaticClass(), UBoolProperty::StaticClass(), UIntProperty::StaticClass(),
		UStructProperty::StaticClass(), UStrProperty::StaticClass(), nullptr
	};
	Class = ClassesPool[FMath::RandHelper(ClassesPool.Num())];

	check(GEngine);
	const TArray<UTexture2D*> TexturesPool = {
		GEngine->DefaultTexture, nullptr, GEngine->DefaultBokehTexture, GEngine->DefaultBloomKernelTexture, GEngine->LightMapDensityTexture,
		GEngine->HighFrequencyNoiseTexture, GEngine->MiniFontTexture, GEngine->PreIntegratedSkinBRDFTexture, nullptr
	};
	Texture2DReference = TexturesPool[FMath::RandHelper(TexturesPool.Num())];
	ConstTexture2D = GEngine->DefaultTexture;

	ObjectPrimitives = UDlgTestObjectPrimitives::New();
	ObjectPrimitives->GenerateRandomData(Options);
}

bool FDlgTestStructPrimitives::IsEqual(const FDlgTestStructPrimitives& Other, FString& OutError) const
{
	//if (Options != Other.Options)
	//{
	//	OutError += FString::Printf(TEXT("This.Options(%s) != Other.Options(%s) This will make the test fail most likely:\n"), *Options.ToString(), *Other.Options.ToString());
	//	return false;
	//}

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

	if (!FDlgTestHelper::IsFloatEqual(Float, Other.Float))
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

	if (Color != Other.Color)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Color (%s) != Other.Color (%s)\n"), *Color.ToString(), *Other.Color.ToString());
	}

	if (!LinearColor.Equals(Other.LinearColor))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.LinearColor (%s) != Other.LinearColor (%s)\n"), *LinearColor.ToString(), *Other.LinearColor.ToString());
	}

	if (DateTime != Other.DateTime)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.DateTime.Ticks (%d) != Other.DateTime.Ticks (%d)\n"), DateTime.GetTicks(), Other.DateTime.GetTicks());
	}

	if (IntPoint != Other.IntPoint)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.IntPoint (%s) != Other.IntPoint (%s)\n"), *IntPoint.ToString(), *Other.IntPoint.ToString());
	}

	if (!Vector3.Equals(Other.Vector3))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Vector3 (%s) != Other.Vector3 (%s)\n"), *Vector3.ToString(), *Other.Vector3.ToString());
	}

	if (!Vector2.Equals(Other.Vector2))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Vector2 (%s) != Other.Vector2 (%s)\n"), *Vector2.ToString(), *Other.Vector2.ToString());
	}

	if (!Vector4.Equals(Other.Vector4))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Vector4 (%s) != Other.Vector4 (%s)\n"), *Vector4.ToString(), *Other.Vector4.ToString());
	}

	if (!Rotator.Equals(Other.Rotator))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Rotator (%s) != Other.Rotator (%s)\n"), *Rotator.ToString(), *Other.Rotator.ToString());
	}

	if (!Matrix.Equals(Other.Matrix))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Matrix (%s) != Other.Matrix (%s)\n"), *Matrix.ToString(), *Other.Matrix.ToString());
	}

	if (!Transform.Equals(Other.Transform))
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Transform (%s) != Other.Transform (%s)\n"), *Transform.ToString(), *Other.Transform.ToString());
	}

	if (Guid != Other.Guid)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Guid (%s) != Other.Guid (%s)\n"), *Guid.ToString(), *Other.Guid.ToString());
	}

	if (Class != Other.Class)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Class (%s) != Other.Class (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(Class), *FDlgTestHelper::GetFullNameFromObject(Other.Class));
	}

	if (EmptyObjectInitialized != Other.EmptyObjectInitialized || EmptyObjectInitialized != nullptr || Other.EmptyObjectInitialized != nullptr)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.EmptyObjectInitialized OR Other.EmptyObjectInitialized are not empty :O\n"));
	}

	if (EmptyObjectInitializedReference != Other.EmptyObjectInitializedReference || EmptyObjectInitializedReference != nullptr || Other.EmptyObjectInitializedReference != nullptr)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.EmptyObjectInitializedReference OR Other.EmptyObjectInitializedReference are not empty :O\n"));
	}

	if (Texture2DReference != Other.Texture2DReference)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.Texture2D (%s) != Other.Texture2D (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(Texture2DReference), *FDlgTestHelper::GetFullNameFromObject(Other.Texture2DReference));
	}

	if (!ObjectPrimitives->IsEqual(Other.ObjectPrimitives, OutError))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectPrimitives (%s) != Other.ObjectPrimitives (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(ObjectPrimitives), *FDlgTestHelper::GetFullNameFromObject(Other.ObjectPrimitives));
	}

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

void FDlgTestStructPrimitives::SetToDefaults()
{
	Integer = 42;
	bBoolean = true;
	Enum = EDlgTestEnum::ETE_Second;
	Float = -23.549f;
	Name = NAME_None;
	String.Empty();
	EmptyString.Empty();
	Text = FText::GetEmpty();
	LinearColor = FLinearColor(ForceInitToZero);
	Color = FColor(ForceInitToZero);
	DateTime = FDateTime(0);
	IntPoint = FIntPoint(ForceInitToZero);
	Vector3 = FVector(ForceInitToZero);
	Vector2 = FVector2D(ForceInitToZero);
	Vector4 = FVector4(ForceInitToZero);
	Rotator = FRotator(ForceInitToZero);
	Matrix = FMatrix(ForceInitToZero);
	Transform = FTransform();
	Guid = FGuid();
	Class = nullptr;
	EmptyObjectInitialized = nullptr;
	EmptyObjectInitializedReference = nullptr;
	Texture2DReference = nullptr;
	ConstTexture2D = nullptr;
	ObjectPrimitives = nullptr;
}

bool FDlgTestStructPrimitives::operator==(const FDlgTestStructPrimitives& Other) const
{
	return bBoolean == Other.bBoolean &&
		   Integer == Other.Integer &&
		   FDlgTestHelper::IsFloatEqual(Float, Other.Float) &&
		   String == Other.String &&
		   EmptyString == Other.EmptyString &&
		   Name == Other.Name &&
		   Text.EqualTo(Other.Text) &&
		   Enum == Other.Enum &&
		   LinearColor.Equals(Other.LinearColor) &&
		   Color == Other.Color &&
		   DateTime == Other.DateTime &&
		   IntPoint == Other.IntPoint &&
		   Vector3.Equals(Other.Vector3) &&
		   Vector2.Equals(Other.Vector2) &&
		   Vector4.Equals(Other.Vector4) &&
		   Rotator.Equals(Other.Rotator) &&
		   Matrix.Equals(Other.Matrix) &&
		   Transform.Equals(Other.Transform) &&
		   Guid == Other.Guid &&
		   Class == Other.Class &&
		   EmptyObjectInitialized == Other.EmptyObjectInitialized &&
		   EmptyObjectInitializedReference == Other.EmptyObjectInitializedReference &&
		   Texture2DReference == Other.Texture2DReference &&
		   *ObjectPrimitives == *Other.ObjectPrimitives;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestArrayPrimitive
void FDlgTestArrayPrimitive::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	IntArray.Empty();
	BoolArray.Empty();
	FloatArray.Empty();
	EnumArray.Empty();
	NameArray.Empty();
	StringArray.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	IntArray.SetNum(Num);
	BoolArray.SetNum(Num);
	FloatArray.SetNum(Num);
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
		FloatArray[i] = FMath::SRand();
		StringArray[i] = FString::SanitizeFloat(FloatArray[i]);
		NameArray[i] = FName(*StringArray[i]);
	}
}

bool FDlgTestArrayPrimitive::IsEqual(const FDlgTestArrayPrimitive& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestArrayPrimitive::IsEqual:\n");

	PropertyName = TEXT("IntArray");
	bIsEqual = bIsEqual && FDlgTestHelper::IsPrimitiveArrayEqual<int32>(IntArray, Other.IntArray, PropertyName, OutError,
		[](const int32& Value) -> FString { return FString::FromInt(Value); });

	PropertyName = TEXT("BoolArray");
	bIsEqual = bIsEqual && FDlgTestHelper::IsPrimitiveArrayEqual<bool>(BoolArray, Other.BoolArray, PropertyName, OutError,
		[](const bool& Value) -> FString { return FString::FromInt(Value); });

	PropertyName = TEXT("FloatArray");
	bIsEqual = bIsEqual && FDlgTestHelper::IsPrimitiveArrayEqual<float>(FloatArray, Other.FloatArray, PropertyName, OutError,
		[](const float& Value) -> FString { return FString::SanitizeFloat(Value); });

	PropertyName = TEXT("EnumArray");
	bIsEqual = bIsEqual && FDlgTestHelper::IsPrimitiveArrayEqual<EDlgTestEnum>(EnumArray, Other.EnumArray, PropertyName, OutError,
		[](const EDlgTestEnum& Value) -> FString { return FString::FromInt(static_cast<int32>(Value)); });

	PropertyName = TEXT("NameArray");
	bIsEqual = bIsEqual && FDlgTestHelper::IsPrimitiveArrayEqual<FName>(NameArray, Other.NameArray, PropertyName, OutError,
		[](const FName& Value) -> FString { return Value.ToString(); });

	PropertyName = TEXT("StringArray");
	bIsEqual = bIsEqual && FDlgTestHelper::IsPrimitiveArrayEqual<FString>(StringArray, Other.StringArray, PropertyName, OutError,
		[](const FString& Value) -> FString { return Value; });

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
		   FDlgHelper::IsArrayEqual(FloatArray, Other.FloatArray) &&
		   EnumArray == Other.EnumArray &&
		   NameArray == Other.NameArray &&
		   StringArray == Other.StringArray;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestArrayStruct
void FDlgTestArrayComplex::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	StructArrayPrimitives.Empty();
	ObjectArrayPrimitives.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 Index = 0; Index < Num; Index++)
	{
		FDlgTestStructPrimitives StructPrimitives;
		StructPrimitives.GenerateRandomData(Options);
		StructArrayPrimitives.Add(StructPrimitives);

		UDlgTestObjectPrimitives* ObjectPrimitives = UDlgTestObjectPrimitives::New();
		ObjectPrimitives->GenerateRandomData(Options);
		ObjectArrayPrimitives.Add(ObjectPrimitives);
	}
}

bool FDlgTestArrayComplex::IsEqual(const FDlgTestArrayComplex& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestArrayStruct::IsEqual:\n");

	PropertyName = TEXT("StructArrayPrimitives");
	bIsEqual = bIsEqual && FDlgTestHelper::IsComplexArrayEqual<FDlgTestStructPrimitives>(StructArrayPrimitives, Other.StructArrayPrimitives, PropertyName, OutError);
	bIsEqual = bIsEqual && FDlgTestHelper::IsComplexPointerArrayEqual<UDlgTestObjectPrimitives>(ObjectArrayPrimitives, Other.ObjectArrayPrimitives, PropertyName, OutError);

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestArrayComplex::operator==(const FDlgTestArrayComplex& Other) const
{
	return StructArrayPrimitives == Other.StructArrayPrimitives &&
		   FDlgHelper::IsArrayOfPointersEqual<UDlgTestObjectPrimitives>(ObjectArrayPrimitives, Other.ObjectArrayPrimitives);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestSetPrimitive
void FDlgTestSetPrimitive::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
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
		const float Float = FMath::SRand();
		const FString String = FString::SanitizeFloat(Float);
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
	bIsEqual = bIsEqual && FDlgTestHelper::IsSetEqual<int32>(IntSet, Other.IntSet, PropertyName, OutError,
		[](const int32& Value) -> FString { return FString::FromInt(Value); });

	PropertyName = TEXT("EnumSet");
	bIsEqual = bIsEqual && FDlgTestHelper::IsSetEqual<EDlgTestEnum>(EnumSet, Other.EnumSet, PropertyName, OutError,
		[](const EDlgTestEnum& Value) -> FString { return FString::FromInt(static_cast<int32>(Value)); });

	PropertyName = TEXT("NameSet");
	bIsEqual = bIsEqual && FDlgTestHelper::IsSetEqual<FName>(NameSet, Other.NameSet, PropertyName, OutError,
		[](const FName& Value) -> FString { return Value.ToString(); });

	PropertyName = TEXT("StringSet");
	bIsEqual = bIsEqual && FDlgTestHelper::IsSetEqual<FString>(StringSet, Other.StringSet, PropertyName, OutError,
		[](const FString& Value) -> FString { return Value; });

	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestSetPrimitive::operator==(const FDlgTestSetPrimitive& Other) const
{
	return FDlgHelper::IsSetEqual(IntSet, Other.IntSet) &&
		   FDlgHelper::IsSetEqual(EnumSet, Other.EnumSet) &&
		   FDlgHelper::IsSetEqual(NameSet, Other.NameSet) &&
		   FDlgHelper::IsSetEqual(StringSet, Other.StringSet);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestSetStruct
void FDlgTestSetComplex::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	StructSetPrimitives.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		FDlgTestStructPrimitives StructPrimitives;
		StructPrimitives.GenerateRandomData(Options);
		StructSetPrimitives.Add(StructPrimitives);
	}
}

bool FDlgTestSetComplex::IsEqual(const FDlgTestSetComplex& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestSetStruct::IsEqual:\n");

	PropertyName = TEXT("StructSetPrimitives");
	bIsEqual = bIsEqual && FDlgTestHelper::IsSetEqual<FDlgTestStructPrimitives>(StructSetPrimitives, Other.StructSetPrimitives, PropertyName, OutError,
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

bool FDlgTestSetComplex::operator==(const FDlgTestSetComplex& Other) const
{
	return FDlgHelper::IsSetEqual(StructSetPrimitives, Other.StructSetPrimitives);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestMapPrimitive
void FDlgTestMapPrimitive::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	IntToIntMap.Empty();
	IntToStringMap.Empty();
	IntToNameMap.Empty();
	StringToIntMap.Empty();
	StringToStringMap.Empty();
	NameToIntMap.Empty();
	NameToNameMap.Empty();
	StringToFloatMap.Empty();
	IntToFloatMap.Empty();
	NameToColorMap.Empty();

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
		StringToFloatMap.Add(StringKey, FMath::SRand());
		IntToFloatMap.Add(FMath::Rand(), FMath::SRand());

		if (Options.bSupportsColorPrimitives)
		{
			const FColor Color(FMath::Rand());
			NameToColorMap.Add(FName(*StringKey), Color);
		}
	}
}

bool FDlgTestMapPrimitive::IsEqual(const FDlgTestMapPrimitive& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestMapPrimitive::IsEqual:\n");

	PropertyName = TEXT("IntToIntMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<int32, int32>(IntToIntMap, Other.IntToIntMap, PropertyName, OutError,
		FDlgTestHelper::IntToString, FDlgTestHelper::IntToString);

	PropertyName = TEXT("IntToStringMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<int32, FString>(IntToStringMap, Other.IntToStringMap, PropertyName, OutError,
		FDlgTestHelper::IntToString, FDlgTestHelper::StringToString);

	PropertyName = TEXT("IntToNameMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<int32, FName>(IntToNameMap, Other.IntToNameMap, PropertyName, OutError,
		FDlgTestHelper::IntToString, FDlgTestHelper::NameToString);

	PropertyName = TEXT("NameToIntMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<FName, int32>(NameToIntMap, Other.NameToIntMap, PropertyName, OutError,
		FDlgTestHelper::NameToString, FDlgTestHelper::IntToString);

	PropertyName = TEXT("NameToNameMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<FName, FName>(NameToNameMap, Other.NameToNameMap, PropertyName, OutError,
		FDlgTestHelper::NameToString, FDlgTestHelper::NameToString);

	PropertyName = TEXT("StringToIntMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<FString, int32>(StringToIntMap, Other.StringToIntMap, PropertyName, OutError,
		FDlgTestHelper::StringToString, FDlgTestHelper::IntToString);

	PropertyName = TEXT("StringToStringMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<FString, FString>(StringToStringMap, Other.StringToStringMap, PropertyName, OutError,
		FDlgTestHelper::StringToString, FDlgTestHelper::StringToString);

	PropertyName = TEXT("StringToFloatMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<FString, float>(StringToFloatMap, Other.StringToFloatMap, PropertyName, OutError,
		FDlgTestHelper::StringToString, FDlgTestHelper::FloatToString);

	PropertyName = TEXT("IntToFloatMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<int32, float>(IntToFloatMap, Other.IntToFloatMap, PropertyName, OutError,
		FDlgTestHelper::IntToString, FDlgTestHelper::FloatToString);

	PropertyName = TEXT("NameToColorMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<FName, FColor>(NameToColorMap, Other.NameToColorMap, PropertyName, OutError,
		FDlgTestHelper::NameToString, FDlgTestHelper::ColorToString);

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

bool FDlgTestMapPrimitive::operator==(const FDlgTestMapPrimitive& Other) const
{
	return FDlgHelper::IsMapEqual(IntToIntMap, Other.IntToIntMap) &&
		   FDlgHelper::IsMapEqual(IntToStringMap, Other.IntToStringMap) &&
		   FDlgHelper::IsMapEqual(IntToNameMap, Other.IntToNameMap) &&
		   FDlgHelper::IsMapEqual(StringToIntMap, Other.StringToIntMap) &&
		   FDlgHelper::IsMapEqual(StringToStringMap, Other.StringToStringMap) &&
		   FDlgHelper::IsMapEqual(NameToIntMap, Other.NameToIntMap) &&
		   FDlgHelper::IsMapEqual(NameToNameMap, Other.NameToNameMap) &&
		   FDlgHelper::IsMapEqual(StringToFloatMap, Other.StringToFloatMap) &&
		   FDlgHelper::IsMapEqual(IntToFloatMap, Other.IntToFloatMap) &&
		   FDlgHelper::IsMapEqual(NameToColorMap, Other.NameToColorMap);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestMapStruct
void FDlgTestMapComplex::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	IntToStructPrimitiveMap.Empty();
	StructPrimitiveToIntMap.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		const FString StringKey = FString::SanitizeFloat(FMath::SRand());
		FDlgTestStructPrimitives StructPrimitives;
		StructPrimitives.GenerateRandomData(Options);

		IntToStructPrimitiveMap.Add(FMath::Rand(), StructPrimitives);
		StructPrimitiveToIntMap.Add(StructPrimitives, FMath::Rand());
	}
}

bool FDlgTestMapComplex::IsEqual(const FDlgTestMapComplex& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestMapStruct::IsEqual:\n");

	auto StructPrimitiveToString = [](const FDlgTestStructPrimitives& Value) -> FString
	{
		return FString::Printf(TEXT("Struct(%s)"), *Value.ToString());
	};

	PropertyName = TEXT("IntToStructPrimitiveMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<int32, FDlgTestStructPrimitives>(IntToStructPrimitiveMap, Other.IntToStructPrimitiveMap, PropertyName, OutError,
		FDlgTestHelper::IntToString, StructPrimitiveToString);

	PropertyName = TEXT("StructPrimitiveToIntMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<FDlgTestStructPrimitives, int32>(StructPrimitiveToIntMap, Other.StructPrimitiveToIntMap, PropertyName, OutError,
		StructPrimitiveToString, FDlgTestHelper::IntToString);

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}


bool FDlgTestMapComplex::operator==(const FDlgTestMapComplex& Other) const
{
	return FDlgHelper::IsMapEqual(IntToStructPrimitiveMap, Other.IntToStructPrimitiveMap) &&
		FDlgHelper::IsMapEqual(StructPrimitiveToIntMap, Other.StructPrimitiveToIntMap);
}
