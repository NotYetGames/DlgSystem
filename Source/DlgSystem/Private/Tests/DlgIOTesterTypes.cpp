// Copyright 2017-2018 Csaba Molnar, Daniel Butum
#include "DlgIOTesterTypes.h"

#include "Engine/Engine.h"

#include "DlgTesterHelper.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDlgTestObjectPrimitivesBase
void UDlgTestObjectPrimitivesBase::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	SetToDefaults();

	Integer = FMath::Rand();
	String = FString::SanitizeFloat(FMath::SRand());
}

void UDlgTestObjectPrimitivesBase::SetToDefaults()
{
	Integer = 0;
	String.Empty();
}

bool UDlgTestObjectPrimitivesBase::IsEqual(const Self* Other, FString& OutError) const
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDlgTestObjectPrimitives_ChildA
void UDlgTestObjectPrimitives_ChildA::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Super::GenerateRandomData(InOptions);
	IntegerChildA = FMath::Rand();
}

void UDlgTestObjectPrimitives_ChildA::SetToDefaults()
{
	Super::SetToDefaults();
	IntegerChildA = 0;
}

bool UDlgTestObjectPrimitives_ChildA::IsEqual(const Super* Other, FString& OutError) const
{
	const Self* SelfOther = Cast<Self>(Other);
	if (SelfOther == nullptr)
	{
		OutError += FString::Printf(TEXT("This.Class (%s) != Other.Class (%s)\n"), *FDlgTestHelper::GetClassNameFromObject(this), *FDlgTestHelper::GetClassNameFromObject(Other));
		return false;
	}

	bool bIsEqual = Super::IsEqual(Other, OutError);
	if (IntegerChildA != SelfOther->IntegerChildA)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.IntegerChildA (%d) != Other.IntegerChildA (%d)\n"), IntegerChildA, SelfOther->IntegerChildA);
	}

	return bIsEqual;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDlgTestObjectPrimitives_ChildB
void UDlgTestObjectPrimitives_ChildB::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Super::GenerateRandomData(InOptions);
	StringChildB = FString::SanitizeFloat(FMath::SRand());
}

void UDlgTestObjectPrimitives_ChildB::SetToDefaults()
{
	Super::SetToDefaults();
	StringChildB.Empty();
}

bool UDlgTestObjectPrimitives_ChildB::IsEqual(const Super* Other, FString& OutError) const
{
	const Self* SelfOther = Cast<Self>(Other);
	if (SelfOther == nullptr)
	{
		OutError += FString::Printf(TEXT("This.Class (%s) != Other.Class (%s)\n"), *FDlgTestHelper::GetClassNameFromObject(this), *FDlgTestHelper::GetClassNameFromObject(Other));
		return false;
	}

	bool bIsEqual = Super::IsEqual(Other, OutError);
	if (StringChildB != SelfOther->StringChildB)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.StringChildB (%s) != Other.StringChildB (%s)\n"), *StringChildB, *SelfOther->StringChildB);
	}

	return bIsEqual;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDlgTestObjectPrimitives_GrandChildA_Of_ChildA
void UDlgTestObjectPrimitives_GrandChildA_Of_ChildA::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Super::GenerateRandomData(InOptions);
	IntegerGrandChildA_Of_ChildA = FMath::Rand();
}

void UDlgTestObjectPrimitives_GrandChildA_Of_ChildA::SetToDefaults()
{
	Super::SetToDefaults();
	IntegerGrandChildA_Of_ChildA = 0;
}

