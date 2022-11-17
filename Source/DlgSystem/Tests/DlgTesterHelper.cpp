// Copyright Csaba Molnar, Daniel Butum. All Rights Reserved.
#include "DlgTesterHelper.h"


std::function<FString(const int32&)> FDlgTestHelper::Int32ToString = [](const int32& Value) -> FString
{
	return FString::FromInt(Value);
};

std::function<FString(const int64&)> FDlgTestHelper::Int64ToString = [](const int64& Value) -> FString
{
	return FString::Printf(TEXT("%lld"), Value);
};


std::function<FString(const FName&)> FDlgTestHelper::NameToString = [](const FName& Value) -> FString
{
	return Value.ToString();
};

std::function<FString(const FString&)> FDlgTestHelper::StringToString = [](const FString& Value) -> FString
{
	return Value;
};

std::function<FString(const float&)> FDlgTestHelper::FloatToString = [](const float& Value) -> FString
{
	return FString::SanitizeFloat(Value);
};

std::function<FString(const FVector&)> FDlgTestHelper::VectorToString = [](const FVector& Value) -> FString
{
	return Value.ToString();
};

std::function<FString(const FColor&)> FDlgTestHelper::ColorToString = [](const FColor& Value) -> FString
{
	return Value.ToString();
};