bool UDlgTestObjectPrimitives_GrandChildA_Of_ChildA::IsEqual(const SuperBase* Other, FString& OutError) const
{
	const Self* SelfOther = Cast<Self>(Other);
	if (SelfOther == nullptr)
	{
		OutError += FString::Printf(TEXT("This.Class (%s) != Other.Class (%s)\n"), *FDlgTestHelper::GetClassNameFromObject(this), *FDlgTestHelper::GetClassNameFromObject(Other));
		return false;
	}

	bool bIsEqual = Super::IsEqual(Other, OutError);
	if (IntegerGrandChildA_Of_ChildA != SelfOther->IntegerGrandChildA_Of_ChildA)
	{
		bIsEqual = false;
		OutError += FString::Printf(TEXT("\tThis.IntegerGrandChildA_Of_ChildA (%d) != Other.IntegerGrandChildA_Of_ChildA (%d)\n"), IntegerGrandChildA_Of_ChildA, IntegerGrandChildA_Of_ChildA);
	}

	return bIsEqual;
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

	ObjectPrimitivesBase = NewObject<UDlgTestObjectPrimitivesBase>();
	ObjectPrimitivesBase->GenerateRandomData(Options);

	ObjectPrimitivesChildA = NewObject<UDlgTestObjectPrimitives_ChildA>();
	ObjectPrimitivesChildA->GenerateRandomData(Options);

	ObjectPrimitivesPolymorphismChildA = NewObject<UDlgTestObjectPrimitives_ChildA>();
	ObjectPrimitivesPolymorphismChildA->GenerateRandomData(Options);

	ObjectPrimitivesPolymorphismChildB = NewObject<UDlgTestObjectPrimitives_ChildB>();
	ObjectPrimitivesPolymorphismChildB->GenerateRandomData(Options);

	ObjectPrimitivesGrandChildA = NewObject<UDlgTestObjectPrimitives_GrandChildA_Of_ChildA>();
	ObjectPrimitivesGrandChildA->GenerateRandomData(Options);

	ObjectPrimitivesPolymorphismBaseGrandChildA = NewObject<UDlgTestObjectPrimitives_GrandChildA_Of_ChildA>();
	ObjectPrimitivesPolymorphismBaseGrandChildA->GenerateRandomData(Options);

	ObjectPrimitivesPolymorphismChildGrandChildA = NewObject<UDlgTestObjectPrimitives_GrandChildA_Of_ChildA>();
	ObjectPrimitivesPolymorphismChildGrandChildA->GenerateRandomData(Options);

	if (FMath::RandBool())
	{
		ObjectSwitch = nullptr;
	}
	else
	{
		ObjectSwitch = NewObject<UDlgTestObjectPrimitivesBase>();
		ObjectSwitch->GenerateRandomData(Options);
	}
}

bool FDlgTestStructPrimitives::IsEqual(const Self& Other, FString& OutError) const
{
	//if (Options != Other.Options)
	//{
	//	OutError += FString::Printf(TEXT("This.Options(%s) != Other.Options(%s) This will make the test fail most likely:\n"), *Options.ToString(), *Other.Options.ToString());
	//	return false;
	//}

	bool bIsEqual = true;
	OutError += TEXT("FDlgTestStructPrimitives::IsEqual:\n");

	if ((ObjectSwitch == nullptr && Other.ObjectSwitch != nullptr) || (ObjectSwitch != nullptr && Other.ObjectSwitch == nullptr))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectSwitch (%d) != Other.ObjectSwitch (%d). Null values are different. Writer supports writing null values?\n"), ObjectSwitch != nullptr, Other.ObjectSwitch != nullptr);
		return false;
	}

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

	if (!ObjectPrimitivesBase->IsEqual(Other.ObjectPrimitivesBase, OutError))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectPrimitives (%s) != Other.ObjectPrimitives (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(ObjectPrimitivesBase), *FDlgTestHelper::GetFullNameFromObject(Other.ObjectPrimitivesBase));
	}

	if (!ObjectPrimitivesChildA->IsEqual(Other.ObjectPrimitivesChildA, OutError))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectPrimitivesChildA (%s) != Other.ObjectPrimitivesChildA (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(ObjectPrimitivesChildA), *FDlgTestHelper::GetFullNameFromObject(Other.ObjectPrimitivesChildA));
	}

	if (!ObjectPrimitivesPolymorphismChildA->IsEqual(Other.ObjectPrimitivesPolymorphismChildA, OutError))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectPrimitivesPolymorphismChildA (%s) != Other.ObjectPrimitivesPolymorphismChildA (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(ObjectPrimitivesPolymorphismChildA), *FDlgTestHelper::GetFullNameFromObject(Other.ObjectPrimitivesPolymorphismChildA));
	}

	if (!ObjectPrimitivesPolymorphismChildB->IsEqual(Other.ObjectPrimitivesPolymorphismChildB, OutError))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectPrimitivesPolymorphismChildB (%s) != Other.ObjectPrimitivesPolymorphismChildB (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(ObjectPrimitivesPolymorphismChildB), *FDlgTestHelper::GetFullNameFromObject(Other.ObjectPrimitivesPolymorphismChildB));
	}

	if (!ObjectPrimitivesGrandChildA->IsEqual(Other.ObjectPrimitivesGrandChildA, OutError))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectPrimitivesGrandChildA (%s) != Other.ObjectPrimitivesGrandChildA (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(ObjectPrimitivesGrandChildA), *FDlgTestHelper::GetFullNameFromObject(Other.ObjectPrimitivesGrandChildA));
	}

	if (!ObjectPrimitivesPolymorphismBaseGrandChildA->IsEqual(Other.ObjectPrimitivesPolymorphismBaseGrandChildA, OutError))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectPrimitivesPolymorphismBaseGrandChildA (%s) != Other.ObjectPrimitivesPolymorphismBaseGrandChildA (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(ObjectPrimitivesPolymorphismBaseGrandChildA), *FDlgTestHelper::GetFullNameFromObject(Other.ObjectPrimitivesPolymorphismBaseGrandChildA));
	}

	if (!ObjectPrimitivesPolymorphismChildGrandChildA->IsEqual(Other.ObjectPrimitivesPolymorphismChildGrandChildA, OutError))
	{
		OutError += FString::Printf(TEXT("\tThis.ObjectPrimitivesPolymorphismChildGrandChildA (%s) != Other.ObjectPrimitivesPolymorphismChildGrandChildA (%s)\n"), *FDlgTestHelper::GetFullNameFromObject(ObjectPrimitivesPolymorphismChildGrandChildA), *FDlgTestHelper::GetFullNameFromObject(Other.ObjectPrimitivesPolymorphismChildGrandChildA));
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
	ObjectPrimitivesBase = nullptr;
	ObjectPrimitivesChildA = nullptr;
	ObjectPrimitivesPolymorphismChildA = nullptr;
	ObjectPrimitivesPolymorphismChildB = nullptr;
	ObjectPrimitivesGrandChildA = nullptr;
	ObjectPrimitivesPolymorphismBaseGrandChildA = nullptr;
	ObjectPrimitivesPolymorphismChildGrandChildA = nullptr;
	ObjectSwitch = nullptr;
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
	ObjectArrayConstantNulls.Empty();

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
	ObjectArrayConstantNulls.SetNum(Num);

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

		// ObjectArrayConstantNulls
		ObjectArrayConstantNulls[i] = nullptr;
	}
}

bool FDlgTestArrayPrimitive::IsEqual(const Self& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestArrayPrimitive::IsEqual:\n");

	PropertyName = TEXT("ObjectArrayConstantNulls");
	if (ObjectArrayConstantNulls.Num() != Other.ObjectArrayConstantNulls.Num())
	{
		OutError = FString::Printf(TEXT("This.ObjectArrayConstantNulls.Num(%d) != Other.ObjectArrayConstantNulls.Num(%d). Most likely because the writer does not support writing nulls."),
			ObjectArrayConstantNulls.Num(), Other.ObjectArrayConstantNulls.Num());
		return false;
	}

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestArrayStruct
void FDlgTestArrayComplex::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	StructArrayPrimitives.Empty();
	StructArrayOfArrayPrimitives.Empty();
	ObjectArrayPrimitivesBase.Empty();
	ObjectArrayPrimitivesAll.Empty();

	const int32 Num = FMath::RandHelper(20) + 3;
	static TArray<UClass*> ObjectPrimitiveClassPool = {
		UDlgTestObjectPrimitivesBase::StaticClass(), UDlgTestObjectPrimitives_ChildA::StaticClass(),
		UDlgTestObjectPrimitives_ChildB::StaticClass(), UDlgTestObjectPrimitives_GrandChildA_Of_ChildA::StaticClass(), nullptr
	};
	for (int32 Index = 0; Index < Num; Index++)
	{
		// StructArrayPrimitives
		FDlgTestStructPrimitives StructPrimitives;
		StructPrimitives.GenerateRandomData(Options);
		StructArrayPrimitives.Add(StructPrimitives);

		//StructArrayOfArrayPrimitives
		FDlgTestArrayPrimitive ArrayOfArrayPrimitive;
		ArrayOfArrayPrimitive.GenerateRandomData(Options);
		StructArrayOfArrayPrimitives.Add(ArrayOfArrayPrimitive);

		// ObjectArrayPrimitivesBase
		{
			UDlgTestObjectPrimitivesBase* ObjectPrimitives = NewObject<UDlgTestObjectPrimitivesBase>();
			ObjectPrimitives->GenerateRandomData(Options);
			ObjectArrayPrimitivesBase.Add(ObjectPrimitives);
		}

		// ObjectArrayPrimitivesAll
		{
			UClass* ChosenClass = ObjectPrimitiveClassPool[FMath::RandHelper(ObjectPrimitiveClassPool.Num())];
			if (ChosenClass == nullptr)
			{
				ObjectArrayPrimitivesAll.Add(nullptr);
			}
			else
			{
				UDlgTestObjectPrimitivesBase* ObjectPrimitives = NewObject<UDlgTestObjectPrimitivesBase>((UObject*)GetTransientPackage(), ChosenClass);
				ObjectPrimitives->GenerateRandomData(Options);
				ObjectArrayPrimitivesAll.Add(ObjectPrimitives);
			}
		}

		// ObjectArrayFrequentsNulls
		{
			const bool bNull = FMath::RandBool();
			if (bNull)
			{
				ObjectArrayFrequentsNulls.Add(nullptr);
			}
			else
			{
				UDlgTestObjectPrimitivesBase* ObjectPrimitives = NewObject<UDlgTestObjectPrimitivesBase>();
				ObjectPrimitives->GenerateRandomData(Options);
				ObjectArrayFrequentsNulls.Add(ObjectPrimitives);
			}
		}
	}
}

bool FDlgTestArrayComplex::IsEqual(const Self& Other, FString& OutError) const
{
	FString PropertyName;
	bool bIsEqual = true;
	OutError += TEXT("FDlgTestArrayStruct::IsEqual:\n");

	PropertyName = TEXT("StructArrayPrimitives");
	bIsEqual = bIsEqual && FDlgTestHelper::IsComplexArrayEqual<FDlgTestStructPrimitives>(StructArrayPrimitives, Other.StructArrayPrimitives, PropertyName, OutError);

	PropertyName = TEXT("StructArrayOfArrayPrimitives");
	bIsEqual = bIsEqual && FDlgTestHelper::IsComplexArrayEqual<FDlgTestArrayPrimitive>(StructArrayOfArrayPrimitives, Other.StructArrayOfArrayPrimitives, PropertyName, OutError);

	PropertyName = TEXT("ObjectArrayPrimitivesBase");
	bIsEqual = bIsEqual && FDlgTestHelper::IsComplexPointerArrayEqual<UDlgTestObjectPrimitivesBase>(ObjectArrayPrimitivesBase, Other.ObjectArrayPrimitivesBase, PropertyName, OutError);

	PropertyName = TEXT("ObjectArrayPrimitivesAll");
	bIsEqual = bIsEqual && FDlgTestHelper::IsComplexPointerArrayEqual<UDlgTestObjectPrimitivesBase>(ObjectArrayPrimitivesAll, Other.ObjectArrayPrimitivesAll, PropertyName, OutError);

	PropertyName = TEXT("ObjectArrayFrequentsNulls");
	bIsEqual = bIsEqual && FDlgTestHelper::IsComplexPointerArrayEqual<UDlgTestObjectPrimitivesBase>(ObjectArrayFrequentsNulls, Other.ObjectArrayFrequentsNulls, PropertyName, OutError);

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
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

bool FDlgTestSetPrimitive::IsEqual(const Self& Other, FString& OutError) const
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

bool FDlgTestSetComplex::IsEqual(const Self& Other, FString& OutError) const
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
	ObjectConstantNullMap.Empty();

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
		// TOOD
		//ObjectConstantNullMap.Add(FName(*StringKey), nullptr);

		if (Options.bSupportsColorPrimitives)
		{
			const FColor Color(FMath::Rand());
			NameToColorMap.Add(FName(*StringKey), Color);
		}
	}
}

bool FDlgTestMapPrimitive::IsEqual(const Self& Other, FString& OutError) const
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

	PropertyName = TEXT("ObjectConstantNullMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsPointersMapEqual<FName, UDlgTestObjectPrimitivesBase>(ObjectConstantNullMap, Other.ObjectConstantNullMap, PropertyName, OutError,
		FDlgTestHelper::NameToString);

	// Clear error message
	if (bIsEqual)
	{
		OutError.Empty();
	}
	return bIsEqual;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDlgTestMapStruct
void FDlgTestMapComplex::GenerateRandomData(const FDlgIOTesterOptions& InOptions)
{
	Options = InOptions;
	IntToStructPrimitiveMap.Empty();
	StructPrimitiveToIntMap.Empty();
	NameToStructPrimitiveMap.Empty();

	const int32 Num = FMath::RandHelper(10) + 2;
	for (int32 i = 0; i < Num; ++i)
	{
		const FString StringKey = FString::SanitizeFloat(FMath::SRand());
		FDlgTestStructPrimitives StructPrimitives;
		StructPrimitives.GenerateRandomData(Options);

		IntToStructPrimitiveMap.Add(FMath::Rand(), StructPrimitives);
		NameToStructPrimitiveMap.Add(FName(*StringKey), StructPrimitives);
		StructPrimitiveToIntMap.Add(StructPrimitives, FMath::Rand());
	}
}

bool FDlgTestMapComplex::IsEqual(const Self& Other, FString& OutError) const
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

	PropertyName = TEXT("NameToStructPrimitiveMap");
	bIsEqual = bIsEqual && FDlgTestHelper::IsMapEqual<FName, FDlgTestStructPrimitives>(NameToStructPrimitiveMap, Other.NameToStructPrimitiveMap, PropertyName, OutError,
		FDlgTestHelper::NameToString, StructPrimitiveToString);

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
